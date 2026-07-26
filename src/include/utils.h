/* See LICENSE file for copyright and license details. */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

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

void *ecalloc(const size_t nmemb, const size_t size);
void *emalloc(const size_t size);
void *erealloc(void *ptr, const size_t size);

/* Sends a single desktop notification with normal urgency. */
void notify(const char *sum, const char *body, const char *icon);

/*
 * Writes the decimal representation of 'in' into 'out'.
 * Dies if 'out' is too small to hold it.
 */
void uitoa(const unsigned int in, char *out, const size_t outsz);

/*
 * Forks and execs 'args' in a new session, searching PATH for args[0].
 * The child never returns.
 */
void execute(char **args);

/*
 * As execute(), but runs the exact binary at 'path'.
 */
void executepath(const char *path, char **args);

/*
 * Joins a NULL-terminated array of path components into 'out'. A component
 * beginning with '$' is replaced by the named environment variable; every
 * other component is prefixed with '/'.
 * Returns 0 on success, 1 if a variable was unset or 'out' was too small.
 */
int getpath(const char *const *parts, char *out, const size_t outsz);

/*
 * Presents 'menu' via xmenu(1) and returns the selected option's value,
 * or -1 if nothing was selected or xmenu could not be run.
 */
int getxmenuopt(const char *menu);

/*
 * Returns the pid of the process whose /proc/<pid>/cmdline starts with
 * 'process', or -1 if no such process exists.
 */
pid_t getpidof(const char *process);

#endif /* UTILS_H */
