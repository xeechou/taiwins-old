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

typedef struct tw_launcher {
	int tty;	//init to 0
	int ttynr;
	char ttyname[16];
	int signalfd;
	int child;

	int kb_mode;	//This is mode that we store for recovering KD_TEXT
} tw_launcher;

/* helper functions that help you know what you are doing */
static inline int mute_keyboard(tw_launcher *tw)
{
	return (ioctl(tw->tty, KDSKBMUTE, 1) &&
		ioctl(tw->tty, KDSKBMODE, K_OFF));
}

static inline int umute_keyboard(tw_launcher *tw)
{
	return (ioctl(tw->tty, KDSKBMUTE, 0) &&
		ioctl(tw->tty, KDSKBMODE, tw->kb_mode));
}

static inline int save_keyboard_mode(tw_launcher *tw)
{
	return ioctl(tw->tty, KDGKBMODE, &tw->kb_mode);
}
static inline int kd_graphic(tw_launcher *tw)
{
	return ioctl(tw->tty, KDSETMODE, KD_GRAPHICS);
}
static inline int kd_text(tw_launcher *tw)
{
	return ioctl(tw->tty, KDSETMODE, KD_TEXT);
}

static inline int vt_process(tw_launcher *tw)
{
	struct vt_mode mode = {0};
	mode.mode = VT_PROCESS;
	mode.relsig = SIGUSR1;
	mode.acqsig = SIGUSR2;
	return ioctl(tw->tty, VT_SETMODE, &mode);
}
static inline int vt_auto(tw_launcher *tw)
{
	struct vt_mode mode = {0};
	mode.mode = VT_AUTO;
	return ioctl(tw->tty, VT_SETMODE, &mode);
}

static inline int quit(tw_launcher *tw)
{
	if (umute_keyboard(tw)) {
		TW_DEBUG_INFO("Failed to umute keyboard\n");
		return -1;
	}
	if (kd_text(tw)) {
		TW_DEBUG_INFO("Failed to return text mode \n");
		return -1;
	}
	if (vt_auto(tw)) {
		TW_DEBUG_INFO("Failed to return auto mode \n");
		return -1;
	}
	return 0;
}
//Help functions end

static int setup_tty(tw_launcher *tw)
{
	struct stat st;
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

	/* dealing with keyboads, vts */
	if (save_keyboard_mode(tw))
		return -1;
	if (mute_keyboard(tw))
		return -1;

	if (kd_graphic(tw))
		return -1;

	/* now we need to unmute the keyboard */
	if (umute_keyboard(tw))
		return -1;

	/* set vts */
	if (vt_process(tw) < 0)
		return -1;

	return 0;
}

static int setup_signals(tw_launcher *tw)
{
	/* setup signal here so we don't need to  
	 * code them in main */
	int ret;
	sigset_t mask;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	ret = sigaction(SIGCHLD, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigaction(SIGHUP, &sa, NULL);

	sigaddset(&mask, SIGCHLD);
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
	int pid, status;

	switch (signo) {
	case SIGCHLD:
		TW_DEBUG_INFO("catch signal SIGCHLD\n");
		pid = waitpid(-1, &status, 0);
		if (pid == tw->child) {
			tw->child = 0;
			quit(tw);
		}
		return -1;
		break;
	case SIGTERM:
	case SIGINT:
		return -1;
		break;
	case SIGUSR1:
		//TW_DEBUG_INFO("catch signal SIGUSR1\n");
		//drmDropMaster(tw->tty);
		ioctl(tw->tty, VT_RELDISP, 1);
		break;
	case SIGUSR2:
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
	int ret;

	ret= setup_tty(&tw);
	if (ret) {
		printf("error.\n");
		return 0;
	}
	init_debug();
	ret = setup_signals(&tw);
	if (ret) {
		quit(&tw);
		printf("error.\n");
		return 0;
	}


	int child_pid = fork();
	if (child_pid == 0) {
		sleep(1);
		return 0;
	} else {
		/* TODO:if my child process never return, I can remove else */
		ssize_t s;
		struct signalfd_siginfo fds;

		if (setup_signals(&tw)) {
			quit(&tw);
			goto err;
		}
		tw.child = child_pid;

		struct pollfd pfd;
		pfd.fd = tw.signalfd;
		pfd.events = POLLIN;

		int i, ret = 0;
		while (ret == 0) {
			i = poll(&pfd, 1, -1);
			if (i < 0) {
				ret = -1;
				break;
			}
			if (pfd.revents & POLLIN) {
				s = read(tw.signalfd, &fds,
					 sizeof(struct signalfd_siginfo));
				if (s != sizeof(struct signalfd_siginfo)) {
					ret = -1;
					break;
				}
				ret = handle_signal(&tw, fds.ssi_signo);
			}
		}
		ret = quit(&tw);
		if (ret)
			printf("error.\n");
	}
	return 0;
err:
	return -1;
}
