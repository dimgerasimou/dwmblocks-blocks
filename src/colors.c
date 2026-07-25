#include <string.h>
#include <stdio.h>

#include "colors.h"
#include "utils.h"

#ifdef NO_COLOR

char *
clr_get(enum Color clr)
{
	char *ret = ecalloc(1);
	ret[0] = '\0';
	return ret;
}

void
clr_init(const enum Color clrs[], size_t clrs_num)
{
	return;
}

#else

#include <X11/Xlib.h>
#include <X11/Xresource.h>

static char colors[CLR_SIZE][8] = {0};

static const char *names[CLR_SIZE] ={
	"clr_bat_crt",
	"clr_bat_low",
	"clr_bat_nrm",
	"clr_bat_chg",
	"clr_bt",
	"clr_date",
	"clr_net_nrm",
	"clr_net_err",
	"clr_krn_pkg",
	"clr_krn_nrm",
	"clr_kbd",
	"clr_mem",
	"clr_pwr",
	"clr_tim",
	"clr_vol_nrm",
	"clr_vol_mut"
};

static int
isxdigit(char c)
{
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

static int
hexverify(char *hex)
{
	if (strlen(hex) != 7)
		return 0;

	if (hex[0] != '#')
		return 0;

	for (size_t i = 1; i < 7; i++) {
		if (!isxdigit(hex[i]))
			return 0;
	}

	return 1;
}

static int
clr_load(XrmDatabase db, enum Color clr)
{
	XrmValue ret;
	char *type;
	char name[256];

	snprintf(name, sizeof(name), "dwmblocks.%s", names[clr]);
	name[sizeof(name) - 1] = '\0';
	if (XrmGetResource(db, name, "*", &type, &ret) && ret.addr) {
		if (hexverify(ret.addr)) {
			strncpy(colors[clr], ret.addr, 7);
			colors[clr][7] = '\0';
			return 1;
		}
	}

	snprintf(name, sizeof(name), "*%s", names[clr]);
	name[sizeof(name) - 1] = '\0';
	if (XrmGetResource(db, name, "*", &type, &ret) && ret.addr) {
		if (hexverify(ret.addr)) {
			strncpy(colors[clr], ret.addr, 7);
			colors[clr][7] = '\0';
			return 1;
		}
	}

	return 0;
}

char *
clr_get(enum Color clr)
{
	char *ret;

	if (!colors[clr]) {
		ret = emalloc(1);
		ret = malloc(1);
		ret[0] = '\0';
		return ret;
	}

	ret = emalloc(11);
	snprintf(ret, 11, "^c%s^", colors[clr]);
	return ret;
}

void
clr_init(const enum Color clrs[], size_t clrs_num)
{
	Display *dpy;
	char *resm;
	XrmDatabase db;

	if (!(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay:");

	XrmInitialize();

	resm = XResourceManagerString(dpy);
	db = resm ? XrmGetStringDatabase(resm) : NULL;

	for (size_t i = 0; i < clrs_num; i++) {
		clr_load(db, clrs[i]);
	}

	XCloseDisplay(dpy);
}

#endif
