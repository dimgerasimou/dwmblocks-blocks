/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <time.h>

#define TIME_C

#include "config.h"
#include "utils.h"
#include "colors.h"

int
main(void)
{
	struct tm *lt = NULL;
	time_t     ct = 0;

	set_name("dwmblocks-time");
	const enum Color def_cols[] = {clr_tim};
	clr_init(def_cols, 1);
	
	ct = time(NULL);
	lt = localtime(&ct);

	printf("%s", clr_get(clr_tim));

	if (show_icon)
		printf(" ");

	printf("%.2d:%.2d" CLR_NRM "\n", lt->tm_hour, lt->tm_min);
	return 0;
}
