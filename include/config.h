#ifndef CONFIG_H
#define CONFIG_H

/* battery.c */
#ifdef BATTERY_C
#define POWER_MANAGEMENT
const char battery_kernel_path[] = "/sys/class/power_supply/BAT1";  /* check if this exists and match */
#endif

#ifdef DATE_C
const char *gui_calender_path[] = { "bin", "zen-browser", NULL };
const char *gui_calendar_args[] = { "zen-browser", "--new-window", "https://calendar.google.com", NULL };
#endif


#endif /* CONFIG_H */