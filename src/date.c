/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATE_C
#define BUFFER_SIZE 64
#define CAL_SIZE    2048
#define LEN(a)      (sizeof(a) / sizeof((a)[0]))

/* Colour used for weekend columns and the current day (Pango markup). */
#ifndef CAL_ACCENT
#define CAL_ACCENT "#F38BA8"
#endif

#include "colors.h"
#include "utils.h"
#include "config.h"

static const char *const months_string[] = {"January",   "February", "March",     "April",
                               "May",       "June",     "July",      "August",
                               "September", "October",  "November", "December"};

static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int
getfirstday(int mday, int wday)
{
	while (mday > 7)
		mday -= 7;

	while (mday > 1) {
		mday--;
		wday--;
		if (wday == -1)
			wday = 6;
	}

	wday--;
	if (wday == -1)
		wday = 6;

	return wday;
}

static int
getmonthdays(const int m, const int y)
{
	if (m != 1)
		return days_in_month[m];
	if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
		return 29;

	return 28;
}

/*
 * Renders the month as Pango markup into 'buf', one week per line,
 * Monday first. Returns 0 on success, 1 if the buffer was too small.
 */
static int
getcalendar(char *buf, const size_t bufsz, const int mday, const int wday,
            const int m, const int y)
{
	size_t off   = 0;
	int    fday  = 0;
	int    daysm = 0;
	int    n     = 0;

	fday  = getfirstday(mday, wday);
	daysm = getmonthdays(m, y);

	n = snprintf(buf, bufsz,
	             "Mo Tu We Th Fr <span color='" CAL_ACCENT "'>Sa Su</span>\n");
	if (n < 0 || (size_t)n >= bufsz)
		return 1;
	off = (size_t)n;

	for (int i = 0; i < fday; i++) {
		n = snprintf(buf + off, bufsz - off, "   ");
		if (n < 0 || (size_t)n >= bufsz - off)
			return 1;
		off += (size_t)n;
	}

	for (int i = 1; i <= daysm; i++) {
		if (fday == 7) {
			fday = 0;
			n = snprintf(buf + off, bufsz - off, "\n");
			if (n < 0 || (size_t)n >= bufsz - off)
				return 1;
			off += (size_t)n;
		}

		if (i == mday)
			n = snprintf(buf + off, bufsz - off,
			             "<span color='black' bgcolor='" CAL_ACCENT "'>%2d</span> ", i);
		else if (fday == 5 || fday == 6)
			n = snprintf(buf + off, bufsz - off,
			             "<span color='" CAL_ACCENT "'>%2d</span> ", i);
		else
			n = snprintf(buf + off, bufsz - off, "%2d ", i);

		if (n < 0 || (size_t)n >= bufsz - off)
			return 1;
		off += (size_t)n;

		fday++;
	}

	return 0;
}

/*
 * Builds "<Month> <year>" padded so it centres over the 20-column body.
 * The rendered string is always strlen(month) + 5 characters ("_YYYY"),
 * so the leading pad is (CAL_WIDTH - (len + 5)) / 2 == (15 - len) / 2,
 * which is what the %*s field width below produces.
 * Returns 0 on success, 1 if the buffer was too small.
 */
static int
getsummary(char *buf, const size_t bufsz, const int m, const int y)
{
	size_t len  = strlen(months_string[m]);
	int    size = 0;
	int    n    = 0;

	size = (len < 15) ? (int)((15 - len) / 2 + len) : (int)len;

	n = snprintf(buf, bufsz, "%*s %d", size, months_string[m], y);

	if (n < 0 || (size_t)n >= bufsz)
		return 1;

	return 0;
}

static void
printcalendar(void)
{
	struct tm *lt = NULL;
	char       body[CAL_SIZE];
	char       sum[BUFFER_SIZE];
	time_t     ct = 0;

	ct = time(NULL);
	lt = localtime(&ct);
	if (!lt) {
		warn("localtime:");
		return;
	}

	if (getcalendar(body, sizeof(body), lt->tm_mday, lt->tm_wday,
	                lt->tm_mon, lt->tm_year + 1900) != 0 ||
	    getsummary(sum, sizeof(sum), lt->tm_mon, lt->tm_year + 1900) != 0) {
		warn("failed to render calendar");
		return;
	}

	notify(sum, body, "calendar");
}

static void
execbutton(void)
{
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 1:
		printcalendar();
		break;

	case 3:
		execute((char **)args_gui_calendar);
		break;

	default:
		break;
	}
}

int
main(void)
{
	const enum Color def_cols[] = { clr_date };

	struct tm *lt = NULL;
	time_t     ct = 0;

	set_name("dwmblocks-date");
	clr_init(def_cols, LEN(def_cols));

	execbutton();

	ct = time(NULL);
	lt = localtime(&ct);
	if (!lt)
		die("localtime:");

	printf("%s", clr_get(clr_date));

	if (show_icon)
		printf(" ");

	printf("%02d/%02d" CLR_NRM "\n", lt->tm_mday, lt->tm_mon + 1);

	return 0;
}
