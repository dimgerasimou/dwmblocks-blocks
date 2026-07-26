/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BATTERY_C

#include "colors.h"
#include "utils.h"
#include "config.h"

#define LEN(a)    (sizeof(a) / sizeof((a)[0]))
#define BODY_SIZE 256

struct Icons {
	const char *str;
	enum Color  clr;
};

static const struct Icons icons[] = {
	{ " ", clr_bat_crt },
	{ " ", clr_bat_low },
	{ " ", clr_bat_nrm },
	{ " ", clr_bat_nrm },
	{ " ", clr_bat_nrm },
	{ " ", clr_bat_chg },
};

#ifdef POWER_MANAGEMENT
struct Optimus {
	const char *name;
	const char *icon;
};

static const struct Optimus optimus[] = {
	{ "Unmanaged",  "battery" },
	{ "Integrated", "intel" },
	{ "Hybrid",     "deepin-graphics-driver-manager" },
	{ "Nvidia",     "nvidia" }
};

static unsigned int
getmode(void)
{
	char  buf[256];
	FILE *ep;

	ep = popen("optimus-manager --status", "r");
	if (!ep) {
		warn("popen() for: \"optimus-manager --status\":");
		return 0;
	}

	buf[0] = '\0';
	while (fgets(buf, sizeof(buf), ep)) {
		if (strstr(buf, "Current"))
			break;
		buf[0] = '\0';
	}

	pclose(ep);

	if (strstr(buf, "integrated"))
		return 1;
	if (strstr(buf, "hybrid"))
		return 2;
	if (strstr(buf, "nvidia"))
		return 3;

	return 0;
}

static void
send_notification(const char *cap, const char *st)
{
	unsigned int mode = getmode();
	char         body[BODY_SIZE];
	int          n;

	n = snprintf(body, sizeof(body),
	             "Battery capacity: %s%%\n"
	             "Battery status:   %s\n"
	             "Optimus Manager:  %s",
	             cap, st, optimus[mode].name);

	if (n < 0 || (size_t)n >= sizeof(body))
		warn("notification body truncated");

	notify("Power", body, optimus[mode].icon);
}

#else /* POWER_MANAGEMENT */

static void
send_notification(const char *cap, const char *st)
{
	char body[BODY_SIZE];
	int  n;

	n = snprintf(body, sizeof(body),
	             "Battery capacity: %s%%\n"
	             "Battery status:   %s",
	             cap, st);

	if (n < 0 || (size_t)n >= sizeof(body))
		warn("notification body truncated");

	notify("Power", body, "battery");
}

#endif /* POWER_MANAGEMENT */

/*
 * Locates the first battery device in sysfs. On success writes its
 * directory into 'out' and returns 0; returns 1 if none was found.
 */
static int
getbatterypath(char *out, const size_t outsz)
{
	static const char base[] = "/sys/class/power_supply";

	DIR           *d;
	struct dirent *e;
	char           buf[64];
	char           path[PATH_MAX];

	d = opendir(base);
	if (!d)
		return 1;

	while ((e = readdir(d))) {
		FILE *fp;
		int   n;

		if (e->d_name[0] == '.')
			continue;

		n = snprintf(path, sizeof(path), "%s/%s/type", base, e->d_name);
		if (n < 0 || (size_t)n >= sizeof(path))
			continue;

		fp = fopen(path, "r");
		if (!fp)
			continue;

		if (fgets(buf, sizeof(buf), fp)) {
			buf[strcspn(buf, "\n")] = '\0';

			if (strcmp(buf, "Battery") == 0) {
				fclose(fp);
				closedir(d);

				n = snprintf(out, outsz, "%s/%s", base, e->d_name);
				return (n < 0 || (size_t)n >= outsz) ? 1 : 0;
			}
		}

		fclose(fp);
	}

	closedir(d);
	return 1;
}

/*
 * Reads the single-line sysfs attribute 'attr' under 'base' into 'out',
 * stripping the trailing newline. Returns 0 on success, 1 otherwise.
 */
static int
readattr(const char *base, const char *attr, char *out, const size_t outsz)
{
	FILE *fp;
	char  path[PATH_MAX];
	int   n;

	n = snprintf(path, sizeof(path), "%s/%s", base, attr);
	if (n < 0 || (size_t)n >= sizeof(path))
		return 1;

	fp = fopen(path, "r");
	if (!fp)
		return 1;

	if (!fgets(out, (int)outsz, fp)) {
		fclose(fp);
		return 1;
	}

	fclose(fp);

	out[strcspn(out, "\n")] = '\0';
	return 0;
}

static unsigned int
getcapacity(const char *base)
{
	char         buf[16];
	unsigned int cap;

	if (readattr(base, "capacity", buf, sizeof(buf)) != 0) {
		warn("failed to read capacity under %s", base);
		return 0;
	}

	if (sscanf(buf, "%u", &cap) != 1)
		return 0;

	return cap > 100 ? 100 : cap;
}

static void
getstatus(const char *base, char *out, const size_t outsz)
{
	if (readattr(base, "status", out, outsz) != 0) {
		warn("failed to read status under %s", base);
		snprintf(out, outsz, "Unknown");
	}
}

static void
execbutton(const unsigned int cap, const char *st)
{
	const char *env = getenv("BLOCK_BUTTON");
	char        c[12];

	if (!env || !*env)
		return;

	if (atoi(env) == 1) {
		uitoa(cap, c, sizeof(c));
		send_notification(c, st);
	}
}

static size_t
battery_icon_index(const unsigned int cap)
{
	if (cap < 10)
		return 0;
	if (cap < 30)
		return 1;
	if (cap < 50)
		return 2;
	if (cap < 75)
		return 3;
	return 4;
}

int
main(void)
{
	const enum Color def_cols[] = {
		clr_bat_nrm, clr_bat_low, clr_bat_chg, clr_bat_crt
	};

	char         base[PATH_MAX];
	char         st[64];
	unsigned int cap;
	size_t       i;

	set_name("dwmblocks-battery");
	clr_init(def_cols, LEN(def_cols));

	if (getbatterypath(base, sizeof(base)) != 0)
		die("no battery found under /sys/class/power_supply");

	cap = getcapacity(base);
	getstatus(base, st, sizeof(st));

	execbutton(cap, st);

	i = (strcmp(st, "Charging") == 0) ? 5 : battery_icon_index(cap);
	if (i >= LEN(icons))
		die("icon index %zu out of range", i);

	printf("%s%s" CLR_NRM "\n", clr_get(icons[i].clr), icons[i].str);

	return 0;
}

