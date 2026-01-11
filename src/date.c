/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DATE_C
#define BUFFER_SIZE 64

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

const char *months_string[] = {"January",   "February", "March",     "April",
                               "May",       "June",     "July",      "August",
                               "September", "October",  "November", "December"};

const int  days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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

static char*
getcalendar(const int mday, const int wday, const int m, const int y)
{
	char buf[BUFFER_SIZE];
	char *ret  = NULL;
	int  fday  = 0;
	int  daysm = 0;
	int  n     = 0;

	fday  = getfirstday(mday, wday);
	daysm = getmonthdays(m, y);
	ret   = strdup("Mo Tu We Th Fr <span color='#F38BA8'>Sa Su</span>\n");

	for (int i = 0; i < fday; i++)
		strapp(&ret, "   ");

	for (int i = 1; i <= daysm; i++) {
		if (fday == 7) {
			fday = 0;
			strapp(&ret, "\n");
		}

		if (i == mday)
			n = snprintf(buf, sizeof(buf), "<span color='black' bgcolor='#F38BA8'>%2d</span> ", i);
		else if (fday == 5 || fday == 6)
			n = snprintf(buf, sizeof(buf), "<span color='#F38BA8'>%2d</span> ", i);
		else
			n = snprintf(buf, sizeof(buf), "%2d ", i);

		if (n < 0 || n >= (int)sizeof(buf)) {
			logwrite("snprintf() buffer overflow", NULL, LOG_ERROR, "dwmblocks-calendar");
			if (ret)
				free(ret);

			return NULL;
		}

		strapp(&ret, buf);
		fday++;
	}

	return ret;
}

static char*
getsummary(const int m, const int y)
{
	char buf[BUFFER_SIZE];
	int  size = 0;
	int  n    = 0;

	size = (15 + strlen(months_string[m])) / 2;

	n = snprintf(buf, sizeof(buf), "%*s %d", size, months_string[m], y);

	if (n < 0 || n >= (int)sizeof(buf)) {
		logwrite("snprintf() buffer overflow", NULL, LOG_ERROR, "dwmblocks-date");
		return NULL;
	}

	return strdup(buf);
}

static void
printcalendar(void)
{
	struct tm *lt   = NULL;
	char      *body = NULL;
	char      *sum  = NULL;
	time_t     ct   = 0;

	ct = time(NULL);
	lt = localtime(&ct);

	body = getcalendar(lt->tm_mday, lt->tm_wday, lt->tm_mon, lt->tm_year + 1900);
	sum  = getsummary(lt->tm_mon, lt->tm_year + 1900);

	if (!body || !sum) {
		if (body)
			free(body);
		if (sum)
			free(sum);
		return;
	}

	notify(sum, body, "calendar", NOTIFY_URGENCY_NORMAL, 0);

	free(body);
	free(sum);
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
		forkexecvp((char **) args_gui_calendar, "dwmblocks-date");
		break;

	default:
		break;
	}
}

int
main(void)
{
	struct tm *lt = NULL;
	time_t     ct = 0;

	execbutton();
	
	ct = time(NULL);
	lt = localtime(&ct);

	if (show_icon) {
		printf(CLR_DATE" ");
	}

	printf(CLR_DATE "%02d/%02d\n" CLR_NRM, lt->tm_mday, lt->tm_mon + 1);

	return 0;
}
