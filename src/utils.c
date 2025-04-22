/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <libnotify/notify.h>
#include <linux/limits.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdint.h>

#define UTILS_C

#include "../include/utils.h"
#include "../include/config.h"


/* file specific functions */

static char*
formatsummary(const char *summary, const char *body)
{
	unsigned int count = 0;
	unsigned int max_count = 0;
	unsigned int total;
	char         *ret;

	for (char *ptr = (char*) body; *ptr != '\0'; ptr++) {
		if (*ptr == '\n') {
			max_count = count > max_count ? count : max_count;
			count = 0;
			continue;
		}
		count++;
	}

	max_count = count > max_count ? count : max_count;
	count = (max_count - strlen(summary)) / 2;
	total = count + strlen(summary);

	ret = malloc((total + 1) * sizeof(char));

	sprintf(ret, "%*s", total, summary);

	return ret;
}

static int
strtouint64(const char *input, uint64_t *output)
{
	char     *endptr;
	uint64_t val;

	errno = 0;
	val = strtoull(input, &endptr, 10);

	if (errno > 0)
		return -1;
	if (!endptr || endptr == input || *endptr != 0)
		return -EINVAL;
	if (val != 0 && input[0] == '-')
		return -ERANGE;

	*output = val;
	return 0;
}

/* header functions */

void
forkexecv(const char *path, char **args, const char *argv0)
{
	switch (fork()) {
	case -1:
		logwrite("fork() failed", NULL, LOG_FATAL, argv0);
		break;

	case 0:
		setsid();
		execv(path, args);
		logwrite("execv() failed for", args[0], LOG_FATAL, argv0);
		break;

	default:
		break;
	}
}

void
forkexecvp(char **args, const char *argv0)
{
	switch (fork()) {
	case -1:
		logwrite("fork() failed", NULL, LOG_FATAL, argv0);
		break;

	case 0:
		setsid();
		execvp(args[0], args);
		logwrite("execvp() failed for", args[0], LOG_FATAL, argv0);
		break;

	default:
		break;
	}
}

char*
getpath(char **path_array)
{
	char path[PATH_MAX];
	char name[NAME_MAX];

	path[0] = '\0';

	for (int i = 0; path_array[i] != NULL; i++) {
		if (path_array[i][0] == '$') {
			const char *ptr = path_array[i] + 1;
			char *env = getenv(ptr);

			if (!env) {
				fprintf(stderr, "Failed to get env variable:%s - %s\n", path_array[i], strerror(errno));
				exit(errno);
			}

			sprintf(name, "%s", env);
			strcat(path, name);
		} else {
			sprintf(name, "/%s", path_array[i]);
			strcat(path, name);
		}
	}

	return strdup(path);
}

pid_t
getpidof(const char *process, const char *argv0)
{
	char          buffer[PATH_MAX];
	struct dirent *ent;
	DIR           *dir;
	FILE          *fp;
	uint64_t      pid;
	pid_t         ret;

	dir = opendir("/proc");
	ret = 0;

	if (!dir) {
		errno = ENOENT;
		logwrite("opendir() failed for directory", "/proc", LOG_ERROR, argv0);
		return -ENOENT;
	}

	while ((ent = readdir(dir)) && ret >= 0) {
		if (strtouint64(ent->d_name, &pid) < 0)
			continue;

		snprintf(buffer, sizeof(buffer), "/proc/%s/cmdline", ent->d_name);
		fp = fopen(buffer, "r");
		if (!fp)
			continue;
		if (!fgets(buffer, sizeof(buffer), fp))
			continue;
		if ((strcmp(buffer, process) == 0)) {
			ret = ret ? -EEXIST : (pid_t) pid;
		}
	}

	closedir(dir);

	if (ret == -EEXIST) {
		errno = EEXIST;
		return -EEXIST;
	} else if (ret == 0) {
		errno = ENOENT;
		return -ENOENT;
	}

	return ret;
}

int
getxmenuopt(const char *menu, const char *argv0)
{
	int  option;
	int  writepipe[2];
	int  readpipe[2];
	char buffer[16];

	option = -EREMOTEIO;
	buffer[0] = '\0';

	if (pipe(writepipe) < 0 || pipe(readpipe) < 0) {
		logwrite("pipe() failed", NULL, LOG_FATAL, argv0);
	}
	
	switch (fork()) {
		case -1:
			logwrite("fork() failed", NULL, LOG_FATAL, argv0);
			break;

		case 0: /* child - xmenu */
			close(writepipe[1]);
			close(readpipe[0]);

			dup2(writepipe[0], STDIN_FILENO);
			close(writepipe[0]);

			dup2(readpipe[1], STDOUT_FILENO);
			close(readpipe[1]);
			
			execl("/usr/bin/xmenu", "xmenu", NULL);
			exit(EXIT_FAILURE);

		default: /* parent */
			close(writepipe[0]);
			close(readpipe[1]);

			write(writepipe[1], menu, strlen(menu) + 1);
			close(writepipe[1]);

			wait(NULL);

			read(readpipe[0], buffer, sizeof(buffer));
			close(readpipe[0]);
	}

	if (buffer[0] != '\0')
		sscanf(buffer, "%d", &option);

	return option;
}

pid_t
killstr(const char *procname, const int signo, const char *argv0)
{
	pid_t pID = getpidof(procname, argv0);

	if (pID > 0) {
		kill(pID, signo);
		return 0;
	}

	return pID;
}

void
logwrite(const char *message, const char *name, const log_level level, const char *argv0)
{
	struct tm *time_info = NULL;
	time_t    raw_time   = 0;
	FILE      *fp        = NULL;
	char      *path  = NULL;
	uint      err = errno;

	if (!message)
		return;

	path = getpath((char**) path_log);
	fp = fopen(path, "a");

	if (!fp) {
		fprintf(stderr, "dwmblocks - fopen() failed for path: %s - %s\n", path, strerror(errno));
		return;
	}

	time(&raw_time);
	time_info = localtime(&raw_time);

	fprintf(fp, "[%d-%02d-%02d %02d:%02d:%02d] ", time_info->tm_year+1900,
	        time_info->tm_mon+1, time_info->tm_mday, time_info->tm_hour,
	        time_info->tm_min, time_info->tm_sec);

	switch (level) {
	case LOG_INFO:
		fprintf(fp, "[INFO]         ");
		break;

	case LOG_WARN:
		fprintf(fp, "[WARN]         ");
		break;

	case LOG_ERROR:
		fprintf(fp, "[ERROR] [E%3d] ", err);
		break;

	case LOG_FATAL:
		fprintf(fp, "[FATAL] [E%3d] ", err);
		break;
	default:
		fprintf(fp, "               ");
		break;
	}

	fprintf(fp, "[%s] %s", argv0, message);

	if (name) fprintf(fp, " '%s'", name);

	fprintf(fp, "\n");

	fclose(fp);
	free(path);

	if (level == LOG_FATAL)
		exit(err);
}

void
notify(const char *summary, const char *body, const char *icon, NotifyUrgency urgency, const int formsum)
{
	char               *sum;
	NotifyNotification *notification;

	if (formsum) {
		sum = formatsummary(summary, body);
	} else {
		sum = malloc((strlen(summary) + 1) * sizeof(char));
		strcpy(sum, summary);
	}

	notify_init("dwmblocks");

	notification = notify_notification_new(sum, body, icon);
	notify_notification_set_urgency(notification, urgency);
	notify_notification_show(notification, NULL);

	g_object_unref(G_OBJECT(notification));
	notify_uninit();
	free(sum);
}

NotifyNotification*
newnotify(const char *summary, const char *body, const char *icon, NotifyUrgency urgency, const int formsum)
{
	char               *sum;
	NotifyNotification *notification;

	if (formsum) {
		sum = formatsummary(summary, body);
	} else {
		sum = malloc((strlen(summary) + 1) * sizeof(char));
		strcpy(sum, summary);
	}

	notify_init("dwmblocks");

	notification = notify_notification_new(sum, body, icon);
	notify_notification_set_urgency(notification, urgency);
	notify_notification_show(notification, NULL);

	free(sum);
	return notification;
}

void
updatenotify(NotifyNotification *notification, const char *summary, const char *body, const char *icon, NotifyUrgency urgency, const int timeout, const int formsum)
{
	char *sum;

	if (formsum) {
		sum = formatsummary(summary, body);
	} else {
		sum = malloc((strlen(summary) + 1) * sizeof(char));
		strcpy(sum, summary);
	}

	notify_notification_update(notification, sum, body, icon);
	notify_notification_set_urgency(notification, urgency);
	if (timeout)
		notify_notification_set_timeout(notification, timeout);

	notify_notification_show(notification, NULL);
	free(sum);
}

void
freenotify(NotifyNotification *notification)
{
	g_object_unref(G_OBJECT(notification));
	notify_uninit();
}

char*
strapp(char **dest, const char *src)
{
	char   *str;
	size_t len;

	if (!src)
		return NULL;

	if (!*dest) {
		*dest = strdup(src);
		return *dest;
	}

	len = strlen(*dest) + strlen(src) + 1;

	if (!(str = realloc(*dest, len * sizeof(char)))) {
		perror("realloc() returned NULL");
		exit(errno);
	}

	strncat(str, src, strlen(src));
	*dest = str;
	return *dest;
}

int
trimtonewl(const char *string)
{
	char *ptr;

	if ((ptr = strchr(string, '\n'))) {
		*ptr = '\0';
		return 1;
	}

	return 0;
}

char*
uitoa(const unsigned int num)
{
	char    *ret     = NULL;
	size_t  digits   = 0;
	int     snCheck = 0;

	for (unsigned int i = num; i > 0; i = i/10)
		digits++;
	if (!digits)
		digits++;

	if (!(ret = malloc((digits + 1) * sizeof(char)))) {
		logwrite("malloc() failed. Returned NULL pointer", NULL, LOG_ERROR, "dwmblocks-battery");
		return NULL;
	}

	snCheck = snprintf(ret, digits + 1, "%u", num);

	if (snCheck < 0 || snCheck > (int) digits) {
		logwrite("snprintf() failed. Buffer overflow", NULL, LOG_ERROR, "dwmblocks-battery");
		return NULL;
	}

	return ret;
}