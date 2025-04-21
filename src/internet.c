#include <NetworkManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERNET_C

#include "nm-dbus-interface.h"
#include "../include/colorscheme.h"
#include "../include/common.h"
#include "../include/config.h"

const char *notif_icons[]  = {"x", "tdenetworkmanager", "wifi-radar"};
const char *status_icons[] = {CLR_9"󰤮  "NRM, CLR_6"  "NRM, CLR_6"󰤯  "NRM, CLR_6"󰤟  "NRM, CLR_6"󰤢  "NRM, CLR_6"󰤥  "NRM, CLR_6"󰤨  "NRM, CLR_9"󰤫  "NRM};
const char *menu_string    = "󱛄 Toggle Wifi\t0\n󱛃 Connect to wifi\t1\n󱚾 TUI options\t2";

/* structs */
struct Ip {
	NMIPConfig *cfg;
	GPtrArray  *arr;
	const char *add;
	const char *gat;
};

/* function definitions */
static void comapp(NMDevice *dev, GString *str);
static void ethapp(NMDevice *dev, GString *str);
static void execbutton(NMClient *c, int icind);
static unsigned int getactive(NMClient *c);
static void nminit(NMClient **c);
static void printinfo(NMClient *c, const int ind);
static void togglewifi(NMClient *client);
static void wifiapp(NMDevice *dev, GString *str);

static void
comapp(NMDevice *dev, GString *str)
{
	NMActiveConnection *ac;
	struct Ip ip4, ip6;
	const char *mac;

	ac = nm_device_get_active_connection(dev);

	if (!ac) return;

	mac = nm_device_get_hw_address(dev);

	ip4.cfg = nm_active_connection_get_ip4_config(ac);
	ip4.arr = nm_ip_config_get_addresses(ip4.cfg);
	ip4.add = nm_ip_address_get_address(g_ptr_array_index(ip4.arr, 0));
	ip4.gat = nm_ip_config_get_gateway(ip4.cfg);

	ip6.cfg = nm_active_connection_get_ip6_config(ac);
	ip6.arr = nm_ip_config_get_addresses(ip6.cfg);
	ip6.add = nm_ip_address_get_address(g_ptr_array_index(ip6.arr, 0));

	g_string_append_printf(str, "MAC:  %s\nIPv4: %s\nGate: %s\nIPv6: %s\n\n",
	                       mac, ip4.add, ip4.gat, ip6.add);
}

static void
ethapp(NMDevice *dev, GString *str)
{
	NMActiveConnection *ac;

	ac = nm_device_get_active_connection(dev);

	if (!ac)
		g_string_append(str, "Ethernet: Disconnected\n\n");
	else
		g_string_append(str, "Ethernet\n");
}

static void
execbutton(NMClient *c, int icind)
{
	char *env;

	if (!(env = getenv("BLOCK_BUTTON")))
		return;

	switch (atoi(env)) {
	case 1:
		printinfo(c, icind);
		break;

	case 3:
		switch(get_xmenu_option(menu_string, "dwmblocks-internet")) {
		case 0:
			togglewifi(c);
			break;
		
		case 1:
		{
			char *path;
			
			path = get_path((char**) path_wifi_connect, 1);
			forkexecv(path, (char**) args_wifi_connect, "dwmblocks-internet");

			free(path);
			break;
		}

		case 2:
			forkexecvp((char**) args_tui_internet, "dwmblocks-internet");
			break;
		
		default:
			break;
		}
		break;

	default:
		break;
	}
}

static unsigned int
getactive(NMClient *c)
{
	NMActiveConnection *ac;
	const GPtrArray *acdev;
	NMAccessPoint *ap;
	unsigned int state;
	NMDeviceWifi *dev;
	const char *type;

	if (!(ac = nm_client_get_primary_connection(c)))
		return 0;
	
	type = nm_active_connection_get_connection_type(ac);

	if (!strcmp(type, "802-3-ethernet"))
		return 1;

	if (!strcmp(type, "802-11-wireless")) {
		acdev = nm_active_connection_get_devices(ac);

		if (!acdev || acdev->len < 1) {
			logwrite("Wifi active devices less than 1", NULL, LOG_INFO, "dwmblocks-internet");
			return 7;
		}

		dev = NM_DEVICE_WIFI(g_ptr_array_index(acdev, 0));

		if (!dev) {
			logwrite("Could not fetch wifi device", NULL, LOG_INFO, "dwmblocks-internet");
			return 7;
		}

		ap = nm_device_wifi_get_active_access_point(dev);

		if (!ap) {
			logwrite("Could not fetch active access point", NULL, LOG_INFO, "dwmblocks-internet");
			return 7;
		}

		state = (nm_access_point_get_strength(ap) / 20);
		state = state > 4 ? 4 : state;
		return (2 + state);
	}
	return 0;
}

static void
nminit(NMClient **c)
{
	GError *error = NULL;

	*c = nm_client_new(NULL, &error);
	if (error)
		logwrite("Error initializing NetworkManager client", error->message, LOG_WARN, "dwmblocks-internet");
}

static void
printinfo(NMClient *c, const int ind)
{
	const GPtrArray *devarr;
	unsigned int icind;
	NMDevice *dev;
	GString *str;

	devarr = nm_client_get_devices(c);
	icind = ind > 2 ? 2 : ind;

	if (!devarr) {
		notify("Network Devices Info", "No network devices detected", (char*) notif_icons[0], NOTIFY_URGENCY_NORMAL, 1);
		return;
	}

	str = g_string_new("");

	for (int i = 0; i < (int) devarr->len; i++) {
		dev = g_ptr_array_index(devarr, i);

		if (!dev) {
			logwrite("Failed to fetch a device", NULL, LOG_INFO, "dwmblocks-internet");
			return;
		}

		int type = nm_device_get_device_type(dev);
		switch (type) {
			case NM_DEVICE_TYPE_ETHERNET:
				ethapp(dev, str);
				goto INFO;

			case NM_DEVICE_TYPE_WIFI:
				wifiapp(dev, str);
			INFO:
				comapp(dev, str);

			default:
				break;
		}
	}

	if (str->len <= 1)
		notify("Network Devices Info", "No network devices detected", (char*) notif_icons[icind], NOTIFY_URGENCY_NORMAL, 1);
	else
		notify("Network Devices Info", str->str, (char*) notif_icons[icind], NOTIFY_URGENCY_NORMAL, 1);

	g_string_free(str, TRUE);
}

static void
togglewifi(NMClient *client)
{
	gboolean current = nm_client_wireless_get_enabled(client);
	nm_client_wireless_set_enabled(client, !current);

	const char *msg = current ? "WiFi Disabled" : "WiFi Enabled";
	notify("WiFi Status", msg, notif_icons[2], NOTIFY_URGENCY_NORMAL, 1);
}

static void
wifiapp(NMDevice *dev, GString *str)
{
	NMActiveConnection *ac;
	NMAccessPoint *ap;
	GString *ssidstr;
	guint32 freq;
	GBytes *ssid;
	guint8 stren;

	if (!(ac = nm_device_get_active_connection(dev))) {
		g_string_append(str, "Wifi: Disconnected\n\n");
		return;
	}

	ap    = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(dev));
	ssid  = nm_access_point_get_ssid(ap);
	freq  = nm_access_point_get_frequency(ap);
	stren = nm_access_point_get_strength(ap);

	if (ssid)
		ssidstr = g_string_new(nm_utils_ssid_to_utf8(g_bytes_get_data(ssid, NULL),
		                       g_bytes_get_size(ssid)));
	else
		ssidstr = g_string_new("*hidden_network*");

	g_string_append_printf(str, "Wifi\nSSID: %s\nStrn: %d%% | Freq: %d GHz\n",
	                       ssidstr->str, stren, freq);

	g_string_free(ssidstr, TRUE);
}

int
main(void)
{
	unsigned int state;
	NMClient *client;

	state = 0;
	nminit(&client);

	if (client) {
		state = getactive(client);
		execbutton(client, state);
		g_object_unref(client);
	}

	printf(" "BG_1" %s\n", status_icons[state]);

	return 0;
}
