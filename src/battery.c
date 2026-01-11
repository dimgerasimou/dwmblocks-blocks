/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BATTERY_C
#define BUF_SIZE 64

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

static const char *icons_battery[] = {
	CLR_BAT_CRT " ",
	CLR_BAT_LOW " ",
	CLR_BAT_NRM " ",
	CLR_BAT_NRM " ",
	CLR_BAT_NRM " ",
};

static const char *icon_charging = CLR_BAT_CHG " ";

#ifdef POWER_MANAGEMENT
const char *optimus_manager_command = "optimus-manager --status";
const char *mode_text_list[]        = { "Unmanaged", "Integrated", "Hybrid", "Nvidia" };
const char *mode_icon_list[]        = { "battery", "intel", "deepin-graphics-driver-manager", "nvidia" };

static unsigned int
getmode(void)
{
	char buf[BUF_SIZE];
	FILE *ep = NULL;
	unsigned int ret = 0;

	ep = popen(optimus_manager_command, "r");
	if (!ep) {
		logwrite("popen() failed for", optimus_manager_command, LOG_WARN, "dwmblocks-battery");
		return 0;
	}

	buf[0] = '\0';
	while (fgets(buf, sizeof(buf), ep)) {
		if (strstr(buf, "Current"))
			break;
		buf[0] = '\0';
	}

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
	char *body = NULL;
	unsigned int mode = getmode();

	strapp(&body, "Battery capacity: ");
	strapp(&body, capacity);
	strapp(&body, "%\nBattery status:   ");
	strapp(&body, status);
	strapp(&body, "\nOptimus manager:  ");
	strapp(&body, mode_text_list[mode]);

	notify("Power", body, (char*)mode_icon_list[mode], NOTIFY_URGENCY_NORMAL, 1);
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
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	if (atoi(env) == 1) {
		char *capstr = uitoa(capacity);
		if (!capstr)
			return;
		send_notification(capstr, status ? status : "Unknown");
		free(capstr);
	}
}

static char *
getbatterypath(void)
{
	const char *base = "/sys/class/power_supply";
	DIR *d = opendir(base);
	struct dirent *e;
	char typepath[PATH_MAX], buf[32];

	if (!d) return NULL;

	while ((e = readdir(d))) {
		FILE *fp;

		if (e->d_name[0] == '.')
			continue;

		snprintf(typepath, sizeof(typepath), "%s/%s/type", base, e->d_name);
		fp = fopen(typepath, "r");
		if (!fp) continue;

		if (fgets(buf, sizeof(buf), fp)) {
			buf[strcspn(buf, "\n")] = 0;
			if (strcmp(buf, "Battery") == 0) {
				fclose(fp);
				closedir(d);
				snprintf(typepath, sizeof(typepath), "%s/%s", base, e->d_name);
				return strdup(typepath);
			}
		}
		fclose(fp);
	}

	closedir(d);
	return NULL;
}

static unsigned int
getcapacity(void)
{
	FILE *fp;
	char *bat;
	char path[PATH_MAX];
	unsigned int cap = 0;

	cap = 0;
	bat = getbatterypath();
	if (!bat)
		return 0;

	snprintf(path, sizeof(path), "%s/capacity", bat);
	free(bat);

	fp = fopen(path, "r");
	if (!fp) {
		logwrite("fopen() failed for file", path, LOG_ERROR, "dwmblocks-battery");
		return 0;
	}

	if (fscanf(fp, "%u", &cap) != 1)
		cap = 0;
	fclose(fp);

	if (cap > 100)
		cap = 100;

	return cap;
}

static char *
getstatus(void)
{
	FILE *fp;
	char *bat;
	char path[PATH_MAX];
	char buf[BUF_SIZE];

	bat = getbatterypath();
	if (!bat)
		return 0;

	snprintf(path, sizeof(path), "%s/status", bat);
	free(bat);


	fp = fopen(path, "r");
	if (!fp) {
		logwrite("fopen() failed for file", path, LOG_ERROR, "dwmblocks-battery");
		return 0;
	}

	buf[0] = '\0';
	if (!fgets(buf, sizeof(buf), fp))
		strcpy(buf, "Unknown");

	fclose(fp);

	trimtonewl(buf);
	return strdup(buf);
}

static unsigned int
battery_icon_index(unsigned int capacity)
{
	if (capacity < 10)
		return 0;
	if (capacity < 30)
		return 1;
	if (capacity < 50)
		return 2;
	if (capacity < 75)
		return 3;
	return 4;
}

int
main(void)
{
	char *status = getstatus();
	unsigned int capacity = getcapacity();
	const char *icon;

	execbutton(capacity, status);

	if (status && strcmp(status, "Charging") == 0)
		icon = icon_charging;
	else
		icon = icons_battery[battery_icon_index(capacity)];

	printf("%s\n" CLR_NRM, icon);

	free(status);
	return 0;
}
