/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "utils.h"

#ifdef NO_COLOR

void
clr_init(void)
{
}

const char *
clr_get(enum Color clr)
{
	(void)clr;
	return "";
}

#else /* NO_COLOR */

#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>

/* Rendered escape: "^c" + "#RRGGBB" + "^" + NUL. */
#define CLR_LEN   11
#define CACHE_NAME "dwmblocks-colors"

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

/*
 * Builds the cache path in $XDG_RUNTIME_DIR, falling back to /tmp keyed by
 * uid. Returns 0 on success.
 */
static int
cachepath(char *out, const size_t outsz)
{
	const char *dir = getenv("XDG_RUNTIME_DIR");
	int         n;

	if (dir && *dir)
		n = snprintf(out, outsz, "%s/%s", dir, CACHE_NAME);
	else
		n = snprintf(out, outsz, "/tmp/%s-%u", CACHE_NAME,
		             (unsigned int)getuid());

	return (n < 0 || (size_t)n >= outsz) ? 1 : 0;
}

/*
 * The cache is stale if ~/.Xresources exists and is newer than it, which
 * covers the usual "edit the file, run xrdb" workflow. $XDG_RUNTIME_DIR is
 * cleared at logout, so a fresh session always rebuilds.
 */
static int
cachestale(const char *cache)
{
	struct stat cst, xst;
	const char *home;
	char        xres[PATH_MAX];
	int         n;

	if (stat(cache, &cst) != 0)
		return 1;

	home = getenv("HOME");
	if (!home || !*home)
		return 0;

	n = snprintf(xres, sizeof(xres), "%s/.Xresources", home);
	if (n < 0 || (size_t)n >= sizeof(xres))
		return 0;

	if (stat(xres, &xst) != 0)
		return 0;

	return xst.st_mtime > cst.st_mtime;
}

/*
 * Reads one escape per colour, in enum order. Every line must be either
 * empty or exactly "^c#RRGGBB^"; anything else means the cache is corrupt
 * and the whole thing is rejected, so a bad file can never reach the bar.
 * Returns 0 on success.
 */
static int
cacheload(const char *path)
{
	char  line[CLR_LEN + 2];
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp)
		return 1;

	for (size_t i = 0; i < CLR_SIZE; i++) {
		char   hex[8];
		size_t len;

		if (!fgets(line, sizeof(line), fp))
			goto corrupt;

		line[strcspn(line, "\n")] = '\0';
		len = strlen(line);

		/* An unset colour is stored as an empty line. */
		if (len == 0) {
			colors[i][0] = '\0';
			continue;
		}

		if (len != CLR_LEN - 1 || line[0] != '^' || line[1] != 'c' ||
		    line[len - 1] != '^')
			goto corrupt;

		memcpy(hex, line + 2, sizeof(hex) - 1);
		hex[sizeof(hex) - 1] = '\0';

		if (!ishexcolor(hex))
			goto corrupt;

		memcpy(colors[i], line, len + 1);
	}

	fclose(fp);
	return 0;

corrupt:
	fclose(fp);
	memset(colors, 0, sizeof(colors));
	return 1;
}

/* Written via a temporary file so concurrent blocks never see a partial cache. */
static void
cachestore(const char *path)
{
	char  tmp[PATH_MAX];
	FILE *fp;
	int   fd, n;

	n = snprintf(tmp, sizeof(tmp), "%s.XXXXXX", path);
	if (n < 0 || (size_t)n >= sizeof(tmp))
		return;

	fd = mkstemp(tmp);
	if (fd < 0)
		return;

	fp = fdopen(fd, "w");
	if (!fp) {
		close(fd);
		unlink(tmp);
		return;
	}

	for (size_t i = 0; i < CLR_SIZE; i++)
		fprintf(fp, "%s\n", colors[i]);

	if (fclose(fp) != 0 || rename(tmp, path) != 0)
		unlink(tmp);
}

/*
 * Looks 'clr' up in 'db' under "dwmblocks.<name>" then "*<name>", storing
 * the rendered escape on the first valid hit.
 */
static void
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

		if (!ishexcolor(ret.addr))
			continue;

		snprintf(colors[clr], sizeof(colors[clr]), "^c%s^", ret.addr);
		return;
	}
}

/* Resolves every colour from X. Returns 0 if the display was reachable. */
static int
clr_loadall(void)
{
	Display     *dpy;
	XrmDatabase  db;
	char        *resm;

	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		warn("XOpenDisplay() failed, rendering without colour");
		return 1;
	}

	XrmInitialize();

	resm = XResourceManagerString(dpy);
	db   = resm ? XrmGetStringDatabase(resm) : NULL;

	if (db) {
		for (size_t i = 0; i < CLR_SIZE; i++)
			clr_load(db, (enum Color)i);

		XrmDestroyDatabase(db);
	}

	XCloseDisplay(dpy);
	return 0;
}

void
clr_init(void)
{
	char path[PATH_MAX];
	int  havepath;

	havepath = (cachepath(path, sizeof(path)) == 0);

	if (havepath && !cachestale(path) && cacheload(path) == 0)
		return;

	if (clr_loadall() != 0)
		return;

	if (havepath)
		cachestore(path);
}

const char *
clr_get(enum Color clr)
{
	if ((unsigned int)clr >= (unsigned int)CLR_SIZE)
		return "";

	return colors[clr];
}

#endif /* NO_COLOR */
