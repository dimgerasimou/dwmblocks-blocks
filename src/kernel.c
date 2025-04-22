/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#define KERNEL_C
#define BUF_SIZE 64

#include "../include/colorscheme.h"
#include "../include/utils.h"
#include "../include/config.h"

static unsigned int
getupdates(const char *cmd)
{
	FILE *ep;
	char buffer[BUF_SIZE];
	int  counter = 0;

	if (!(ep = popen(cmd, "r"))) {
		logwrite("popen() failed for command", cmd, LOG_WARN, "dwmblocks-kernel");
		return 0;
	}

	while(fgets(buffer, sizeof(buffer), ep))
		counter++;

	pclose(ep);
	return counter;
}

static void
execbutton(unsigned int  *aur, unsigned int *pm)
{
	char *env;
	
	if (!(env = getenv("BLOCK_BUTTON")))
		return;

	switch (atoi(env)) {
	case 1:
	{
		char *body = NULL;
		NotifyNotification *n = NULL;

		n = newnotify("Packages", "Getting packages upgrade info..", "tux", NOTIFY_URGENCY_NORMAL, 1);

		*aur = getupdates(cmd_aur_updates);
		*pm  = getupdates(cmd_pm_updates);

		strapp(&body, "󰏖 Pacman Updates: ");
		strapp(&body, uitoa(*pm));
		strapp(&body, "\n AUR Updates: ");
		strapp(&body, uitoa(*aur));

		updatenotify(n, "Packages", body, "tux", NOTIFY_URGENCY_NORMAL, 0, 1);
		free(body);
		freenotify(n);
		break;
	}

	case 3:
		forkexecvp((char**) args_update_cmd, "dwmblocks-kernel");
		break;

	default:
		break;
	}
}

int
main(void)
{
	struct utsname buf;
	char*          release;
	unsigned int   aur = 0;
	unsigned int   pm  = 0;

	execbutton(&aur, &pm);

	aur = aur == 0 ? getupdates(cmd_aur_updates) : aur;
	pm  = pm == 0  ? getupdates(cmd_pm_updates)  : pm;

	if (uname(&buf))
		logwrite("Failed in allocatting utsname struct", NULL, LOG_FATAL, "dwmblocks-kernel");

	release = strtok(buf.release, "-");

	if ((aur + pm) > 0)
		printf(CLR_12" 󰏖 %d  %s\n", aur + pm, release);
	else
		printf(CLR_12"  %s\n", release);

	return 0;
}
