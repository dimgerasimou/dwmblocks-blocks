/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#define KERNEL_C
#define BUF_SIZE  64
#define BODY_SIZE 128
#define LEN(a)    (sizeof(a) / sizeof((a)[0]))

#include "colors.h"
#include "utils.h"
#include "config.h"

static unsigned int
getupdates(const char *cmd)
{
	FILE *ep;
	char buffer[BUF_SIZE];
	unsigned int counter = 0;

	if (!cmd || !*cmd)
		return 0;

	ep = popen(cmd, "r");
	if (!ep) {
		warn("popen() for: %s", cmd);
		return 0;
	}

	while (fgets(buffer, sizeof(buffer), ep))
		counter++;

	(void)pclose(ep);

	return counter;
}

static void
execbutton(int *aur, int *pm)
{
	const char *env;

	env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 1: {
		char body[BODY_SIZE];
		int  n;

		*aur = (int)getupdates(cmd_aur_updates);
		*pm  = (int)getupdates(cmd_pm_updates);

		n = snprintf(body, sizeof(body),
		             "󰏖 Pacman Updates: %d\n"
		             " AUR Updates: %d",
		             *pm, *aur);

		if (n < 0 || (size_t)n >= sizeof(body))
			warn("notification body truncated");

		notify("Packages", body, "tux");
		break;
	}

	case 3:
		execute((char **)args_update_cmd);
		break;

	default:
		break;
	}
}

int
main(void)
{
	const enum Color def_cols[] = { clr_krn_pkg, clr_krn_nrm };

	struct utsname un;
	char          *release = NULL;
	int            aur = -1;
	int            pm  = -1;

	set_name("dwmblocks-kernel");
	clr_init(def_cols, LEN(def_cols));

	execbutton(&aur, &pm);

	if (aur == -1)
		aur = (int)getupdates(cmd_aur_updates);
	if (pm == -1)
		pm = (int)getupdates(cmd_pm_updates);

	if (show_release) {
		if (uname(&un) != 0) {
			warn("uname():");
		} else {
			char *dash;

			/* strip a suffix like "-arch1-1" in place */
			release = un.release;
			dash = strchr(release, '-');
			if (dash)
				*dash = '\0';
		}
	}

	if ((aur + pm) > 0) {
		printf("%s󰏖 ", clr_get(clr_krn_pkg));
		if (show_update_count)
			printf("%d ", aur + pm);
	}

	printf("%s", clr_get(clr_krn_nrm));

	if (show_release && release && *release)
		printf(" %s", release);

	printf(CLR_NRM "\n");

	return 0;
}
