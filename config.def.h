/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H

static const char term_cmd[] = "st";
static const char term_title_opt[] = "-t";

/* ============================================================
 * COLOURS
 * ============================================================ */

/*
 * How a colour is written into the status line. CLR_FMT must contain
 * exactly one %s, which receives a "#RRGGBB" string; CLR_NRM ends the
 * coloured run. The defaults target dwm's status2d patch.
 *
 *   NAME                            CLR_FMT                  CLR_NRM
 *   status2d (dwm)                  "^c%s^"                  "^d^"
 *   Pango (waybar, polybar, i3bar)  "<span color='%s'>"      "</span>"
 *   polybar native                  "%%{F%s}"                "%%{F-}"
 *   no colour                       "%.0s"                   ""
 */
#define CLR_FMT "^c%s^"
#define CLR_NRM "^d^"

/*
 * Default colours, listed in enum Color order (see src/include/colors.h).
 * An empty string leaves that colour unset, so the block renders in the
 * status bar's own colour. Matching entries in the X resource database
 * override these at runtime, e.g.
 *
 *   dwmblocks.clr_bat_crt: #F38BA8
 *
 * A compile-time check in colors.c fails the build if this list and the
 * enum ever fall out of step.
 */
static const char *const clr_defaults[] = {
	"#F38BA8",  /* clr_bat_crt  battery critical */
	"#F9E2AF",  /* clr_bat_low  battery low      */
	"#CDD6F4",  /* clr_bat_nrm  battery normal   */
	"#CDD6F4",  /* clr_bat_chg  battery charging */
	"#CDD6F4",  /* clr_bt       bluetooth        */
	"#CDD6F4",  /* clr_date     date             */
	"#CDD6F4",  /* clr_net_nrm  network normal   */
	"#F38BA8",  /* clr_net_err  network error    */
	"#89B4FA",  /* clr_krn_pkg  pending updates  */
	"#CDD6F4",  /* clr_krn_nrm  kernel release   */
	"#CDD6F4",  /* clr_kbd      keyboard layout  */
	"#CDD6F4",  /* clr_mem      memory           */
	"#F38BA8",  /* clr_pwr      power menu       */
	"#CDD6F4",  /* clr_tim      clock            */
	"#CDD6F4",  /* clr_vol_nrm  volume normal    */
	"#F38BA8",  /* clr_vol_mut  volume muted     */
	"#F38BA8"   /* clr_cal      calendar accent   */
};

/* ============================================================
 * BATTERY BLOCK
 * ============================================================ */
#ifdef BATTERY_C

/* Enable power management features (optimus-manager support) */
#define POWER_MANAGEMENT


/* Status icons, ordered from empty to full; the last is "charging". */
static const char *const icons_battery[] = {
	" ",
	" ",
	" ",
	" ",
	" ",
	" ",
};
#endif

/* ============================================================
 * BLUETOOTH BLOCK
 * ============================================================ */
#ifdef BLUETOOTH_C

/* TUI application for bluetooth settings */
const char *bt_tui_cmd[] = { term_cmd, "bluetuith", NULL };


/* Status icons: [0] disabled, [1] enabled. */
static const char *const icons_bluetooth[] = {
	"󰂲",
	"󰂯",
};
#endif

/* ============================================================
 * DATE BLOCK
 * ============================================================ */
#ifdef DATE_C

/* Show calendar icon in bar */
const unsigned int show_icon = 1;

/* GUI calendar application */
const char *args_gui_calendar[] = {
	"zen-browser",
	"--new-window",
	"https://calendar.google.com",
	NULL
};


/* Icon shown in the bar when show_icon is set. */
static const char icon_date[] = " ";
#endif

/* ============================================================
 * INTERNET BLOCK
 * ============================================================ */
#ifdef INTERNET_C

/* Network management TUI */
const char *args_tui_internet[] = {
	term_cmd,
	term_title_opt, "Network Configuration",
	"nmtui",
	NULL
};

/* WiFi connection script */
const char *path_wifi_connect = "~/.local/bin/dmenu-wifi-prompt";
const char *args_wifi_connect[] = {"dmenu-wifi-prompt", NULL};


/* Bar icons, indexed by connection state. */
static const char *const icons_internet[] = {
	"󰤮 ",  /* 0: no primary connection / unknown */
	" ",  /* 1: ethernet */
	"󰤯 ",  /* 2: wifi 0 */
	"󰤟 ",  /* 3: wifi 1 */
	"󰤢 ",  /* 4: wifi 2 */
	"󰤥 ",  /* 5: wifi 3 */
	"󰤨 ",  /* 6: wifi 4 */
	"󰤫 ",  /* 7: error */
};

/* Notification icons: [0] error, [1] wired, [2] wireless. */
static const char *const icons_internet_notif[] = {
	"x",
	"tdenetworkmanager",
	"wifi-radar",
};

/* xmenu prompt. Each line is "<label>\t<value>". */
static const char menu_internet[] = "󱛄 Toggle Wifi\t0\n󱛃 Connect to wifi\t1\n󱚾 TUI options\t2";
#endif

/* ============================================================
 * KERNEL BLOCK
 * ============================================================ */
#ifdef KERNEL_C

/* System update command */
const char *args_update_cmd[] = {
	term_cmd,
	term_title_opt, "System Upgrade",
	"sh", "-c",
	"echo \"Upgrading system\" && paru",
	NULL
};

/* Package update check commands */
const char *cmd_aur_updates = "/bin/paru -Qua";
const char *cmd_pm_updates  = "/bin/checkupdates";

/* Show release info in bar */
const unsigned int show_release = 1;

/* Show update count in bar*/
const unsigned int show_update_count = 1;


/* Bar icons: the kernel (tux) glyph and the pending-updates glyph. */
static const char icon_kernel_tux[] = "";
static const char icon_kernel_pkg[] = "󰏖";
static const char icon_kernel_pacman[] = "󰏖";
static const char icon_kernel_aur[] = "";

/*
 * Seconds before cached update counts are refreshed. Set to 0 to disable
 * caching entirely and query on every run.
 */
static const long update_cache_ttl = 3600;

/*
 * The cache is also invalidated whenever this path's mtime is newer than
 * it, which is how a pacman post-transaction hook makes the count drop to
 * zero immediately instead of waiting for the TTL.
 */
static const char pacman_local_db[] = "/var/lib/pacman/local";
#endif

/* ============================================================
 * KEYBOARD BLOCK
 * ============================================================ */
#ifdef KEYBOARD_C

const unsigned int show_icon = 1;

/* Keyboard layout switching script */
const char *path_language_switch = "~/.local/bin/dwm-xkbnext";
const char *args_language_switch[] = { "dwm-xkbnext", NULL };


/* Icon shown in the bar when show_icon is set. */
static const char icon_keyboard[] = " ";
#endif

/* ============================================================
 * MEMORY BLOCK
 * ============================================================ */
#ifdef MEMORY_C

const unsigned int show_icon = 1;

/* Task manager application */
const char *args_task_manager[] = { term_cmd, "sh", "-c", "htop", NULL };


/* Icon shown in the bar when show_icon is set. */
static const char icon_memory[] = " ";
#endif

/* ============================================================
 * POWER BLOCK
 * ============================================================ */
#ifdef POWER_C

/* Enable clipboard integration */
#define CLIPBOARD

/* Enable power management features (optimus-manager support) */
#define POWER_MANAGEMENT

/* dwmblocks executable path and arguments */
const char *path_dwmblocks = "/usr/local/bin/dwmblocks";
const char *args_dwmblocks[] = {"dwmblocks", NULL};

/* Lock screen command */
const char *args_lockscreen[]       = {"slock", NULL};

/* Clipboard management */
const char *args_clipboard_delete[] = {"sh", "-c", "clipdel -d \".*\"", NULL};


/* Bar icon. */
static const char icon_power[] = "";

/* xmenu prompts. Each line is "<label>\t<value>". */
static const char menu_power[] = " Shutdown\t0\n Reboot\t1\n\n󰗽 Logout\t2\n Lock\t3\n\n Restart DwmBlocks\t4";
static const char menu_power_optimus[] = "\n󰘚 Optimus Manager\t5";
static const char menu_power_clipboard[] = "\n󰅌 Clipmenu\t6";
static const char menu_optimus[] = "Integrated\t0\nHybrid\t1\nNvidia\t2";
static const char menu_clipboard[] = "Pause clipmenu for 1 minute\t0\nClear clipboard\t1";
static const char menu_yes_no[] = "Are you sure?\t-1\nYes\t1\nNo\t0";
#endif

/* ============================================================
 * TIME BLOCK
 * ============================================================ */
#ifdef TIME_C

const unsigned int show_icon = 1;


/* Icon shown in the bar when show_icon is set. */
static const char icon_time[] = " ";
#endif


/* ============================================================
 * VOLUME BLOCK
 * ============================================================ */
#ifdef VOLUME_C

/* What the block displays:
 *   0 - icon and volume
 *   1 - icon only
 *   2 - volume only
 */
const unsigned int display_type = 0;

/* Padding for volume string; boolean */
const unsigned int volume_padding = 1;

/* Audio equalizer application */
const char *args_eqalizer[]        = {"easyeffects", NULL};

/* Volume control script and arguments */
const char *args_volume_increase[] = {"dwm-audio", "up", NULL};
const char *args_volume_decrase[]  = {"dwm-audio", "down", NULL};
const char *args_volume_mute[]     = {"dwm-audio", "mute", NULL};
static const char path_volume_control[] = "~/.local/bin/dwm-audio";


/* Bar icons: [0] muted, [1] low, [2] medium, [3] high, [4] no sink. */
static const char *const icons_volume[] = {
	" ",
	" ",
	" ",
	" ",
	"",
};

/* Notification icons for the default sink and source. */
static const char icon_vol_sink[] = "";
static const char icon_vol_source[] = "";
#endif

#endif /* CONFIG_H */
