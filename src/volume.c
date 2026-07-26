/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <math.h>
#include <pulse/pulseaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOLUME_C
#define LEN(a) (sizeof(a) / sizeof((a)[0]))

#include "colors.h"
#include "utils.h"
#include "config.h"

typedef struct {
	unsigned int volume;
	unsigned int mute;
	unsigned int done;
} AudioInfo;

/* [0] = default sink, [1] = default source. */
#define AI_SINK   0
#define AI_SOURCE 1
#define AI_COUNT  2

static const char *icons[] = {" ", " ", " ", " ", ""};

static pa_mainloop *ml = NULL;
static pa_context *ctx = NULL;
static pa_mainloop_api *mlapi = NULL;

static void contextcb(pa_context *c, void *userdata);
static void execbutton(AudioInfo *a);
static void freepa(void);
static void getaudioinfo(AudioInfo *a);
static void initpa(void);
static void propnotify(const AudioInfo *a);
static void servercb(pa_context *c, const pa_server_info *i, void *userdata);
static void sinkcb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
static void sourcecb(pa_context *c, const pa_source_info *i, int eol, void *userdata);
static void quitml(const AudioInfo *a);

static void
contextcb(pa_context *c, void *userdata)
{
	pa_operation *o;

	switch (pa_context_get_state(c)) {
	case PA_CONTEXT_READY:
		if ((o = pa_context_get_server_info(c, servercb, userdata)))
			pa_operation_unref(o);
		break;

	case PA_CONTEXT_FAILED:
	case PA_CONTEXT_TERMINATED:
		pa_mainloop_quit(ml, 1);
		break;

	default:
		break;
	}
}

static void
execbutton(AudioInfo *a)
{
	const char *env;
	char        path[PATH_MAX];

	env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 1:
		propnotify(a);
		break;

	case 2:
		execute((char **)args_eqalizer);
		break;

	case 3:
		if (getpath(path_volume_control, path, sizeof(path)) == 0)
			executepath(path, (char **)args_volume_mute);
		break;

	case 4:
		if (getpath(path_volume_control, path, sizeof(path)) == 0)
			executepath(path, (char **)args_volume_increase);
		break;

	case 5:
		if (getpath(path_volume_control, path, sizeof(path)) == 0)
			executepath(path, (char **)args_volume_decrase);
		break;

	default:
		break;
	}
}

static void
freepa(void)
{
	if (ctx) {
		pa_context_disconnect(ctx);
		pa_context_unref(ctx);
		ctx = NULL;
	}
	if (ml) {
		pa_mainloop_free(ml);
		ml = NULL;
	}
}

static void
getaudioinfo(AudioInfo *a)
{
	initpa();

	pa_context_set_state_callback(ctx, contextcb, a);
	pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

	pa_mainloop_run(ml, NULL);

	freepa();
}

static void
initpa(void)
{
	if (!(ml = pa_mainloop_new()))
		die("pa_mainloop_new() failed to initialize");
	if (!(mlapi = pa_mainloop_get_api(ml)))
		die("pa_mainloop_get_api() failed to initialize");
	if (!(ctx = pa_context_new(mlapi, "dwmblocks-volume")))
		die("pa_context_new() failed to initialize");
}

static void
propnotify(const AudioInfo *a)
{
	char   body[256];
	size_t off = 0;
	int    n;

	if (a[AI_SINK].done == 1)
		n = snprintf(body, sizeof(body), " Volume: %3u%%, Muted: %s\n",
		             a[AI_SINK].volume, a[AI_SINK].mute ? "Yes" : "No");
	else
		n = snprintf(body, sizeof(body), "No audio sink detected.\n");

	if (n < 0 || (size_t)n >= sizeof(body)) {
		warn("notification body truncated");
		return;
	}
	off = (size_t)n;

	if (a[AI_SOURCE].done == 1)
		n = snprintf(body + off, sizeof(body) - off, " Volume: %3u%%, Muted: %s\n",
		             a[AI_SOURCE].volume, a[AI_SOURCE].mute ? "Yes" : "No");
	else
		n = snprintf(body + off, sizeof(body) - off, "No audio source detected.\n");

	if (n < 0 || (size_t)n >= sizeof(body) - off)
		warn("notification body truncated");

	notify("Audio Properties", body, "audio-headphones");
}

static void
servercb(pa_context *c, const pa_server_info *i, void *userdata)
{
	pa_operation *o;

	if (!i)
		return;

	if ((o = pa_context_get_sink_info_by_name(c, i->default_sink_name, sinkcb, userdata)))
		pa_operation_unref(o);
	if ((o = pa_context_get_source_info_by_name(c, i->default_source_name, sourcecb, userdata)))
		pa_operation_unref(o);
}

/* Converts a PulseAudio volume to a 0..100 percentage. */
static unsigned int
volpercent(const pa_cvolume *cv)
{
	double pct = (pa_cvolume_avg(cv) * 100.0) / PA_VOLUME_NORM;

	if (pct < 0.0)
		return 0;
	if (pct > 100.0)
		return 100;

	return (unsigned int)(pct + 0.5);
}

static void
sinkcb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	AudioInfo *a = userdata;

	(void)c;

	if (eol > 0)
		return;

	if (!i) {
		a[AI_SINK].done = 2;
		quitml(a);
		return;
	}

	a[AI_SINK].volume = volpercent(&i->volume);
	a[AI_SINK].mute   = i->mute ? 1u : 0u;
	a[AI_SINK].done   = 1;

	quitml(a);
}

static void
sourcecb(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	AudioInfo *a = userdata;

	(void)c;

	if (eol > 0)
		return;

	if (!i) {
		a[AI_SOURCE].done = 2;
		quitml(a);
		return;
	}

	a[AI_SOURCE].volume = volpercent(&i->volume);
	a[AI_SOURCE].mute   = i->mute ? 1u : 0u;
	a[AI_SOURCE].done   = 1;

	quitml(a);
}

static void
quitml(const AudioInfo *a)
{
	if (a[AI_SINK].done && a[AI_SOURCE].done)
		pa_mainloop_quit(ml, 0);
}

int
main(void)
{
	const enum Color def_cols[] = { clr_vol_mut, clr_vol_nrm };

	AudioInfo    a[AI_COUNT] = {{0, 0, 0}, {0, 0, 0}};
	char         v[16] = "";
	const char  *icon = icons[4];
	unsigned int volume;
	unsigned int mute;

	set_name("dwmblocks-volume");
	clr_init(def_cols, LEN(def_cols));

	getaudioinfo(a);
	execbutton(a);

	if (a[AI_SINK].done == 1) {
		mute   = a[AI_SINK].mute;
		volume = a[AI_SINK].volume;
	} else {
		mute   = 1;
		volume = 0;
	}

	if (display_type != 2) {
		if (volume > 66)
			icon = icons[3];
		else if (volume > 33)
			icon = icons[2];
		else
			icon = icons[1];

		if (mute)
			icon = icons[0];
	}

	if (display_type != 1) {
		int pad = volume_padding ? 3 : 0;
		int n   = snprintf(v, sizeof(v), "%*u%%", pad, volume);

		if (n < 0 || (size_t)n >= sizeof(v))
			warn("volume string truncated");
	}

	printf("%s%s%s" CLR_NRM "\n",
	       clr_get(mute ? clr_vol_mut : clr_vol_nrm), icon, v);

	return 0;
}
