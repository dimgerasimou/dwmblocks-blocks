#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#define KEYBOARD_C

#include "../include/colorscheme.h"
#include "../include/common.h"
#include "../include/config.h"

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
}

static void
execbutton(void)
{
	char *env = NULL;

	if (!(env = getenv("BLOCK_BUTTON")))
		return;

	switch(atoi(env)) {
	case 1:
	{
		char *path = get_path((char**) language_switch_path, 1);

		unsetenv("BLOCK_BUTTON");
		forkexecv(path, (char**) language_switch_args, "dwmblocks-keyboard");

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
	Display          *dpy = NULL;
	char             *tok = NULL;
	XkbStateRec      state;
	XkbRF_VarDefsRec vd;

	execbutton();

	if (!(dpy = XOpenDisplay(NULL)))
		logwrite("XOpenDisplay() failed", NULL, LOG_FATAL, "dwmblocks-keyboard");

	XkbGetState(dpy, XkbUseCoreKbd, &state);
	XkbRF_GetNamesProp(dpy, NULL, &vd);
	
	tok = strtok(vd.layout, ",");

	for (int i = 0; i < state.group; i++) {
		tok = strtok(NULL, ",");
		if (!tok) {
			logwrite("Invalid layout for given group", NULL, LOG_WARN, "dwmblocks-keyboard");
			goto cleanup;
		}
	}

	printf(CLR_5"   %s"NRM"\n", tok);

cleanup:
	XkbRF_FreeVarDefs(&vd);
	XCloseDisplay(dpy);

	return 0;
}
