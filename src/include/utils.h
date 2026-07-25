/* See LICENSE file for copyright and license details. */

#ifndef UTILS_H
#define UTILS_H

#include <libnotify/notification.h>

void set_name(const char *name);
const char *get_name(void);

/*
 * Prints formated message to stderr and exits.
 * If last char is ':', prints strerror with set errno.
 */
_Noreturn void die(const char *fmt, ...);

/*
 * Prints formated message to stderr and returns.
 * If last char is ':', prints strerror with set errno.
 */
void warn(const char *fmt, ...);

void *emalloc(const size_t size);

/*
 * Forks and executes given command.
 */
void forkexecv(const char *path, char **args);

/*
 * Forks and executes given command silently, redirecting
 * stdout and stderr to '/dev/null'.
 */
void forkexecvs(const char *path, char **args);

/*
 * Forks and executes given command from bin directory.
 */
 void forkexecvp(char **args);

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
pid_t getpidof(const char *process);

/*
 * Returns the output of xmenu, after `menu` string is passed as the
 * argument. Negative values are the corresponding error values.
 * Parses only single integer xmenu outputs.
 */
int getxmenuopt(const char *menu);

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
