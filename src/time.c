/* See LICENSE file for copyright and license details. */

#include "themes/catppuccin.h"
#include <stdio.h>
#include <time.h>

#define TIME_C

#include "config.h"
#include "colorscheme.h"

int
main(void)
{
	struct tm *lt = NULL;
	time_t     ct = 0;
	
	ct = time(NULL);
	lt = localtime(&ct);

	if (show_icon)
		printf(CLR_TIM " ");

	printf(CLR_TIM "%.2d:%.2d\n" CLR_NRM, lt->tm_hour, lt->tm_min);
	return 0;
}
