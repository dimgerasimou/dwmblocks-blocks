#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>

#define BLUETOOTH_C

#include "../include/colorscheme.h"
#include "../include/common.h"
#include "../include/config.h"

const char *bt_icons[] = {"󰂲", "󰂯", "󰥰"};

static int
getbtadapterstate(DBusConnection  *conn, DBusError *err)
{
	DBusMessage     *msg, *reply;
	DBusMessageIter  args, replyArgs;

	const char  *iface   = "org.bluez.Adapter1";
	const char  *prop    = "Powered";
	dbus_bool_t  powered = FALSE;

	msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0",
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
		logwrite("D-Bus Error:", err->message, LOG_WARN, "dwmblocks-bluetooth");
		dbus_error_free(err);
		return -1;
	}

	if (!reply) {
		logwrite("D-Bus: No reply received.", NULL, LOG_WARN, "dwmblocks-bluetooth");
		return -1;
	}

	if (dbus_message_iter_init(reply, &replyArgs)) {
		DBusMessageIter variant;
		dbus_message_iter_recurse(&replyArgs, &variant);
		dbus_message_iter_get_basic(&variant, &powered);
	}
	dbus_message_unref(reply);

	return powered;
}

static int
getbtstate(void)
{
	DBusConnection *conn  = NULL;
	DBusError       err;
	int             state = 0;

	/* initializing DBus connection */
	dbus_error_init(&err);
	
	if (!(conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err)) || dbus_error_is_set(&err)) {
		logwrite("Failed to connect to the DBus system bus.", err.message, LOG_WARN, "dwmblocks-bluetooth-menu");
		dbus_error_free(&err);
		return -1;
	}

	state = getbtadapterstate(conn, &err);

	if (state != 1) {
		dbus_connection_unref(conn);
		return state;
	}

	dbus_connection_unref(conn);
	return state;
}

static void
togglebt(void)
{
	DBusConnection  *conn;
	DBusMessage     *msg;
	DBusMessage     *reply;
	DBusMessageIter  args;
	DBusMessageIter  replyArgs;
	DBusMessageIter  valueIter;
	DBusError        err;

	const char  *iface   = "org.bluez.Adapter1";
	const char  *prop    = "Powered";
	dbus_bool_t  powered = FALSE;

	dbus_error_init(&err);

	if (!(conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err))) {
		logwrite("Failed to connect to the DBus system bus.", err.message, LOG_WARN, "dwmblocks-bluetooth-menu");
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}

	msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0",
	                                   "org.freedesktop.DBus.Properties", "Get");

	if (!msg) {
		logwrite("Failed to create DBus message.", NULL, LOG_WARN, "dwmblocks-bluetooth-menu");
		exit(EXIT_FAILURE);
	}

	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
	dbus_message_unref(msg);

	if (!reply) {
		logwrite("Failed to get Bluetooth Powered State.", err.message, LOG_WARN, "dwmblocks-bluetooth-menu");
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}

	if (dbus_message_iter_init(reply, &replyArgs)) {
		DBusMessageIter variant;
		dbus_message_iter_recurse(&replyArgs, &variant);
		dbus_message_iter_get_basic(&variant, &powered);
	}
	dbus_message_unref(reply);

	powered = !powered;

	msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0",
	                                   "org.freedesktop.DBus.Properties", "Set");

	if (!msg) {
		logwrite("Failed to create DBus message.", NULL, LOG_WARN, "dwmblocks-bluetooth-menu");
		exit(EXIT_FAILURE);
	}

	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &iface);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &prop);

	dbus_message_iter_open_container(&args, DBUS_TYPE_VARIANT, "b", &valueIter);
	dbus_message_iter_append_basic(&valueIter, DBUS_TYPE_BOOLEAN, &powered);
	dbus_message_iter_close_container(&args, &valueIter);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
	dbus_message_unref(msg);

	if (!reply) {
		logwrite("Failed to set Bluetooth Powered state.", err.message, LOG_WARN, "dwmblocks-bluetooth-menu");
		dbus_error_free(&err);
		exit(EXIT_FAILURE);
	}

	dbus_message_unref(reply);
}

static void
execbutton(void)
{
	char *env = NULL;

	if (!(env = getenv("BLOCK_BUTTON")))
		return;

	switch (atoi(env)) {
	case 2:
		togglebt();
		break;

	case 1:
		forkexecvp((char**) args_tui_settings, "dwmblocks-bluetooth");
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

	if (state < 0)
		return -state;

	printf(CLR_4 BG_1" %s " NRM "\n", bt_icons[state]);

	return EXIT_SUCCESS;
}
