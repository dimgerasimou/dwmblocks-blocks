/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>

#define BLUETOOTH_C

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

/* two states: off, on */
static const char *icons_bt[] = { "󰂲", "󰂯" };

/* minimal robustness: try hci0 then hci1 */
static const char *adapter_paths[] = { "/org/bluez/hci0", "/org/bluez/hci1", NULL };

static int
getbtadapterstate(DBusConnection *conn, DBusError *err, const char *objpath)
{
	DBusMessage     *msg, *reply;
	DBusMessageIter  args, replyArgs;

	const char  *iface   = "org.bluez.Adapter1";
	const char  *prop    = "Powered";
	dbus_bool_t  powered = FALSE;

	msg = dbus_message_new_method_call("org.bluez", objpath,
	                                   "org.freedesktop.DBus.Properties", "Get");
	if (!msg) {
		logwrite("Failed to create DBus message.", NULL, LOG_WARN, "dwmblocks-bluetooth");
		return -1;
	}

	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, err);
	dbus_message_unref(msg);

	if (dbus_error_is_set(err)) {
		logwrite("D-Bus error", err->message, LOG_WARN, "dwmblocks-bluetooth");
		dbus_error_free(err);
		return -1;
	}

	if (!reply)
		return -1;

	if (dbus_message_iter_init(reply, &replyArgs)) {
		DBusMessageIter variant;
		dbus_message_iter_recurse(&replyArgs, &variant);
		dbus_message_iter_get_basic(&variant, &powered);
	}

	dbus_message_unref(reply);
	return powered ? 1 : 0;
}

static int
getbtstate(void)
{
	DBusConnection *conn = NULL;
	DBusError err;
	int state = -1;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn || dbus_error_is_set(&err)) {
		logwrite("Failed to connect to the DBus system bus.", err.message, LOG_WARN, "dwmblocks-bluetooth");
		dbus_error_free(&err);
		return -1;
	}

	for (int i = 0; adapter_paths[i]; i++) {
		dbus_error_init(&err);
		state = getbtadapterstate(conn, &err, adapter_paths[i]);
		if (state >= 0)
			break;
	}

	dbus_connection_unref(conn);
	return state;
}

static int
setbtpowered(DBusConnection *conn, DBusError *err, const char *objpath, dbus_bool_t powered)
{
	DBusMessage     *msg, *reply;
	DBusMessageIter  args, valueIter;

	const char *iface = "org.bluez.Adapter1";
	const char *prop  = "Powered";

	msg = dbus_message_new_method_call("org.bluez", objpath,
	                                   "org.freedesktop.DBus.Properties", "Set");
	if (!msg) {
		logwrite("Failed to create DBus message.", NULL, LOG_WARN, "dwmblocks-bluetooth");
		return -1;
	}

	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

	dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "b", &valueIter);
	dbus_message_iter_append_basic(&valueIter, DBUS_TYPE_BOOLEAN, &powered);
	dbus_message_iter_close_container(&args, &valueIter);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, err);
	dbus_message_unref(msg);

	if (dbus_error_is_set(err)) {
		logwrite("D-Bus error", err->message, LOG_WARN, "dwmblocks-bluetooth");
		dbus_error_free(err);
		return -1;
	}

	if (!reply)
		return -1;

	dbus_message_unref(reply);
	return 0;
}

static void
togglebt(void)
{
	DBusConnection  *conn = NULL;
	DBusError        err;
	int              state = -1;
	const char      *objpath = NULL;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn || dbus_error_is_set(&err)) {
		logwrite("Failed to connect to the DBus system bus.", err.message, LOG_WARN, "dwmblocks-bluetooth");
		dbus_error_free(&err);
		return;
	}

	/* find an adapter we can talk to */
	for (int i = 0; adapter_paths[i]; i++) {
		dbus_error_init(&err);
		state = getbtadapterstate(conn, &err, adapter_paths[i]);
		if (state >= 0) {
			objpath = adapter_paths[i];
			break;
		}
	}

	if (!objpath) {
		logwrite("No Bluetooth adapter found (hci0/hci1).", NULL, LOG_WARN, "dwmblocks-bluetooth");
		dbus_connection_unref(conn);
		return;
	}

	/* toggle */
	dbus_error_init(&err);
	if (setbtpowered(conn, &err, objpath, state ? FALSE : TRUE) < 0) {
		/* error already logged */
		dbus_connection_unref(conn);
		return;
	}

	dbus_connection_unref(conn);
}

static void
execbutton(void)
{
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 2:
		togglebt();
		break;
	case 1:
		forkexecvp((char**)bt_tui_cmd, "dwmblocks-bluetooth");
		break;
	default:
		break;
	}
}

int
main(void)
{
	int state;

	execbutton();

	state = getbtstate();
	if (state < 0) {
		/* don’t make the whole block fail; just show “off” */
		state = 0;
	}

	printf(CLR_BT "%s\n" CLR_NRM, icons_bt[state ? 1 : 0]);
	return EXIT_SUCCESS;
}
