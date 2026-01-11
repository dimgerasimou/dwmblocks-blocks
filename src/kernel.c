/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#define KERNEL_C
#define BUF_SIZE 64

#include "colorscheme.h"
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
		logwrite("popen() failed for command", cmd, LOG_WARN, "dwmblocks-kernel");
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
		char *body = NULL;
		NotifyNotification *n = NULL;
		char numbuf[32];

		n = newnotify("Packages", "Getting packages upgrade info..", "tux",
		              NOTIFY_URGENCY_NORMAL, 1);

		*aur = (int)getupdates(cmd_aur_updates);
		*pm  = (int)getupdates(cmd_pm_updates);

		strapp(&body, "󰏖 Pacman Updates: ");
		snprintf(numbuf, sizeof(numbuf), "%d", *pm);
		strapp(&body, numbuf);

		strapp(&body, "\n AUR Updates: ");
		snprintf(numbuf, sizeof(numbuf), "%d", *aur);
		strapp(&body, numbuf);

		if (n)
			updatenotify(n, "Packages", body ? body : "No data", "tux",
			             NOTIFY_URGENCY_NORMAL, 0, 1);

		free(body);
		if (n)
			freenotify(n);
		break;
	}

	case 3:
		forkexecvp((char **)args_update_cmd, "dwmblocks-kernel");
		break;

	default:
		break;
	}
}

int
main(void)
{
	char *release = NULL;
	int aur = -1;
	int pm  = -1;

	execbutton(&aur, &pm);

	if (aur == -1)
		aur = (int)getupdates(cmd_aur_updates);
	if (pm == -1)
		pm = (int)getupdates(cmd_pm_updates);

	if (show_release) {
		struct utsname buf;

		if (uname(&buf) != 0) {
			logwrite("uname() failed while reading kernel release", NULL, LOG_WARN,
			         "dwmblocks-kernel");
		} else {
			/* duplicate and strip suffix like "-arch1-1" */
			release = strdup(buf.release);
			if (release) {
				char *dash = strchr(release, '-');
				if (dash)
					*dash = '\0';
			}
		}
	}

	if ((aur + pm) > 0) {
		printf(CLR_KRN_PKG "󰏖 ");
		if (show_update_count)
			printf("%d ", aur + pm);
	}

	printf(CLR_KRN_NRM "");

	if (show_release && release && *release)
		printf(" %s", release);

	printf("\n" CLR_NRM);

	free(release);
	return 0;
}
