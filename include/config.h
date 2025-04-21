#ifndef CONFIG_H
#define CONFIG_H

#ifdef BATTERY_C
#define POWER_MANAGEMENT
const char battery_kernel_path[] = "/sys/class/power_supply/BAT1";
#endif

#ifdef DATE_C
const char *gui_calendar_args[] = { "zen-browser", "--new-window", "https://calendar.google.com", NULL };
#endif

#ifdef KERNEL_C
const char *update_cmd_args[] = { "st", "-t", "System Upgrade", "-e", "sh", "-c", "echo \"Upgrading system\" && paru", NULL };
const char *aur_updates_cmd   = "/bin/paru -Qua";
const char *pm_updates_cmd    = "/bin/checkupdates";
#endif

#ifdef KEYBOARD_C
const char *language_switch_path[] = { "$HOME", ".local", "bin", "dwm", "keyboard.sh", NULL };
const char *language_switch_args[] = { "keyboard.sh", NULL };
#endif

#ifdef MEMORY_C
const char *task_manager_args[] = { "st", "-e", "sh", "-c", "htop", NULL };
#endif

#ifdef INTERNET_C
const char *tui_internet_args[] = {"st", "-t", "Network Configuration", "-e", "nmtui", NULL};
const char *wifi_connect_path[] = {"$HOME", ".local", "bin", "dmenu", "dmenu-wifi-prompt", NULL};
const char *wifi_connect_args[] = {"dmenu-wifi-prompt", NULL};
#endif

#endif /* CONFIG_H */