/* See LICENSE file for copyright and license details. */

#ifndef UTILS_H
#define UTILS_H

#include <libnotify/notification.h>

typedef enum {
	LOG_SILLY,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
} log_level;

/*
 * Forks and executes given command.
 */
void forkexecv(const char *path, char **args, const char *argv0);

/*
 * Forks and executes given command from bin directory.
 */
 void forkexecvp(char **args, const char *argv0);

/*
 * Returns the absolute path of the concatenated path_array.
 * If is_file is true, then it doesn't add the last backslash.
 * Works with environment variables too, in the bas format.
 */
char* getpath(char **path_array);

/*
 * Returns the pid of the process that it's cmdline argument
 * matches the process string. Negative pids are the corresponding
 * error values. (If process exists more than one time, it fails)
 */
pid_t getpidof(const char *process, const char *argv0);

/*
 * Returns the output of xmenu, after `menu` string is passed as the
 * argument. Negative values are the corresponding error values.
 * Parses only single integer xmenu outputs.
 */
int getxmenuopt(const char *menu, const char *argv0);

/*
 * Writes a log to the file defined by log_path with a timestamp
 * and the name of the caller (argv0). If name str is NULL,
 * it doesn't add it. Has multiple log levels:
 * 	LOG_SILLY: does not write the strerror or log level.
 * 	LOG_INFO:  writes INFO log level but no strerror.
 * 	LOG_WARN:  writes WARN log level and strerror.
 * 	LOG_ERROR: writes ERROR log level, strerror and terminates with errno.
 */
void logwrite(const char *log, const char *name, const log_level level, const char *argv0);

/*
 * Sends a single desktop notification.
 */
void notify(const char *summary, const char *body, const char *icon, NotifyUrgency urgency, const int format_summary);

/*
 * Sends a notification and returns the pointer for manipulation of the notification.
 */
NotifyNotification* newnotify(const char *summary, const char *body, const char *icon, NotifyUrgency urgency, const int form_sum);

/*
 * Updates a notification.
 */
void updatenotify(NotifyNotification *notification, const char *summary, const char *body, const char *icon, NotifyUrgency urgency, const int timeout, const int form_sum);

/*
 * Frees the objects related to the notification.
 */
void freenotify(NotifyNotification *notification);

/*
 * Works exactly as strcat but with the destination allocated in the heap.
 */
char* strapp(char **dest, const char *src);

/*
 * Trims the string up to the first '\n' character. Returns 1 if it removes any.
 */
int trimtonewl(const char *string);

/*
 * Converts an unsigned int to a string. Allocates the memory.
 */
char* uitoa(const unsigned int num);

#endif /* UTILS_H */
