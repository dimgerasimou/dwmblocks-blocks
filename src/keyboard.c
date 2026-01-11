/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#define KEYBOARD_C

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

static void
XkbRF_FreeVarDefs(XkbRF_VarDefsRec *var_defs)
{
	if (!var_defs)
		return;

	free(var_defs->model);
	free(var_defs->layout);
	free(var_defs->variant);
	free(var_defs->options);
	free(var_defs->extra_names);
	free(var_defs->extra_values);

	/* optional: make double-free impossible if called again */
	memset(var_defs, 0, sizeof(*var_defs));
}

/*
 * Return a newly allocated copy of the n-th (0-based) token in a comma-separated list.
 * Returns NULL if not found. Caller must free().
 */
static char *
nth_csv_token_dup(const char *s, unsigned int n)
{
	const char *p, *start;
	unsigned int i = 0;

	if (!s || !*s)
		return NULL;

	p = s;
	start = s;

	while (1) {
		if (*p == ',' || *p == '\0') {
			if (i == n) {
				size_t len = (size_t)(p - start);
				char *out = malloc(len + 1);
				if (!out)
					return NULL;
				memcpy(out, start, len);
				out[len] = '\0';
				return out;
			}
			if (*p == '\0')
				break;
			i++;
			start = p + 1;
		}
		p++;
	}

	return NULL;
}

static void
execbutton(void)
{
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 1: {
		char *path = getpath((char **)path_language_switch);
		if (!path || !*path) {
			logwrite("getpath() failed for language switch", NULL, LOG_WARN, "dwmblocks-keyboard");
			free(path);
			return;
		}
		forkexecv(path, (char **)args_language_switch, "dwmblocks-keyboard");
		free(path);
		break;
	}
	default:
		break;
	}
}

int
main(void)
{
	Display *dpy = NULL;
	XkbStateRec state;
	XkbRF_VarDefsRec vd = {0};
	char *layout = NULL;

	execbutton();

	dpy = XOpenDisplay(NULL);
	if (!dpy)
		logwrite("XOpenDisplay() failed", NULL, LOG_FATAL, "dwmblocks-keyboard");

	if (XkbGetState(dpy, XkbUseCoreKbd, &state) != Success) {
		logwrite("XkbGetState() failed", NULL, LOG_WARN, "dwmblocks-keyboard");
		goto cleanup;
	}

	if (!XkbRF_GetNamesProp(dpy, NULL, &vd) || !vd.layout || !*vd.layout) {
		logwrite("XkbRF_GetNamesProp() failed or layout missing", NULL, LOG_WARN, "dwmblocks-keyboard");
		goto cleanup;
	}

	layout = nth_csv_token_dup(vd.layout, (unsigned int)state.group);
	if (!layout || !*layout) {
		logwrite("Invalid layout for given group", NULL, LOG_WARN, "dwmblocks-keyboard");
		goto cleanup;
	}

	if (show_icon)
		printf(CLR_KBD " ");
	printf(CLR_KBD "%s\n" CLR_NRM, layout);

cleanup:
	free(layout);
	XkbRF_FreeVarDefs(&vd);
	if (dpy)
		XCloseDisplay(dpy);

	return 0;
}
