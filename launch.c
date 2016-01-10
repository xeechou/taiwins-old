#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

	long kb_mode;	//This is mode that we store for recovering KD_TEXT
	bool activated;
} tw_launcher;

static tw_launcher launcher = {0};

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
	struct vt_mode mode = {.mode = VT_AUTO};
	return ioctl(tw->tty, VT_SETMODE, &mode);
}

static void activate(tw_launcher *tw)
{
	//for now we need root privillege to setMaster
	drmSetMaster(tw->tty);	//XXX:its gonna be a err anyway
	tw->activated = true;
}

static void deactivate(tw_launcher *tw)
{
	//for now we need root privillege to setMaster
	drmDropMaster(tw->tty);
	tw->activated = false;
}

static inline int quit(tw_launcher *tw)
{
	deactivate(tw);
	umute_keyboard(tw);
	kd_text(tw);
	vt_auto(tw);
	//close  fd
	close(tw->tty);
	return 0;
}
//Help functions end

static int setup_tty(tw_launcher *tw)
{
	int ret = 0;
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
		goto err;	//TODO, open a avaliable terminal

	/* dealing with keyboads, vts */
	if (save_keyboard_mode(tw)) {
		ret = -1;
		goto err;
	}
	if (mute_keyboard(tw)) {
		ret = -2;
		goto err;
	}
	if (kd_graphic(tw)) {
		ret = -3;
		goto err_kd;
	}
	if (vt_process(tw) < 0) {
		ret = -4;
		goto err_vt;
	}
	activate(&launcher);
	/* now we need to unmute the keyboard */
	if (umute_keyboard(tw)) {
		ret = -5;
		goto err_last;
	}
	return 0;

err_last:
	deactivate(tw);
	vt_auto(tw);
err_vt:
	kd_text(tw);
err_kd:
	umute_keyboard(tw);
err:
	return ret;
}

/* there are two ways to handle signal, one is by setting signal handler on each
 * signal, the other is by blocking it and use signalfd to read fd.
 *
 * sigprocmask(SIG_BLOCK, &sigset, NULL) is usually for blocking signal for a
 * short time to do something in a atomical env.
 */
static void handle_chld(int signal)
{
	int status;
	wait(&status);
	fprintf(stderr, "Server exit with status %d\n", WEXITSTATUS(status));
	quit(&launcher);
	exit(WEXITSTATUS(status));
}

static void handle_usr1(int signal)
{
	tw_launcher *tw = &launcher;
	//drmDropMaster(tw->tty);
	//TW_DEBUG_INFO("handle_usr1\n");
	ioctl(tw->tty, VT_RELDISP, 1);
}
static void handle_usr2(int signal)
{
	tw_launcher *tw = &launcher;
	//TW_DEBUG_INFO("handle_usr2\n");
	ioctl(tw->tty, VT_RELDISP, VT_ACKACQ);
	//drmSetMaster(tw->tty);
}
static void forward_signal(int signal)
{
	kill(launcher.child, signal);
}

static int setup_signals(tw_launcher *tw)
{
	struct sigaction sa = {0};
	char *fsig_reg = "Failed to register signal handler for";

	//handle child terminate or stoped
	sa.sa_handler = &handle_chld;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGCHLD\n", fsig_reg);
		return -1;
	} //release the vt
	sa.sa_handler = &handle_usr1;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGUSR1\n", fsig_reg);
		return -1;
	} //return back to vt
	sa.sa_handler = &handle_usr2;
	if (sigaction(SIGUSR2, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGUSR2\n", fsig_reg);
		return -1;
	} //all the other signal can cause child to terminate, we just forward it
	sa.sa_handler = &forward_signal;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGINT\n", fsig_reg);
		return -1;
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGACTION\n", fsig_reg);
		return -1;
	}
	return 0;
}
static int reset_signals(void)
{
	struct sigaction sa = {0};
	char *fsig_reg = "Failed to register signal handler for";
	return 0;
	sa.sa_handler = SIG_DFL;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGCHLD\n", fsig_reg);
		return -1;
	}
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGUSR1\n", fsig_reg);
		return -1;
	}
	if (sigaction(SIGUSR2, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGUSR2\n", fsig_reg);
		return -1;
	}
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGINT\n", fsig_reg);
		return -1;
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		fprintf(stderr, "%s SIGACTION\n", fsig_reg);
		return -1;
	}
}


/* we gotta have two different processes, so when the compositor is dead, we can
 * still switch out */
int main()
{
	int ret;
	if ((ret = setup_tty(&launcher))) {
		//TW_DEBUG_ERROR("Could not setup a tty to work %d\n", ret);
		//printf("Could not setup a tty to work.\n");	//printf is not
		//working is this env anymore
		goto err_tty;
	}
	if (setup_signals(&launcher)) {
		goto err_sig;
		//TW_DEBUG_ERROR("Could not setup signals.\n");
		return -1;
	}

	int child_pid = fork();
	/* children context */
	if (child_pid == 0) {
		reset_signals();
		//TODO: reset signal handlers back to default
		//the child process sleep until we wake it up.
		sleep(10);
		return 0;
	}
	/* parent context */
	else {
		launcher.child = child_pid;

		int ret = 0;
		while (ret == 0) {
			//there is a empty loop
		}
		ret = quit(&launcher);
		if (ret)
			printf("error.\n");
	}
	return 0;

err_sig:
	quit(&launcher);
err_tty:
	return -1;
}

//int main()
//{
//	int ret = 0;
//	init_debug();
//	if ((ret = setup_tty(&launcher))) {
//		TW_DEBUG_ERROR("Could not setup a tty to work %d\n", ret);
//		//printf("Could not setup a tty to work.\n");
//		goto err_tty;
//	}
//	if (setup_signals(&launcher)) {
//		goto err_sig;
//		TW_DEBUG_ERROR("Could not setup signals.\n");
//		return -1;
//	}
//
//	quit(&launcher);
//	close_debug();
//	return 0;
//err_sig:
//	quit(&launcher);
//err_tty:
//	return -1;
//}
