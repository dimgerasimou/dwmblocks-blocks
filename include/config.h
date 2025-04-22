/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef UTILS_C
const char *path_log[] = {"$HOME", "window-manager.log", NULL};
#endif

#ifdef BATTERY_C
#define POWER_MANAGEMENT
const char path_battery_kernel[] = "/sys/class/power_supply/BAT1";
#endif

#ifdef BLUETOOTH_C
const char *args_tui_settings[] = { "st", "-e", "bluetuith", NULL };
#endif

#ifdef DATE_C
const char *args_gui_calendar[] = { "zen-browser", "--new-window", "https://calendar.google.com", NULL };
#endif

#ifdef INTERNET_C
const char *args_tui_internet[] = {"st", "-t", "Network Configuration", "-e", "nmtui", NULL};
const char *path_wifi_connect[] = {"$HOME", ".local", "bin", "dmenu", "dmenu-wifi-prompt", NULL};
const char *args_wifi_connect[] = {"dmenu-wifi-prompt", NULL};
#endif

#ifdef KERNEL_C
const char *args_update_cmd[] = { "st", "-t", "System Upgrade", "-e", "sh", "-c", "echo \"Upgrading system\" && paru", NULL };
const char *cmd_aur_updates   = "/bin/paru -Qua";
const char *cmd_pm_updates    = "/bin/checkupdates";
#endif

#ifdef KEYBOARD_C
const char *path_language_switch[] = { "$HOME", ".local", "bin", "dwm", "keyboard.sh", NULL };
const char *args_language_switch[] = { "keyboard.sh", NULL };
#endif

#ifdef MEMORY_C
const char *args_task_manager[] = { "st", "-e", "sh", "-c", "htop", NULL };
#endif

#ifdef POWER_C
#define CLIPBOARD
#define POWER_MANAGEMENT
const char *path_dwmblocks[]        = {"usr", "local", "bin", "dwmblocks", NULL};
const char *args_dwmblocks[]        = {"dwmblocks", NULL};
const char *args_lockscreen[]       = {"slock", NULL};
const char *args_clipboard_delete[] = {"sh", "-c", "clipdel -d \".*\"", NULL};
#endif

#ifdef VOLUME_C
const char *args_eqalizer[]        = {"easyeffects", NULL};
const char *args_volume_increase[] = {"audiocontrol", "sink", "increase", NULL};
const char *args_volume_decrase[]  = {"audiocontrol", "sink", "decrease", NULL};
const char *args_volume_mute[]     = {"audiocontrol", "sink", "mute", NULL};
const char *path_volume_control[]  = {"$HOME", ".local", "bin", "dwm", "audiocontrol", NULL};
#endif

#endif /* CONFIG_H */