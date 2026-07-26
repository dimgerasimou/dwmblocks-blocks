/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#define POWER_C
#define MENU_SIZE 512
#define LEN(a)    (sizeof(a) / sizeof((a)[0]))

#include "colors.h"
#include "utils.h"
#include "config.h"

/* menu prompts */
static const char *const menu_power           = " Shutdown\t0\n Reboot\t1\n\n󰗽 Logout\t2\n Lock\t3\n\n Restart DwmBlocks\t4";
static const char *const menu_power_optimus   = "\n󰘚 Optimus Manager\t5";
static const char *const menu_power_clipboard = "\n󰅌 Clipmenu\t6";
static const char *const menu_optimus         = "Integrated\t0\nHybrid\t1\nNvidia\t2";
static const char *const menu_clipboard       = "Pause clipmenu for 1 minute\t0\nClear clipboard\t1";
static const char *const menu_yes_no          = "Are you sure?\t-1\nYes\t1\nNo\t0";

#ifdef CLIPBOARD
static void
clippause(const unsigned int seconds)
{
	pid_t pid;

	switch (fork()) {
	case -1:
		die("fork:");

	case 0:
		setsid();

		pid = getpidof("clipmenud");
		if (pid < 0)
			pid = getpidof("bash");

		if (pid < 0) {
			warn("could not find clipmenud");
			_exit(EXIT_FAILURE);
		}

		if (kill(pid, SIGUSR1) < 0) {
			warn("kill() for clipmenud:");
			_exit(EXIT_FAILURE);
		}

		notify("Clipboard", "clipmenu is now disabled.",
		       "com.github.davidmhewitt.clipped");

		sleep(seconds);

		if (kill(pid, SIGUSR2) < 0)
			warn("kill() for clipmenud:");

		notify("Clipboard", "clipmenu is now enabled.",
		       "com.github.davidmhewitt.clipped");

		_exit(EXIT_SUCCESS);

	default:
		break;
	}
}

static void
clipmenu(void)
{
	switch (getxmenuopt(menu_clipboard)) {
	case 0:
		clippause(60);
		break;

	case 1:
		execute((char **)args_clipboard_delete);
		break;

	default:
		break;
	}
}
#endif

#ifdef POWER_MANAGEMENT
static const char *const args_optimus_integrated[] = {"optimus-manager", "--no-confirm", "--switch", "integrated", NULL};
static const char *const args_optimus_hybrid[]     = {"optimus-manager", "--no-confirm", "--switch", "hybrid", NULL};
static const char *const args_optimus_nvidia[]     = {"optimus-manager", "--no-confirm", "--switch", "nvidia", NULL};

static void
optimusmenu(void)
{
	switch (getxmenuopt(menu_optimus)) {
	case 0:
		if (getxmenuopt(menu_yes_no) == 1)
			execute((char **)args_optimus_integrated);
		break;

	case 1:
		if (getxmenuopt(menu_yes_no) == 1)
			execute((char **)args_optimus_hybrid);
		break;

	case 2:
		if (getxmenuopt(menu_yes_no) == 1)
			execute((char **)args_optimus_nvidia);
		break;

	default:
		break;
	}
}
#endif

static void
restartdwmblocks(void)
{
	char  path[PATH_MAX];
	pid_t pid;

	pid = getpidof("dwmblocks");

	if (pid < 0 && getpath(path_dwmblocks, path, sizeof(path)) == 0)
		pid = getpidof(path);

	if (pid < 0) {
		warn("failed to get the pid of dwmblocks");
		return;
	}

	if (kill(pid, SIGTERM) < 0) {
		warn("kill() for dwmblocks:");
		return;
	}

	unsetenv("BLOCK_BUTTON");
	execute((char **)args_dwmblocks);
}

static void
lockscreen(void)
{
	sleep(1);
	execute((char **)args_lockscreen);
}

static void
mainmenu(void)
{
	static char *const args_shutdown[] = { "shutdown", "now", NULL };
	static char *const args_reboot[]   = { "shutdown", "-r", "now", NULL };

	char   menu[MENU_SIZE];
	size_t off = 0;
	int    n;

	n = snprintf(menu, sizeof(menu), "%s", menu_power);
	if (n < 0 || (size_t)n >= sizeof(menu))
		die("menu too long");
	off = (size_t)n;

#ifdef POWER_MANAGEMENT
	n = snprintf(menu + off, sizeof(menu) - off, "%s", menu_power_optimus);
	if (n < 0 || (size_t)n >= sizeof(menu) - off)
		die("menu too long");
	off += (size_t)n;
#endif

#ifdef CLIPBOARD
	n = snprintf(menu + off, sizeof(menu) - off, "%s", menu_power_clipboard);
	if (n < 0 || (size_t)n >= sizeof(menu) - off)
		die("menu too long");
	off += (size_t)n;
#endif

	(void)off;

	switch (getxmenuopt(menu)) {
	case 0:
		if (getxmenuopt(menu_yes_no) == 1)
			execute((char **)args_shutdown);
		break;

	case 1:
		if (getxmenuopt(menu_yes_no) == 1)
			execute((char **)args_reboot);
		break;

	case 2:
		if (getxmenuopt(menu_yes_no) == 1) {
			pid_t pid = getpidof("/usr/local/bin/dwm");

			if (pid < 0)
				warn("failed to get the pid of dwm");
			else if (kill(pid, SIGTERM) < 0)
				warn("kill() for dwm:");
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
}

static void
execbutton(void)
{
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	if (!strcmp(env, "1"))
		mainmenu();
}

int
main(void)
{
	const enum Color def_cols[] = { clr_pwr };

	set_name("dwmblocks-power");
	clr_init(def_cols, LEN(def_cols));

	execbutton();

	printf("%s " CLR_NRM "\n", clr_get(clr_pwr));

	return 0;
}
