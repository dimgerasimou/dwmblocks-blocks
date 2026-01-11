/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MEMORY_C
#define BUF_SIZE 256

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

static long
getmemoryusage_kib(void)
{
	char  buffer[BUF_SIZE];
	FILE *fp = NULL;
	long  total = -1;
	long  avail = -1;

	fp = fopen("/proc/meminfo", "r");
	if (!fp)
		logwrite("fopen() failed for", "/proc/meminfo", LOG_FATAL, "dwmblocks-memory");

	while (fgets(buffer, sizeof(buffer), fp)) {
		if (strncmp(buffer, "MemTotal:", 9) == 0) {
			if (sscanf(buffer + 9, "%ld", &total) != 1)
				total = -1;
		} else if (strncmp(buffer, "MemAvailable:", 13) == 0) {
			if (sscanf(buffer + 13, "%ld", &avail) != 1)
				avail = -1;
		}

		if (total >= 0 && avail >= 0)
			break;
	}

	fclose(fp);

	if (total < 0 || avail < 0) {
		logwrite("Failed to parse MemTotal/MemAvailable", NULL, LOG_WARN, "dwmblocks-memory");
		return 0;
	}

	/* used = total - available */
	if (avail > total)
		return 0;

	return total - avail;
}

static void
execbutton(void)
{
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 3:
		forkexecvp((char **)args_task_manager, "dwmblocks-memory");
		break;
	default:
		break;
	}
}

int
main(void)
{
	long used_kib;

	execbutton();

	used_kib = getmemoryusage_kib();

	if (show_icon)
		printf(CLR_MEM " ");

	printf(CLR_MEM "%.1fGiB\n" CLR_NRM, (double)used_kib / 1024.0 / 1024.0);
	return 0;
}
