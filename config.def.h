/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H

static const char term_cmd[] = "st";
static const char term_title_opt[] = "-t";

/* ============================================================
 * GLOBAL SETTINGS
 * ============================================================ */
#ifdef UTILS_C

/* Log file path */
const char *path_log[] = {"$HOME", "window-manager.log", NULL};

#endif

/* ============================================================
 * BATTERY BLOCK
 * ============================================================ */
#ifdef BATTERY_C

/* Enable power management features (optimus-manager support) */
#define POWER_MANAGEMENT

#endif

/* ============================================================
 * BLUETOOTH BLOCK
 * ============================================================ */
#ifdef BLUETOOTH_C

/* TUI application for bluetooth settings */
const char *bt_tui_cmd[] = { term_cmd, "bluetuith", NULL };

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
const char *path_wifi_connect[] = {"$HOME", ".local", "bin", "dmenu-wifi-prompt", NULL};
const char *args_wifi_connect[] = {"dmenu-wifi-prompt", NULL};

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

#endif

/* ============================================================
 * KEYBOARD BLOCK
 * ============================================================ */
#ifdef KEYBOARD_C

const unsigned int show_icon = 1;

/* Keyboard layout switching script */
const char *path_language_switch[] = { "$HOME", ".local", "bin", "dwm-xkbswitch", NULL };
const char *args_language_switch[] = { "keyboard.sh", NULL };

#endif

/* ============================================================
 * MEMORY BLOCK
 * ============================================================ */
#ifdef MEMORY_C

/* Task manager application */
const char *args_task_manager[] = { term_cmd, "sh", "-c", "htop", NULL };

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
const char *path_dwmblocks[]        = {"usr", "local", "bin", "dwmblocks", NULL};
const char *args_dwmblocks[]        = {"dwmblocks", NULL};

/* Lock screen command */
const char *args_lockscreen[]       = {"slock", NULL};

/* Clipboard management */
const char *args_clipboard_delete[] = {"sh", "-c", "clipdel -d \".*\"", NULL};

#endif

/* ============================================================
 * VOLUME BLOCK
 * ============================================================ */
#ifdef VOLUME_C

/* Audio equalizer application */
const char *args_eqalizer[]        = {"easyeffects", NULL};

/* Volume control script and arguments */
const char *args_volume_increase[] = {"audiocontrol", "sink", "increase", NULL};
const char *args_volume_decrase[]  = {"audiocontrol", "sink", "decrease", NULL};
const char *args_volume_mute[]     = {"audiocontrol", "sink", "mute", NULL};
const char *path_volume_control[]  = {"$HOME", ".local", "bin", "dwm-audio-control", NULL};

#endif

#endif /* CONFIG_H */
