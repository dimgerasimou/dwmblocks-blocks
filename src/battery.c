/* See LICENSE file for copyright and license details. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BATTERY_C
#define BUFFER_SIZE 64

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

const char *battery_icon_list[]  = { CLR_1" ", CLR_3" ", CLR_2" ", CLR_2" ", CLR_2" " };

#ifdef POWER_MANAGEMENT
const char *optimus_manager_command = "optimus-manager --status";
const char *mode_text_list[]        = { "Unmanaged", "Integrated", "Hybrid", "Nvidia" };
const char *mode_icon_list[]        = { "battery", "intel", "deepin-graphics-driver-manager", "nvidia" };

static unsigned int
getmode(void)
{
	char buf[BUFFER_SIZE];
	FILE         *ep  = NULL;
	unsigned int  ret = 0;

	if (!(ep = popen(optimus_manager_command, "r"))) {
		logwrite("popen() failed for", optimus_manager_command, LOG_WARN, "dwmblocks-battery");
		return 0;
	}

	while (fgets(buf, sizeof(buf), ep))
		if (strstr(buf, "Current"))
			break;

	if (strstr(buf, "integrated"))
		ret = 1;
	else if (strstr(buf, "hybrid"))
		ret = 2;
	else if (strstr(buf, "nvidia"))
		ret = 3;

	pclose(ep);

	return ret;
}

static void
send_notification(const char *capacity, const char *status)
{
	char         *body = NULL;
	unsigned int  mode = getmode();

	strapp(&body, "Battery capacity: ");
	strapp(&body, capacity);
	strapp(&body, "%\nBattery status:   ");
	strapp(&body, status);
	strapp(&body, "\nOptimus manager:  ");
	strapp(&body, mode_text_list[mode]);

	notify("Power", body, (char*) mode_icon_list[mode], NOTIFY_URGENCY_NORMAL, 1);

	free(body);
}

#else /* POWER_MANAGEMENT */

static void
send_notification(const char *capacity, const char *status)
{
	char *body = NULL;

	strapp(&body, "Battery capacity: ");
	strapp(&body, capacity);
	strapp(&body, "%\nBattery status:   ");
	strapp(&body, status);

	notify("Power", body, "battery", NOTIFY_URGENCY_NORMAL, 1);

	free(body);
}

#endif /* POWER_MANAGEMENT */

static void
execbutton(const unsigned int capacity, const char *status)
{
	char *env = NULL;

	if (!(env = getenv("BLOCK_BUTTON")))
		return;

	switch (atoi(env)) {
	case 1:
	{
		char *capstr = uitoa(capacity);

		if (!capstr)
			break;

		send_notification(capstr, status);
		
		free(capstr);
		break;
	}
	default:
		break;
	}
}

static unsigned int
getcapacity(void)
{
	FILE         *fp   = NULL;
	char         *path = NULL;
	unsigned int  ret  = 0;

	strapp(&path, path_battery_kernel);
	strapp(&path, "/capacity");

	if (!(fp = fopen(path, "r")))
		logwrite("fopen() failed for file", path, LOG_FATAL, "dwmblocks-battery");

	fscanf(fp, "%u", &ret);
	fclose(fp);
	free(path);

	return ret;
}

static char*
getstatus(void)
{
	char buf[BUFFER_SIZE];
	FILE *fp   = NULL;
	char *path = NULL;

	strapp(&path, path_battery_kernel);
	strapp(&path, "/status");

	if (!(fp = fopen(path, "r")))
		logwrite("fopen() failed for file", path, LOG_FATAL, "dwmblocks-battery");

	fgets(buf, sizeof(buf), fp);
	buf[BUFFER_SIZE - 1] = '\0';

	fclose(fp);
	free(path);

	trimtonewl(buf);

	return strdup(buf);
}


static void
batterymode(const unsigned int capacity)
{
	FILE *fp;
	char *path = NULL;
	
	unsigned int prev_capacity = 0;
	unsigned int prev_ac = 0;
	unsigned int ac = 0;
	
	fp = fopen(path_tmp_file, "r");

	if (fp) {
		fscanf(fp, "%u, %u", &prev_ac, &prev_capacity);
		fclose(fp);
	}

	strapp(&path, path_adapter_kernel);
	strapp(&path, "/online");
	
	fp = fopen(path, "r");
	if (fp) {
		fscanf(fp, "%u", &ac);
		fclose(fp);
	}

	free(path);

	fp = fopen(path_tmp_file, "w");
	fprintf(fp, "%u, %u\n", ac, capacity);
	fclose(fp);

	if (ac != prev_ac || (!ac && prev_capacity >= 25 && capacity < 25)) {
		path = getpath((char**) path_power_mode);
		forkexecvs(path, (char**) args_power_mode, "dwmblocks-battery");
		free(path);
	}
}
int
main(void)
{
	char         *status   = getstatus();
	unsigned int  capacity = getcapacity();

	batterymode(capacity);
	execbutton(capacity, status);

	if(!strcmp(status, "Charging")) {
		printf(CLR_3 BG_1" "NRM"\n");
		free(status);

		return EXIT_SUCCESS;
	}

	printf(BG_1"%s\n", battery_icon_list[lround(capacity / 25.0)]);

	free(status);
	return 0;
}
