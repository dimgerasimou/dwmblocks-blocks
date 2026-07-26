/* See LICENSE file for copyright and license details. */

#ifndef CALENDAR_H
#define CALENDAR_H

#include <stddef.h>

/* Width in columns of a rendered calendar body ("Mo Tu We Th Fr Sa Su"). */
#define CAL_WIDTH 20

/*
 * Returns the weekday index of the first day of the month, Monday = 0,
 * given any 'mday' in that month and its 'wday' (Sunday = 0, as tm_wday).
 */
int cal_firstday(int mday, int wday);

/* Returns the number of days in month 'm' (0 = January) of year 'y'. */
int cal_monthdays(const int m, const int y);

/* Returns the English name of month 'm' (0 = January), or NULL if out of range. */
const char *cal_monthname(const int m);

/*
 * Renders the month as Pango markup into 'buf', one week per line,
 * Monday first, with 'mday' highlighted.
 * Returns 0 on success, 1 if the buffer was too small.
 */
int cal_render(char *buf, const size_t bufsz, const int mday, const int wday,
               const int m, const int y);

/*
 * Builds "<Month> <year>", padded so it centres over a CAL_WIDTH body.
 * Returns 0 on success, 1 if the buffer was too small.
 */
int cal_heading(char *buf, const size_t bufsz, const int m, const int y);

#endif /* CALENDAR_H */
