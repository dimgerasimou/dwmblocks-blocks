/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <time.h>

#define DATE_C
#define HEAD_SIZE 64
#define CAL_SIZE  2048
#define LEN(a)    (sizeof(a) / sizeof((a)[0]))

#include "calendar.h"
#include "colors.h"
#include "utils.h"
#include "config.h"

static void
on_left(void *ctx)
{
	struct tm *lt = NULL;
	char       body[CAL_SIZE];
	char       head[HEAD_SIZE];
	time_t     ct = 0;

	(void)ctx;

	ct = time(NULL);
	lt = localtime(&ct);
	if (!lt) {
		warn("localtime:");
		return;
	}

	if (cal_render(body, sizeof(body), clr_hex(clr_cal), lt->tm_mday,
	               lt->tm_wday, lt->tm_mon, lt->tm_year + 1900) != 0 ||
	    cal_heading(head, sizeof(head), lt->tm_mon, lt->tm_year + 1900) != 0) {
		warn("failed to render calendar");
		return;
	}

	notify(head, body, "calendar");
}

static void
on_right(void *ctx)
{
	(void)ctx;
	execute((char **)args_gui_calendar);
}

int
main(void)
{
	static const struct Button buttons[] = {
		{ 1, on_left },
		{ 3, on_right },
	};

	struct tm *lt = NULL;
	time_t     ct = 0;

	set_name("dwmblocks-date");
	clr_init();

	dispatch(buttons, LEN(buttons), NULL);

	ct = time(NULL);
	lt = localtime(&ct);

	printf("%s", clr_get(clr_date));

	if (show_icon)
		printf("%s", icon_date);

	if (lt)
		printf("%02d/%02d" CLR_NRM "\n", lt->tm_mday, lt->tm_mon + 1);
	else {
		warn("localtime:");
		printf("--/--" CLR_NRM "\n");
	}

	return 0;
}
