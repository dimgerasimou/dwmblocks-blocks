/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H

/*
 * HOW THIS FILE IS ORGANISED
 *
 * Everything before the first "#ifdef" is shared by every block. After
 * that, each block has its own section guarded by a macro that only that
 * block defines: battery.c does "#define BATTERY_C" before including this
 * file, so it sees the BATTERY BLOCK section and nothing else.
 *
 * That is why names such as show_icon appear several times below without
 * colliding: exactly one section is ever compiled into a given block. It
 * also means a setting placed in the wrong section is silently ignored
 * rather than reported, so keep each one under the block that reads it.
 *
 * Settings marked "Requires:" name an external program that must be on
 * $PATH, or a script you must supply yourself. Clicking a block whose
 * program is missing does nothing and writes a message to stderr, which
 * dwmblocks discards; run the block by hand in a terminal to see it.
 */

/* Terminal used by blocks that open a TUI. Requires: this terminal. */
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
 *   status2d (dwm)   "^c%s^"                  "^d^"
 *   Pango (waybar,
 *   polybar, i3bar)  "<span color='%s'>"      "</span>"
 *   polybar native   "%%{F%s}"                "%%{F-}"
 *   no colour        "%.0s"                   ""
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
	"#FAB387",  /* clr_bat_low  battery low      */
	"#A6E3A1",  /* clr_bat_nrm  battery normal   */
	"#A6E3A1",  /* clr_bat_chg  battery charging */
	"#89B4FA",  /* clr_bt       bluetooth        */
	"#CBA6F7",  /* clr_date     date             */
	"#94E2D5",  /* clr_net_nrm  network normal   */
	"#F38BA8",  /* clr_net_err  network error    */
	"#F9E2AF",  /* clr_sys_pkg  pending updates  */
	"#89DCEB",  /* clr_sys_nrm  kernel release   */
	"#B4BEFE",  /* clr_kbd      keyboard layout  */
	"#F5C2E7",  /* clr_mem      memory           */
	"#F38BA8",  /* clr_pwr      power menu       */
	"#FAB387",  /* clr_tim      clock            */
	"#A6E3A1",  /* clr_vol_nrm  volume normal    */
	"#6C7086",  /* clr_vol_mut  volume muted     */
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

/* Calendar opened on right click. Requires: the browser named below. */
const char *args_gui_calendar[] = {
	"zen-browser",
	"--new-window",
	"https://calendar.google.com",
	NULL
};


/*
 * First column of the calendar, as a tm_wday value: 1 = Monday (most of
 * Europe), 0 = Sunday (US), 6 = Saturday. Month and weekday names come
 * from the locale, so run the date block under the locale you want.
 */
static const int calendar_week_start = 1;

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
static const char path_wifi_connect[] = "~/.local/bin/dmenu-wifi-prompt";
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
 * SYSTEM BLOCK
 * ============================================================ */
#ifdef SYSTEM_C

/*
 * The block counts pending updates from two sources and shows the kernel
 * release. Nothing below is distribution-specific; the defaults are for
 * Arch, and the comments give equivalents for other package managers.
 * Set a command to "" to disable that source.
 *
 *   Debian/Ubuntu   primary:   "apt list --upgradable 2>/dev/null | tail -n +2"
 *                   watch:     "/var/lib/dpkg/status"
 *   Fedora          primary:   "dnf -q --refresh check-update"
 *                   watch:     "/var/lib/rpm"
 *   void            primary:   "xbps-install -Mun"
 *                   watch:     "/var/db/xbps"
 *
 * Each command should print one line per pending update; only the line
 * count is used. A non-zero exit status is ignored, because several of
 * these exit non-zero precisely when there is nothing to update.
 */

/* Requires: a package manager, and an AUR helper for the secondary source. */
const char *cmd_updates_primary   = "/bin/checkupdates";
const char *cmd_updates_secondary = "/bin/paru -Qua";

/* Labels used in the notification, one per source. */
static const char label_updates_primary[]   = "Pacman Updates";
static const char label_updates_secondary[] = "AUR Updates";

/* System update command. Requires: a package manager. */
const char *args_update_cmd[] = {
	term_cmd,
	term_title_opt, "System Upgrade",
	"sh", "-c",
	"echo \"Upgrading system\" && paru",
	NULL
};

/* Show release info in bar */
const unsigned int show_release = 1;

/* Show update count in bar*/
const unsigned int show_update_count = 1;

/* Bar icons: the kernel (tux) glyph and the pending-updates glyph. */
static const char icon_system_kernel[] = "";
static const char icon_system_pkg[] = "󰏖";

/* Notification icons, one per update source. */
static const char icon_updates_primary[]   = "󰏖";
static const char icon_updates_secondary[] = "";

/*
 * Seconds before cached update counts are refreshed. Set to 0 to disable
 * caching entirely and query on every run.
 */
static const long update_cache_ttl = 3600;

/*
 * The cache is also invalidated whenever this path's mtime is newer than
 * it, so a post-transaction hook from the package manager makes the count
 * drop to zero immediately instead of waiting for the TTL. Point it at
 * whatever the package manager rewrites when it installs or removes
 * something. Set to "" to rely on the TTL alone.
 */
static const char update_watch_path[] = "/var/lib/pacman/local";
#endif

/* ============================================================
 * KEYBOARD BLOCK
 * ============================================================ */
#ifdef KEYBOARD_C

const unsigned int show_icon = 1;

/*
 * Keyboard layout switching script, run on left click.
 * Requires: a script of your own that cycles the layout. The default
 * points at dwm-xkbnext from https://github.com/dimgerasimou/binaries
 * A plain alternative: setxkbmap with your layouts, wrapped in a script.
 */
static const char path_language_switch[] = "~/.local/bin/dwm-xkbnext";
const char *args_language_switch[] = { "dwm-xkbnext", NULL };


/* Icon shown in the bar when show_icon is set. */
static const char icon_keyboard[] = " ";
#endif

/* ============================================================
 * MEMORY BLOCK
 * ============================================================ */
#ifdef MEMORY_C

const unsigned int show_icon = 1;

/* Task manager, opened on right click. Requires: htop, and term_cmd. */
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

/* Path to the status bar itself, restarted from the power menu.
 * Requires: dwmblocks. Adjust if yours is installed elsewhere. */
static const char path_dwmblocks[] = "/usr/local/bin/dwmblocks";
const char *args_dwmblocks[]        = {"dwmblocks", NULL};

/* Lock screen command. Requires: slock, or any locker you prefer. */
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
/*
 * Volume control script, run on middle click and scroll.
 * Requires: a script of your own accepting "up", "down" and "mute". The
 * default points at dwm-audio from https://github.com/dimgerasimou/binaries
 * A plain alternative: a wrapper around wpctl or pamixer.
 */
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
