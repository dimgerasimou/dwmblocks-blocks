/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_C
#define BUF_SIZE 256
#define LEN(a)   (sizeof(a) / sizeof((a)[0]))

#include "colors.h"
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
		die("fopen() failed for: \"/proc/meminfo\":");

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
		warn("Failed to parse MemTotal/MemAvailable");
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
		execute((char **)args_task_manager);
		break;
	default:
		break;
	}
}

int
main(void)
{
	const enum Color def_cols[] = { clr_mem };

	long used_kib;

	set_name("dwmblocks-memory");
	clr_init(def_cols, LEN(def_cols));

	execbutton();

	used_kib = getmemoryusage_kib();

	printf("%s", clr_get(clr_mem));
	if (show_icon)
		printf(" ");

	printf("%.1fGiB" CLR_NRM "\n", (double)used_kib / 1024.0 / 1024.0);
	return 0;
}
