/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <time.h>

#define TIME_C
#define LEN(a) (sizeof(a) / sizeof((a)[0]))

#include "config.h"
#include "utils.h"
#include "colors.h"

int
main(void)
{
	const enum Color def_cols[] = { clr_tim };

	struct tm *lt = NULL;
	time_t     ct = 0;

	set_name("dwmblocks-time");
	clr_init(def_cols, LEN(def_cols));

	ct = time(NULL);
	lt = localtime(&ct);
	if (!lt)
		die("localtime:");

	printf("%s", clr_get(clr_tim));

	if (show_icon)
		printf(" ");

	printf("%.2d:%.2d" CLR_NRM "\n", lt->tm_hour, lt->tm_min);
	return 0;
}
