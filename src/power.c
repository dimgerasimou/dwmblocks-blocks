/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <libnotify/notification.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#define POWER_C

#include "../include/colorscheme.h"
#include "../include/utils.h"
#include "../include/config.h"

/* menu prompts */
const char *menu_power           = " Shutdown\t0\n Reboot\t1\n\n󰗽 Logout\t2\n Lock\t3\n\n Restart DwmBlocks\t4";
const char *menu_power_optimus   = "\n󰘚 Optimus Manager\t5";
const char *menu_power_clipboard = "\n󰅌 Clipmenu\t6";
const char *menu_optimus         = "Integrated\t0\nHybrid\t1\nNvidia\t2";
const char *menu_clipboard       = "Pause clipmenu for 1 minute\t0\nClear clipboard\t1";
const char *menu_yes_no          = "Are you sure?\t-1\nYes\t1\nNo\t0";

#ifdef CLIPBOARD
static void
clippause(const unsigned int seconds)
{
	switch (fork()) {
	case -1:
		errno = ECHILD;
		logwrite("fork() failed", NULL, LOG_FATAL, "dwmblocks-power");
		exit(errno);

	case 0:
		setsid();
		pid_t pid = getpidof("clipmenud", "dmblocks-power");

		if (pid < 0) {
			pid = getpidof("bash\0/usr/bin/clipmenud", "dmblocks-power");
			if (pid < 0) {
				errno = ESRCH;
				logwrite("Couldn't get pID of", "clipmenud", LOG_FATAL, "dwmblocks-power");
			}
		}

		kill(pid, SIGUSR1);
		notify("Clipboard", "clipmenu is now disabled.", "com.github.davidmhewitt.clipped", NOTIFY_URGENCY_NORMAL, 1);

		sleep(seconds);

		kill(pid, SIGUSR2);
		notify("Clipboard", "clipmenu is now enabled.", "com.github.davidmhewitt.clipped", NOTIFY_URGENCY_NORMAL, 1);

		exit(EXIT_SUCCESS);

	default:
		break;
	}
}

static void
clipmenu(void)
{
	switch (getxmenuopt(menu_clipboard, "dwmblocks-power")) {
	case 0:
		clippause(60);
		break;

	case 1:
		forkexecvp((char**) args_clipboard_delete, "dwmblocks-power");
		break;

	default:
		break;
	}
}
#endif

#ifdef POWER_MANAGEMENT
const char *args_optimus_integrated[] = {"optimus-manager", "--no-confirm", "--switch", "integrated", NULL};
const char *args_optimus_hybrid[]     = {"optimus-manager", "--no-confirm", "--switch", "hybrid", NULL};
const char *args_optimus_nvidia[]     = {"optimus-manager", "--no-confirm", "--switch", "nvidia", NULL};

static void
optimusmenu(void)
{
	switch (getxmenuopt(menu_optimus, "dwmblocks-power")) {
	case 0:
		if (getxmenuopt(menu_yes_no, "dwmblocks-power") == 1)
			forkexecvp((char**) args_optimus_integrated, "dwmblocks-power");
		break;

	case 1:
		if (getxmenuopt(menu_yes_no, "dwmblocks-power") == 1)
			forkexecvp((char**) args_optimus_hybrid, "dwmblocks-power");
		break;

	case 2:
		if (getxmenuopt(menu_yes_no, "dwmblocks-power") == 1)
			forkexecvp((char**) args_optimus_nvidia, "dwmblocks-power");
		break;

	default:
		break;
	}
}
#endif

static void
restartdwmblocks(void)
{
	char *path = NULL;
	int pid = 0;

	path = getpath((char**) path_dwmblocks);

	if ((pid = getpidof("dwmblocks", "dwmblocks-power")) == -ENOENT) {
		pid = getpidof(path, "dwmblocks-power");
	}

	if (pid <= 0)
		logwrite("Failed to get the pID of", path, LOG_FATAL, "dwmblocks-power");

	kill(pid, SIGTERM);
	unsetenv("BLOCK_BUTTON");
	forkexecvp((char**) args_dwmblocks, "dwmblocks-power");
	free(path);
}

static void
lockscreen(void)
{
	sleep(1);
	forkexecvp((char**) args_lockscreen, "dwmblocks-power");
}

static void
mainmenu(void)
{
	char *menu = strdup(menu_power);

	#ifdef POWER_MANAGEMENT
		strapp(&menu, menu_power_optimus);
	#endif
	
	#ifdef CLIPBOARD
		strapp(&menu, menu_power_clipboard);
	#endif

	switch (getxmenuopt(menu, "dwmblocks-power")) {
	case 0:
		if (getxmenuopt(menu_yes_no, "dwmblocks-power") == 1)
			execl("/bin/shutdown", "shutdown", "now", NULL);
		break;

	case 1:
		if (getxmenuopt(menu_yes_no, "dwmblocks-power") == 1)
			execl("/bin/shutdown", "shutdown", "now", "-r", NULL);
		break;

	case 2:
		if (getxmenuopt(menu_yes_no, "dwmblocks-power") == 1) {
			int pid = getpidof("/usr/local/bin/dwm", "dwmblocks-power");
			if (pid > 0) {
				kill(pid, SIGTERM);
			} else {
				logwrite("Failed to get pid of process: ", "dwm", LOG_ERROR, "dwmblocks-power");
				exit(errno);
			}
		}
		break;

	case 3:
		lockscreen();
		break;

	case 4:
		restartdwmblocks();
		break;

	#ifdef POWER_MANAGEMENT
	case 5:
		optimusmenu();
		break;
	#endif
	
	#ifdef CLIPBOARD
	case 6:
		clipmenu();
		break;
	#endif
	
	default:
		break;
	}
	free(menu);
}

static void
execbutton(void)
{
	char *env = NULL;

	env = getenv("BLOCK_BUTTON");

	if (!env || strcmp(env, "1"))
		return;

	mainmenu();
}

int
main(void)
{
	execbutton();
	printf(CLR_1 BG_1"  \n");

	return 0;
}
