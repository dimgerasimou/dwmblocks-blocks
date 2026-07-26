/* See LICENSE file for copyright and license details. */

#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static unsigned int checks;
static unsigned int failures;

#define CHECK(cond)                                                    \
	do {                                                           \
		checks++;                                              \
		if (!(cond)) {                                         \
			failures++;                                    \
			fprintf(stderr, "  FAIL %s:%d: %s\n",          \
			        __FILE__, __LINE__, #cond);            \
		}                                                      \
	} while (0)

static void
test_getpath_absolute(void)
{
	static const char *const parts[] = { "usr", "local", "bin", "dwmblocks", NULL };
	char out[PATH_MAX];

	CHECK(getpath(parts, out, sizeof(out)) == 0);
	CHECK(strcmp(out, "/usr/local/bin/dwmblocks") == 0);
}

static void
test_getpath_env(void)
{
	static const char *const parts[] = { "$DWMB_TEST_HOME", ".local", "bin", "x", NULL };
	char out[PATH_MAX];

	setenv("DWMB_TEST_HOME", "/home/tester", 1);

	CHECK(getpath(parts, out, sizeof(out)) == 0);
	CHECK(strcmp(out, "/home/tester/.local/bin/x") == 0);
}

static void
test_getpath_unset_env(void)
{
	static const char *const parts[] = { "$DWMB_NO_SUCH_VAR", "bin", NULL };
	char out[PATH_MAX];

	unsetenv("DWMB_NO_SUCH_VAR");

	/* An unset variable must fail rather than silently produce a bad path. */
	CHECK(getpath(parts, out, sizeof(out)) == 1);
}

static void
test_getpath_overflow(void)
{
	static const char *const parts[] = { "aaaaaaaa", "bbbbbbbb", "cccccccc", NULL };
	char out[8];

	/* Must report truncation rather than overrun the buffer. */
	CHECK(getpath(parts, out, sizeof(out)) == 1);
}

static void
test_getpath_empty(void)
{
	static const char *const parts[] = { NULL };
	char out[PATH_MAX];

	CHECK(getpath(parts, out, sizeof(out)) == 0);
	CHECK(out[0] == '\0');
}

static void
test_uitoa(void)
{
	char buf[16];

	uitoa(0, buf, sizeof(buf));
	CHECK(strcmp(buf, "0") == 0);

	uitoa(7, buf, sizeof(buf));
	CHECK(strcmp(buf, "7") == 0);

	uitoa(100, buf, sizeof(buf));
	CHECK(strcmp(buf, "100") == 0);

	uitoa(4294967295u, buf, sizeof(buf));
	CHECK(strcmp(buf, "4294967295") == 0);
}

static void
test_ishexcolor(void)
{
	CHECK(ishexcolor("#F38BA8") == 1);
	CHECK(ishexcolor("#000000") == 1);
	CHECK(ishexcolor("#abcdef") == 1);
	CHECK(ishexcolor("#ABCDEF") == 1);

	CHECK(ishexcolor("#GGGGGG") == 0);   /* not hex */
	CHECK(ishexcolor("F38BA8") == 0);    /* no leading # */
	CHECK(ishexcolor("#F38BA") == 0);    /* too short */
	CHECK(ishexcolor("#F38BA88") == 0);  /* too long */
	CHECK(ishexcolor("") == 0);
	CHECK(ishexcolor(NULL) == 0);
}

static void
test_getpidof_missing(void)
{
	/* No process should ever have this cmdline. */
	CHECK(getpidof("dwmblocks-no-such-process-xyzzy") < 0);
	CHECK(getpidof("") < 0);
	CHECK(getpidof(NULL) < 0);
}

int
main(void)
{
	set_name("test-utils");

	test_getpath_absolute();
	test_getpath_env();
	test_getpath_unset_env();
	test_getpath_overflow();
	test_getpath_empty();
	test_uitoa();
	test_ishexcolor();
	test_getpidof_missing();

	printf("test-utils: %u checks, %u failures\n", checks, failures);

	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
