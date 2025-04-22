/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MEMORY_C

#include "../include/colorscheme.h"
#include "../include/utils.h"
#include "../include/config.h"


static long
getmemoryusage(void)
{
	char  buffer[128];
	FILE *fp   = NULL;
	char *ptr  = NULL;
	long  used = 0;
	long  temp = 0;

	if (!(fp = fopen("/proc/meminfo", "r")))
		logwrite("fopen() failed for", "/proc/meminfo", LOG_FATAL, "dwmblocks-memory");

	while (fgets(buffer, sizeof(buffer), fp)) {

		if (strstr(buffer, "MemTotal")) {
			ptr = (strchr(buffer, ':')) + 1;
			sscanf(ptr, "%ld", &temp);
			used += temp;
			continue;
		}

		if (strstr(buffer, "MemFree")) {
			ptr = (strchr(buffer, ':')) + 1;
			sscanf(ptr, "%ld", &temp);
			used -= temp;
			continue;
		}

		if (strstr(buffer, "Buffers")) {
			ptr = (strchr(buffer, ':')) + 1;
			sscanf(ptr, "%ld", &temp);
			used -= temp;
			continue;
		}

		if (strstr(buffer, "Cached")) {
			ptr = (strchr(buffer, ':')) + 1;
			sscanf(ptr, "%ld", &temp);
			used -= temp;
			break;
		}
	}

	fclose(fp);
	return used;
}

static void
execbutton(void)
{
	char *env = NULL;

	if (!(env = getenv("BLOCK_BUTTON")))
		return;

	switch(atoi(env)) {
	case 3:
		forkexecvp((char**) args_task_manager, "dwmblocks-memory");
		break;
	
	default:
		break;
	}
}

int
main(void)
{
	execbutton();
	printf(CLR_3"   %.1lfGiB"NRM"\n", ((getmemoryusage())/1024.0)/1024.0);
	return 0;
}
