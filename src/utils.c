/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libnotify/notify.h>

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
ecalloc(const size_t nmemb, const size_t size)
{
	void *p;

	if ((p = calloc(nmemb, size)) == NULL)
		die("calloc:");
	return p;
}

void *
emalloc(const size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL)
		die("malloc:");
	return p;
}

void *
erealloc(void *ptr, const size_t size)
{
	void *p;

	if ((p = realloc(ptr, size)) == NULL)
		die("realloc:");
	return p;
}

void
notify(const char *sum, const char *body, const char *icon)
{
	NotifyNotification *n;

	notify_init(get_name());

	n = notify_notification_new(sum, body, icon);
	if (!n) {
		warn("notify_notification_new() failed");
		notify_uninit();
		return;
	}

	notify_notification_set_urgency(n, NOTIFY_URGENCY_NORMAL);
	notify_notification_show(n, NULL);

	g_object_unref(G_OBJECT(n));
	notify_uninit();
}

void
uitoa(const unsigned int in, char *out, const size_t outsz)
{
	size_t digits = 0;
	int    n      = 0;

	for (unsigned int i = in; i > 0; i = i / 10)
		digits++;
	if (!digits)
		digits++;

	if (digits + 1 > outsz)
		die("uitoa: buffer too small for %u", in);

	n = snprintf(out, digits + 1, "%u", in);

	if (n < 0 || (size_t)n > digits)
		die("snprintf: buffer overflow");
}

void
execute(char **args)
{
	if (!args || !args[0])
		die("execute: empty argument vector");

	switch (fork()) {
	case -1:
		die("fork:");

	case 0:
		setsid();
		execvp(args[0], args);
		warn("execvp for: %s:", args[0]);
		_exit(127);

	default:
		break;
	}
}

void
executepath(const char *path, char **args)
{
	if (!path || !*path || !args || !args[0])
		die("executepath: empty path or argument vector");

	switch (fork()) {
	case -1:
		die("fork:");

	case 0:
		setsid();
		execv(path, args);
		warn("execv for: %s:", path);
		_exit(127);

	default:
		break;
	}
}

int
getpath(const char *const *parts, char *out, const size_t outsz)
{
	size_t off = 0;
	int    n   = 0;

	if (!parts || !out || outsz == 0)
		return 1;

	out[0] = '\0';

	for (size_t i = 0; parts[i]; i++) {
		if (parts[i][0] == '$') {
			const char *env = getenv(parts[i] + 1);

			if (!env) {
				warn("getenv() for: %s", parts[i]);
				return 1;
			}

			n = snprintf(out + off, outsz - off, "%s", env);
		} else {
			n = snprintf(out + off, outsz - off, "/%s", parts[i]);
		}

		if (n < 0 || (size_t)n >= outsz - off) {
			warn("getpath: path too long");
			return 1;
		}

		off += (size_t)n;
	}

	return 0;
}

int
getxmenuopt(const char *menu)
{
	char   buf[16];
	int    towrite[2], toread[2];
	int    option = -1;
	pid_t  pid;
	size_t len;

	if (!menu || !*menu)
		return -1;

	if (pipe(towrite) < 0)
		die("pipe:");

	if (pipe(toread) < 0)
		die("pipe:");

	switch ((pid = fork())) {
	case -1:
		die("fork:");

	case 0:
		close(towrite[1]);
		close(toread[0]);

		if (dup2(towrite[0], STDIN_FILENO) < 0 ||
		    dup2(toread[1], STDOUT_FILENO) < 0)
			_exit(127);

		close(towrite[0]);
		close(toread[1]);

		execlp("xmenu", "xmenu", (char *)NULL);
		_exit(127);

	default:
		break;
	}

	close(towrite[0]);
	close(toread[1]);

	len = strlen(menu);
	if (write(towrite[1], menu, len) != (ssize_t)len)
		warn("write() to xmenu:");
	close(towrite[1]);

	{
		ssize_t got = read(toread[0], buf, sizeof(buf) - 1);

		if (got > 0) {
			buf[got] = '\0';
			if (sscanf(buf, "%d", &option) != 1)
				option = -1;
		}
	}

	close(toread[0]);

	while (waitpid(pid, NULL, 0) < 0 && errno == EINTR)
		;

	return option;
}

pid_t
getpidof(const char *process)
{
	char           buf[PATH_MAX];
	struct dirent *ent;
	DIR           *dir;
	pid_t          ret = -1;

	if (!process || !*process)
		return -1;

	dir = opendir("/proc");
	if (!dir) {
		warn("opendir() for \"/proc\":");
		return -1;
	}

	while ((ent = readdir(dir))) {
		FILE *fp;
		char *end;
		long  pid;

		pid = strtol(ent->d_name, &end, 10);
		if (end == ent->d_name || *end != '\0' || pid <= 0)
			continue;

		snprintf(buf, sizeof(buf), "/proc/%s/cmdline", ent->d_name);

		fp = fopen(buf, "r");
		if (!fp)
			continue;

		buf[0] = '\0';
		if (fgets(buf, sizeof(buf), fp) && strcmp(buf, process) == 0) {
			fclose(fp);
			ret = (pid_t)pid;
			break;
		}

		fclose(fp);
	}

	closedir(dir);

	if (ret < 0)
		errno = ESRCH;

	return ret;
}

void
dispatch(const struct Button *buttons, const size_t n, void *ctx)
{
	const char *env;
	int         want;

	if (!buttons || n == 0)
		return;

	env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	want = atoi(env);

	for (size_t i = 0; i < n; i++) {
		if (buttons[i].button == want) {
			if (buttons[i].handler)
				buttons[i].handler(ctx);
			return;
		}
	}
}

static int
ishexdigit(char c)
{
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

int
ishexcolor(const char *s)
{
	if (!s || strlen(s) != 7 || s[0] != '#')
		return 0;

	for (size_t i = 1; i < 7; i++) {
		if (!ishexdigit(s[i]))
			return 0;
	}

	return 1;
}
