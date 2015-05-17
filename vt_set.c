
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>


#include <linux/vt.h>
#include <linux/kd.h>
#include <xf86drm.h>	/* for drmSetMaster and drmDropMaster */

#ifndef KDSKBMUTE
#define KDSKBMUTE	0x4B51
#endif

static const char *TTY0 = "/dev/tty0";

typedef struct tw_launcher {
	int tty;	//init to 0
	int ttynr;
	char ttyname[16];

	int kb_mode;
} tw_launcher;
static int
setup_tty(tw_launcher *tw)
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
	mode.mode = VT_PROCESS;
	mode.relsig = SIGUSR1;
	mode.acqsig = SIGUSR2;
	if (ioctl(tw->tty, VT_SETMODE, &mode) < 0)
		return -1;

	getchar();
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

//static int
//handle_signal(struct weston_launch *wl)
//{
//	struct signalfd_siginfo sig;
//	int pid, status, ret;
//
//	if (read(wl->signalfd, &sig, sizeof sig) != sizeof sig) {
//		error(0, errno, "reading signalfd failed");
//		return -1;
//	}
//
//	switch (sig.ssi_signo) {
//	case SIGCHLD:
//		pid = waitpid(-1, &status, 0);
//		if (pid == wl->child) {
//			wl->child = 0;
//			if (WIFEXITED(status))
//				ret = WEXITSTATUS(status);
//			else if (WIFSIGNALED(status))
//				/*
//				 * If weston dies because of signal N, we
//				 * return 10+N. This is distinct from
//				 * weston-launch dying because of a signal
//				 * (128+N).
//				 */
//				ret = 10 + WTERMSIG(status);
//			else
//				ret = 0;
//			quit(wl, ret);
//		}
//		break;
//	case SIGTERM:
//	case SIGINT:
//		if (wl->child)
//			kill(wl->child, sig.ssi_signo);
//		break;
//	case SIGUSR1:
//		send_reply(wl, WESTON_LAUNCHER_DEACTIVATE);
//		close_input_fds(wl);
//		drmDropMaster(wl->drm_fd);
//		ioctl(wl->tty, VT_RELDISP, 1);
//		break;
//	case SIGUSR2:
//		ioctl(wl->tty, VT_RELDISP, VT_ACKACQ);
//		drmSetMaster(wl->drm_fd);
//		send_reply(wl, WESTON_LAUNCHER_ACTIVATE);
//		break;
//	default:
//		return -1;
//	}
//
//	return 0;
//}

int main()
{
	tw_launcher tw;
	memset(&tw, 0, sizeof(tw));
	int ret = setup_tty(&tw);
	if (ret)
		printf("error.\n");
}
