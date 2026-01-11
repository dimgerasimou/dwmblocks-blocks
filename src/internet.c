/* See LICENSE file for copyright and license details. */

#include <NetworkManager.h>
#include <nm-dbus-interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERNET_C

#include "colorscheme.h"
#include "utils.h"
#include "config.h"

const char *notif_icons[]  = {"x", "tdenetworkmanager", "wifi-radar"};
const char *status_icons[] = {
	CLR_NET_ERR "󰤮 ", /* 0: no primary connection / unknown */
	CLR_NET_NRM " ", /* 1: ethernet */
	CLR_NET_NRM "󰤯 ", /* 2: wifi 0 */
	CLR_NET_NRM "󰤟 ", /* 3: wifi 1 */
	CLR_NET_NRM "󰤢 ", /* 4: wifi 2 */
	CLR_NET_NRM "󰤥 ", /* 5: wifi 3 */
	CLR_NET_NRM "󰤨 ", /* 6: wifi 4 */
	CLR_NET_ERR "󰤫 "  /* 7: error */
};
const char *menu_string    = "󱛄 Toggle Wifi\t0\n󱛃 Connect to wifi\t1\n󱚾 TUI options\t2";

/* function definitions */
static void comapp(NMDevice *dev, GString *str);
static void ethapp(NMDevice *dev, GString *str);
static void execbutton(NMClient *c, int icind);
static unsigned int getactive(NMClient *c);
static void nminit(NMClient **c);
static void printinfo(NMClient *c, const int ind);
static void togglewifi(NMClient *client);
static void wifiapp(NMDevice *dev, GString *str);

static inline unsigned int
clampstate(unsigned int s)
{
	const unsigned int n = (unsigned int)(sizeof(status_icons) / sizeof(status_icons[0]));
	if (s >= n)
		return 0;
	return s;
}

static void
comapp(NMDevice *dev, GString *str)
{
	NMActiveConnection *ac;
	const char *mac;

	ac = nm_device_get_active_connection(dev);
	if (!ac)
		return;

	mac = nm_device_get_hw_address(dev);
	if (!mac || !*mac)
		mac = "*unknown*";

	/* IPv4 */
	const char *ip4_addr = NULL;
	const char *ip4_gw   = NULL;
	NMIPConfig *ip4_cfg  = nm_active_connection_get_ip4_config(ac);
	if (ip4_cfg) {
		GPtrArray *a = nm_ip_config_get_addresses(ip4_cfg);
		if (a && a->len >= 1) {
			NMIPAddress *ip = g_ptr_array_index(a, 0);
			if (ip)
				ip4_addr = nm_ip_address_get_address(ip);
		}
		ip4_gw = nm_ip_config_get_gateway(ip4_cfg);
	}

	/* IPv6 */
	const char *ip6_addr = NULL;
	NMIPConfig *ip6_cfg  = nm_active_connection_get_ip6_config(ac);
	if (ip6_cfg) {
		GPtrArray *a = nm_ip_config_get_addresses(ip6_cfg);
		if (a && a->len >= 1) {
			NMIPAddress *ip = g_ptr_array_index(a, 0);
			if (ip)
				ip6_addr = nm_ip_address_get_address(ip);
		}
	}

	if (!ip4_addr || !*ip4_addr) ip4_addr = "*none*";
	if (!ip4_gw   || !*ip4_gw)   ip4_gw   = "*none*";
	if (!ip6_addr || !*ip6_addr) ip6_addr = "*none*";

	g_string_append_printf(str,
		"MAC:  %s\n"
		"IPv4: %s\n"
		"Gate: %s\n"
		"IPv6: %s\n\n",
		mac, ip4_addr, ip4_gw, ip6_addr);
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
	const char *env = getenv("BLOCK_BUTTON");
	if (!env || !*env)
		return;

	switch (atoi(env)) {
	case 1:
		printinfo(c, icind);
		break;

	case 3:
		switch (getxmenuopt(menu_string, "dwmblocks-internet")) {
		case 0:
			togglewifi(c);
			break;

		case 1: {
			char *path = getpath((char **)path_wifi_connect);
			if (path) {
				forkexecv(path, (char **)args_wifi_connect, "dwmblocks-internet");
				free(path);
			}
			break;
		}

		case 2:
			forkexecvp((char **)args_tui_internet, "dwmblocks-internet");
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

	ac = nm_client_get_primary_connection(c);
	if (!ac)
		return 0;

	type = nm_active_connection_get_connection_type(ac);
	if (!type)
		return 0;

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

		/* map 0..100 -> 0..4 */
		state = (nm_access_point_get_strength(ap) / 20);
		state = state > 4 ? 4 : state;
		return (2 + state); /* 2..6 */
	}

	return 0;
}

static void
nminit(NMClient **c)
{
	GError *error = NULL;

	if (!c)
		return;

	*c = nm_client_new(NULL, &error);

	if (error) {
		logwrite("Error initializing NetworkManager client", error->message, LOG_WARN, "dwmblocks-internet");
		g_error_free(error);
	}

	/* If nm_client_new failed, *c will be NULL. Caller already checks. */
}

static void
printinfo(NMClient *c, const int ind)
{
	const GPtrArray *devarr;
	unsigned int icind;
	NMDevice *dev;
	GString *str;

	devarr = nm_client_get_devices(c);
	icind = (unsigned int)(ind > 2 ? 2 : ind);

	if (!devarr) {
		notify("Network Devices Info", "No network devices detected", (char *)notif_icons[0],
		       NOTIFY_URGENCY_NORMAL, 1);
		return;
	}

	str = g_string_new("");
	if (!str) {
		notify("Network Devices Info", "Out of memory", (char *)notif_icons[0],
		       NOTIFY_URGENCY_NORMAL, 1);
		return;
	}

	for (int i = 0; i < (int)devarr->len; i++) {
		dev = g_ptr_array_index(devarr, i);
		if (!dev) {
			logwrite("Failed to fetch a device", NULL, LOG_INFO, "dwmblocks-internet");
			continue;
		}

		int type = nm_device_get_device_type(dev);
		switch (type) {
		case NM_DEVICE_TYPE_ETHERNET:
			ethapp(dev, str);
			comapp(dev, str);
			break;

		case NM_DEVICE_TYPE_WIFI:
			wifiapp(dev, str);
			comapp(dev, str);
			break;

		default:
			break;
		}
	}

	if (str->len <= 1)
		notify("Network Devices Info", "No network devices detected", (char *)notif_icons[icind],
		       NOTIFY_URGENCY_NORMAL, 1);
	else
		notify("Network Devices Info", str->str, (char *)notif_icons[icind],
		       NOTIFY_URGENCY_NORMAL, 1);

	g_string_free(str, TRUE);
}

static void
togglewifi(NMClient *client)
{
	/* May fail due to permissions/polkit; we at least report what we observe after the call. */
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
	GBytes *ssid;
	guint32 freq_mhz;
	guint8 stren;

	ac = nm_device_get_active_connection(dev);
	if (!ac) {
		g_string_append(str, "Wifi: Disconnected\n\n");
		return;
	}

	ap = nm_device_wifi_get_active_access_point(NM_DEVICE_WIFI(dev));
	if (!ap) {
		g_string_append(str, "Wifi: Connected (AP unknown)\n\n");
		return;
	}

	ssid     = nm_access_point_get_ssid(ap);
	freq_mhz = nm_access_point_get_frequency(ap); /* MHz */
	stren    = nm_access_point_get_strength(ap);

	const char *ssid_disp = "*hidden_network*";
	char *ssid_utf8 = NULL;

	if (ssid) {
		ssid_utf8 = nm_utils_ssid_to_utf8(g_bytes_get_data(ssid, NULL),
		                                 g_bytes_get_size(ssid));
		if (ssid_utf8 && *ssid_utf8)
			ssid_disp = ssid_utf8;
	}

	/* Convert MHz -> GHz for display */
	double freq_ghz = freq_mhz > 0 ? (freq_mhz / 1000.0) : 0.0;

	g_string_append_printf(str,
		"Wifi\n"
		"SSID: %s\n"
		"Strn: %d%% | Freq: %.1f GHz\n",
		ssid_disp, (int)stren, freq_ghz);

	if (ssid_utf8)
		g_free(ssid_utf8);
}

int
main(void)
{
	unsigned int state = 0;
	NMClient *client = NULL;

	nminit(&client);

	if (client) {
		state = getactive(client);
		state = clampstate(state);
		execbutton(client, (int)state);
		g_object_unref(client);
	} else {
		state = 7;
	}

	state = clampstate(state);
	printf("%s\n" CLR_NRM, status_icons[state]);

	return 0;
}
