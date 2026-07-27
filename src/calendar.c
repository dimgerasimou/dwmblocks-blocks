/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <string.h>

#include "calendar.h"

/* Colour used for weekend columns and the current day (Pango markup). */
#ifndef CAL_ACCENT
#define CAL_ACCENT "#F38BA8"
#endif

static const char *const months[] = {
	"January", "February", "March",     "April",
	"May",     "June",     "July",      "August",
	"September", "October", "November", "December"
};

static const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int
cal_firstday(int mday, int wday)
{
	while (mday > 7)
		mday -= 7;

	while (mday > 1) {
		mday--;
		wday--;
		if (wday == -1)
			wday = 6;
	}

	/* Shift from Sunday-first (tm_wday) to Monday-first. */
	wday--;
	if (wday == -1)
		wday = 6;

	return wday;
}

int
cal_monthdays(const int m, const int y)
{
	if (m < 0 || m > 11)
		return 0;

	if (m != 1)
		return days_in_month[m];

	if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
		return 29;

	return 28;
}

const char *
cal_monthname(const int m)
{
	if (m < 0 || m > 11)
		return NULL;

	return months[m];
}

int
cal_render(char *buf, const size_t bufsz, const char *accent,
           const int mday, const int wday, const int m, const int y)
{
	size_t off   = 0;
	int    fday  = 0;
	int    daysm = 0;
	int    n     = 0;

	if (!buf || bufsz == 0)
		return 1;

	daysm = cal_monthdays(m, y);
	if (daysm == 0)
		return 1;

	/* An unset colour renders the calendar without markup. */
	if (accent && !*accent)
		accent = NULL;

	fday = cal_firstday(mday, wday);

	if (accent)
		n = snprintf(buf, bufsz,
		             "Mo Tu We Th Fr <span color='%s'>Sa Su</span>\n", accent);
	else
		n = snprintf(buf, bufsz, "Mo Tu We Th Fr Sa Su\n");

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

		if (i == mday && accent)
			n = snprintf(buf + off, bufsz - off,
			             "<span color='black' bgcolor='%s'>%2d</span> ",
			             accent, i);
		else if (i == mday)
			n = snprintf(buf + off, bufsz - off, "<b>%2d</b> ", i);
		else if (accent && (fday == 5 || fday == 6))
			n = snprintf(buf + off, bufsz - off,
			             "<span color='%s'>%2d</span> ", accent, i);
		else
			n = snprintf(buf + off, bufsz - off, "%2d ", i);

		if (n < 0 || (size_t)n >= bufsz - off)
			return 1;
		off += (size_t)n;

		fday++;
	}

	return 0;
}

int
cal_heading(char *buf, const size_t bufsz, const int m, const int y)
{
	const char *name = cal_monthname(m);
	size_t      len;
	int         width;
	int         n;

	if (!buf || bufsz == 0 || !name)
		return 1;

	/*
	 * The rendered string is strlen(name) + 5 characters ("_YYYY"), so the
	 * leading pad that centres it is (CAL_WIDTH - (len + 5)) / 2. Folding
	 * that into a %*s field width gives len + (15 - len) / 2.
	 */
	len = strlen(name);
	width = (len < 15) ? (int)((15 - len) / 2 + len) : (int)len;

	n = snprintf(buf, bufsz, "%*s %d", width, name, y);

	if (n < 0 || (size_t)n >= bufsz)
		return 1;

	return 0;
}
