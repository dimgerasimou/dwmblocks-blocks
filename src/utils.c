/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <fcntl.h>
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

#include "utils.h"

static const char *program_name;

void
set_name(const char *name)
{
	program_name = name;
}

const char *
get_name(void)
{
	return program_name;
}

void
die(const char *fmt, ...)
{
	va_list ap;
	int saved_errno = errno;

	fprintf(stderr, "%s: ", program_name);

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':')
		fprintf(stderr, " %s", strerror(saved_errno));
	fputc('\n', stderr);

	exit(1);
}

void
warn(const char *fmt, ...)
{
	va_list ap;
	int saved_errno = errno;

	fprintf(stderr, "%s: ", program_name);

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':')
		fprintf(stderr, " %s", strerror(saved_errno));
	fputc('\n', stderr);
}

void *
emalloc(const size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL)
		die("malloc:");
	return p;
}

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
forkexecv(const char *path, char **args)
{
	switch (fork()) {
	case -1:
		die("fork():");

	case 0:
		setsid();
		execv(path, args);
		die("execv() for: %s:", args[0]);

	default:
		break;
	}
}

void
forkexecvs(const char *path, char **args)
{
	switch (fork()) {
	case -1:
		die("fork:");

	case 0:
	{
		int fd;

		if (!(fd = open("/dev/null", O_WRONLY)))
			die("open() for \"/dev/null\":");

		if (dup2(fd, STDOUT_FILENO) == -1)
			die("dup2():");

		if (dup2(fd, STDERR_FILENO) == -1)
			die("dup2():");

		close(fd);

		setsid();
		execv(path, args);
		die("execv() for: %s:", args[0]);
	}

	default:
		break;
	}
}


void
forkexecvp(char **args)
{
	switch (fork()) {
	case -1:
		die("fork():");

	case 0:
		setsid();
		execvp(args[0], args);
		die("execvp() for: %s:", args[0]);

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

			die("getenv() for: %s:", path_array[i]);

			if (!env)
				die("getenv() for: %s:", path_array[i]);

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
getpidof(const char *process)
{
	char          buffer[PATH_MAX];
	struct dirent *ent;
	DIR           *dir;
	FILE          *fp;
	uint64_t      pid;
	pid_t         ret;

	dir = opendir("/proc");
	ret = 0;

	if (!dir)
		die("opendir() for \"/proc\":");

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
getxmenuopt(const char *menu)
{
	int  option;
	int  writepipe[2];
	int  readpipe[2];
	char buffer[16];

	option = -EREMOTEIO;
	buffer[0] = '\0';

	if (pipe(writepipe) < 0 || pipe(readpipe) < 0)
		die("pipe():");
	
	switch (fork()) {
		case -1:
			die("fork():");

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
killstr(const char *procname, const int signo)
{
	pid_t pID = getpidof(procname);

	if (pID > 0) {
		kill(pID, signo);
		return 0;
	}

	return pID;
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

	if (!(ret = malloc((digits + 1) * sizeof(char))))
		die("malloc():");

	snCheck = snprintf(ret, digits + 1, "%u", num);

	if (snCheck < 0 || snCheck > (int) digits)
		die("snprintf() buffer overflow");

	return ret;
}
