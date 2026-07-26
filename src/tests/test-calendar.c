/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "calendar.h"

static unsigned int checks;
static unsigned int failures;

#define CHECK(cond)                                                    \
	do {                                                           \
		checks++;                                              \
		if (!(cond)) {                                         \
			failures++;                                    \
			fprintf(stderr, "  FAIL %s:%d: %s\n",          \
			        __FILE__, __LINE__, #cond);            \
		}                                                      \
	} while (0)

/*
 * cal_firstday() is hand-rolled date arithmetic, so check it against the
 * C library for every day of a multi-year span rather than spot values.
 */
static void
test_firstday_matches_mktime(void)
{
	unsigned int mismatches = 0;

	for (int y = 2020; y <= 2030; y++) {
		for (int m = 0; m < 12; m++) {
			struct tm first = {0};
			int       want;

			first.tm_year = y - 1900;
			first.tm_mon  = m;
			first.tm_mday = 1;
			first.tm_hour = 12;
			mktime(&first);

			/* Monday-first index of the 1st of this month. */
			want = (first.tm_wday + 6) % 7;

			for (int d = 1; d <= cal_monthdays(m, y); d++) {
				struct tm cur = {0};

				cur.tm_year = y - 1900;
				cur.tm_mon  = m;
				cur.tm_mday = d;
				cur.tm_hour = 12;
				mktime(&cur);

				if (cal_firstday(d, cur.tm_wday) != want)
					mismatches++;
			}
		}
	}

	CHECK(mismatches == 0);
}

static void
test_monthdays(void)
{
	CHECK(cal_monthdays(0, 2025) == 31);   /* January */
	CHECK(cal_monthdays(3, 2025) == 30);   /* April */
	CHECK(cal_monthdays(11, 2025) == 31);  /* December */

	/* February: the three leap-year rules. */
	CHECK(cal_monthdays(1, 2023) == 28);   /* common */
	CHECK(cal_monthdays(1, 2024) == 29);   /* divisible by 4 */
	CHECK(cal_monthdays(1, 1900) == 28);   /* century, not by 400 */
	CHECK(cal_monthdays(1, 2000) == 29);   /* divisible by 400 */

	/* Out of range must not read past the table. */
	CHECK(cal_monthdays(-1, 2025) == 0);
	CHECK(cal_monthdays(12, 2025) == 0);
}

static void
test_monthname(void)
{
	CHECK(strcmp(cal_monthname(0), "January") == 0);
	CHECK(strcmp(cal_monthname(11), "December") == 0);
	CHECK(cal_monthname(-1) == NULL);
	CHECK(cal_monthname(12) == NULL);
}

static void
test_heading_centres(void)
{
	char buf[64];

	/* "January 2026" is 12 chars; centring in 20 gives 4 leading spaces. */
	CHECK(cal_heading(buf, sizeof(buf), 0, 2026) == 0);
	CHECK(strcmp(buf, "    January 2026") == 0);

	/* "September 2026" is 14 chars, so 3 leading spaces. */
	CHECK(cal_heading(buf, sizeof(buf), 8, 2026) == 0);
	CHECK(strcmp(buf, "   September 2026") == 0);

	CHECK(cal_heading(buf, sizeof(buf), 99, 2026) == 1);
}

static void
test_heading_overflow(void)
{
	char tiny[4];

	CHECK(cal_heading(tiny, sizeof(tiny), 0, 2026) == 1);
}

static void
test_render(void)
{
	char buf[2048];

	/* July 2026 starts on a Wednesday and has 31 days. */
	CHECK(cal_render(buf, sizeof(buf), 1, 3, 6, 2026) == 0);
	CHECK(strncmp(buf, "Mo Tu We Th Fr", 14) == 0);
	CHECK(strstr(buf, "31") != NULL);

	/* The current day is highlighted with a background span. */
	CHECK(strstr(buf, "bgcolor=") != NULL);

	/* An invalid month must fail rather than render garbage. */
	CHECK(cal_render(buf, sizeof(buf), 1, 3, 12, 2026) == 1);
}

static void
test_render_overflow(void)
{
	char tiny[16];

	CHECK(cal_render(tiny, sizeof(tiny), 1, 3, 6, 2026) == 1);
}

int
main(void)
{
	test_firstday_matches_mktime();
	test_monthdays();
	test_monthname();
	test_heading_centres();
	test_heading_overflow();
	test_render();
	test_render_overflow();

	printf("test-calendar: %u checks, %u failures\n", checks, failures);

	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
