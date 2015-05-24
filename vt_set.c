#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signalfd.h>


#include <linux/vt.h>
#include <linux/kd.h>
#include <xf86drm.h>

#ifndef KDSKBMUTE
#define KDSKBMUTE	0x4B51	/* not gonna work for now */
#endif

static const char *TTY0 = "/dev/tty0";
static int signal_recv = 0;

typedef struct tw_launcher {
	int tty;	//init to 0
	int ttynr;
	char ttyname[16];
	int signalfd;

	int kb_mode;	//This is mode that we store for recovering KD_TEXT
} tw_launcher;


static int quit(tw_launcher *tw)
{
	/* recover the the context that we hade before */
	struct vt_mode mode = {0};

	if (ioctl(tw->tty, KDSKBMUTE, 0) &&
	    ioctl(tw->tty, KDSKBMODE, tw->kb_mode))
		return -1;
	if (ioctl(tw->tty, KDSETMODE, KD_TEXT))
		return -1;
	mode.mode = VT_AUTO;
	if (ioctl(tw->tty, VT_SETMODE, &mode) < 0)
		return -1;
	return 0;
}

static int setup_tty(tw_launcher *tw)
{
	struct stat st;
	struct vt_mode mode = {0};
	char ttyname[16];
	int fd = -1;
	int found_tty;

	/* now we control the tty you invoke me */
	fd = STDIN_FILENO;
	found_tty = isatty(STDIN_FILENO);
	if (found_tty < 0) {
		fd = STDOUT_FILENO;
		found_tty = isatty(STDOUT_FILENO);
	}
	if (found_tty < 0) {
		fd = STDERR_FILENO;
		found_tty = isatty(STDERR_FILENO);
	}
	if (found_tty >= 0)
		ttyname_r(fd, tw->ttyname, 16);
	else
		return -1;	//TODO, open a avaliable terminal

	/* mute keyboard, before we done set signals */
	if (ioctl(tw->tty, KDGKBMODE, &tw->kb_mode))
		return -1;
	if (ioctl(tw->tty, KDSKBMUTE, 1) &&
	    ioctl(tw->tty, KDSKBMODE, K_OFF))
		return -1;


	if (ioctl(tw->tty, KDSETMODE, KD_GRAPHICS))
		return -1;

	/* now we need to unmute the keyboard */
	if (ioctl(tw->tty, KDSKBMUTE, 0) &&
	    ioctl(tw->tty, KDSKBMODE, tw->kb_mode))
		return -1;
	mode.mode = VT_PROCESS;
	mode.relsig = SIGUSR1;
	mode.acqsig = SIGUSR2;
	if (ioctl(tw->tty, VT_SETMODE, &mode) < 0)
		return -1;

	return 0;
}

static int setup_signals(tw_launcher *tw)
{
	int ret;
	sigset_t mask;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	ret = sigaction(SIGCHLD, &sa, NULL);

	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	ret = sigprocmask(SIG_BLOCK, &mask, NULL);
	if (ret)
		return -errno;

	tw->signalfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
	if (tw->signalfd < 0)
		return -errno;

	return 0;
}

static int
handle_signal(tw_launcher *tw, int signo)
{
	switch (signo) {
	//case SIGCHLD:
	//	pid = waitpid(-1, &status, 0);
	//	if (pid == wl->child) {
	//		wl->child = 0;
	//		if (WIFEXITED(status))
	//			ret = WEXITSTATUS(status);
	//		else if (WIFSIGNALED(status))
	//			/*
	//			 * If weston dies because of signal N, we
	//			 * return 10+N. This is distinct from
	//			 * weston-launch dying because of a signal
	//			 * (128+N).
	//			 */
	//			ret = 10 + WTERMSIG(status);
	//		else
	//			ret = 0;
	//		quit(wl, ret);
	//	}
	//	break;
	//case SIGTERM:
	case SIGINT:
		return -1;
		break;
	case SIGUSR1:
		signal_recv += 1;
		//TW_DEBUG_INFO("catch signal SIGUSR1\n");
		//drmDropMaster(tw->tty);
		ioctl(tw->tty, VT_RELDISP, 1);
		break;
	case SIGUSR2:
		signal_recv += 1;
		//TW_DEBUG_INFO("catch signal SIGUSR2\n");
		ioctl(tw->tty, VT_RELDISP, VT_ACKACQ);
		//drmSetMaster(tw->tty);
		//send_reply(wl, WESTON_LAUNCHER_ACTIVATE);
		break;
	default:
		return -1;
	}

	return 0;
}



int main()
{
	tw_launcher tw;
	memset(&tw, 0, sizeof(tw));
	int ret = setup_tty(&tw);
	if (ret) {
		printf("error.\n");
		return 0;
	}
	init_debug();
	//ret = setup_signals(&tw);
	//if (ret) {
	//	quit(&tw);
	//	printf("error.\n");
	//	return 0;
	//}

	struct signalfd_siginfo fds;
	sigset_t sigmask;
	ssize_t s;

	sigaddset(&sigmask, SIGUSR1);
	sigaddset(&sigmask, SIGUSR2);
	if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1) {
		quit(&tw);
		return 0;
	}
	tw.signalfd = signalfd(-1, &sigmask, 0);
	if (tw.signalfd == -1) {
		quit(&tw);
		printf("error in get signalfd\n");
	}

	struct pollfd pfd;
	pfd.fd = tw.signalfd;
	pfd.events = POLLIN;

	int i;
	for (;;) {
		i = poll(&pfd, 1, -1);
		if (i < 0)
			return -1;
		if (pfd.revents & POLLIN) {
			s = read(tw.signalfd, &fds,
				 sizeof(struct signalfd_siginfo));
			if (s != sizeof(struct signalfd_siginfo)) {
				quit(&tw);
				return -1;
			}
			ret = handle_signal(&tw, fds.ssi_signo);
		}
	}
	ret = quit(&tw);
	printf("%d %d %d\n", STDIN_FILENO, tw.signalfd, signal_recv);
	if (ret)
		printf("error.\n");
}
