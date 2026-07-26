/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <string.h>

#include "colors.h"
#include "utils.h"

#ifdef NO_COLOR

void
clr_init(const enum Color clrs[], size_t clrs_num)
{
	(void)clrs;
	(void)clrs_num;
}

const char *
clr_get(enum Color clr)
{
	(void)clr;
	return "";
}

#else /* NO_COLOR */

#include <X11/Xlib.h>
#include <X11/Xresource.h>

/* Rendered escape: "^c" + "#RRGGBB" + "^" + NUL. */
#define CLR_LEN  11
#define HEX_LEN  7

static char colors[CLR_SIZE][CLR_LEN] = {0};

static const char *const names[CLR_SIZE] = {
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
ishexdigit(char c)
{
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

/* Accepts exactly "#RRGGBB". */
static int
hexverify(const char *hex)
{
	if (!hex || strlen(hex) != HEX_LEN)
		return 0;

	if (hex[0] != '#')
		return 0;

	for (size_t i = 1; i < HEX_LEN; i++) {
		if (!ishexdigit(hex[i]))
			return 0;
	}

	return 1;
}

/*
 * Looks 'clr' up in 'db' under "dwmblocks.<name>" then "*<name>", storing
 * the rendered escape on the first valid hit. Returns 1 if one was found.
 */
static int
clr_load(XrmDatabase db, enum Color clr)
{
	static const char *const prefixes[] = { "dwmblocks.", "*" };

	XrmValue ret;
	char    *type = NULL;
	char     name[256];

	for (size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); i++) {
		snprintf(name, sizeof(name), "%s%s", prefixes[i], names[clr]);

		if (!XrmGetResource(db, name, "*", &type, &ret) || !ret.addr)
			continue;

		if (!hexverify(ret.addr))
			continue;

		snprintf(colors[clr], sizeof(colors[clr]), "^c%s^", ret.addr);
		return 1;
	}

	return 0;
}

void
clr_init(const enum Color clrs[], size_t clrs_num)
{
	Display     *dpy;
	XrmDatabase  db;
	char        *resm;

	if (!(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay:");

	XrmInitialize();

	resm = XResourceManagerString(dpy);
	db   = resm ? XrmGetStringDatabase(resm) : NULL;

	if (db) {
		for (size_t i = 0; i < clrs_num; i++) {
			if ((unsigned int)clrs[i] < (unsigned int)CLR_SIZE)
				clr_load(db, clrs[i]);
		}

		XrmDestroyDatabase(db);
	}

	XCloseDisplay(dpy);
}

const char *
clr_get(enum Color clr)
{
	if ((unsigned int)clr >= (unsigned int)CLR_SIZE)
		return "";

	return colors[clr];
}

#endif /* NO_COLOR */
