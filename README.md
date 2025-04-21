# dwmblocks-blocks

This repository contains blocks, that can be used by a status line, e.g. dwmblocks. These blocks
are clickable for dwm, and support color (for dwm patched with status 2d). They are written using C,
mostly using system libraries. The complete library usage is WIP as well as customization for a wider
range of devices and drivers.

A nerd font must definetly be used in the window manager to render properly, as well as ttf-font-awesome.

Here is a list of the blocks, with a summary of its function and dependencies:

## battery

### Usage

Reports the battery level, status and optionally the power manager's status. Reads the battery info
from `/sys/class/power_supply/BAT@`. Check if the correct path is set in the `config.h` file.

### Dependencies

Optional:
 - optimus-manager

## bluetooth

### Usage

Returns the blueooth state, can toggle the bluetooth state and can open a TUI interface with the
bluetooth settings.

### Dependencies

- bluez

Optional:
- A bluetooth TUI manager (by default bluetuith running on st)

## date

### Usage

Returns current day, can send via notify a cute monthly calendar or optionaly launch a calendar through
a browser.

### Dependencies

Optional:
- A browser (by default: zen-browser)

## internet

### Usage

Returns the state, can notify the properties of the ethernet and wifi adapter. Can toggle wifi state,
spawn a utility to connect to wifi, or open a TUI utility.

### Dependencies

- libnm

Optional:
- A TUI interface (default: nmtui through network-manager package running on st)
- A wifi connection prompt (default: [dmenu-wifi-prompt](https://github.com/dimgerasimou/binaries))

## kernel

### Usage

Returns current kernel version and the number of packages to be updated, notifies more analiticly about their source, macman or aur and can perform a system upgrade.

### Dependencies

- An AUR package manager (default: paru)
- A utility to check pacman updates (default: checkupdates from package pacman-contrib)

Optional:
- A utility to perform system upgrade (default: paru running on st)

## keyboard

### Usage

Returns the current keyboard layout and optionally switch languages on click.

### Dependencies

- libx11
- libxkbcommon

Optional:
- A keyboard layout switcher (default: [keyboard.sh](https://github.com/dimgerasimou/binaries))

## memory

### Usage

Returns the memory that is currently used. Can optionally run a task manager.

### Dependencies

Optionally:
- A task manager (default: htop running on st)

## power

### Usage

Prints a power menu, that can: shutdown, restart, lock, restart the statusbar and optionally
pause the clipboard, delete the clipboard contents and switch the power mode from optimus manager.

### Dependencies

- xmenu
- [dwmblocks](https://github.com/dwmasyncblocks)
- A lock screen utility (default: [slock](https://github.com/slock))

Optional:
- clipmenu
- optimus-manager

## time

### Usage

Returns the current time in 24h format.

### Dependencies

No dependecies.

## volume

### Usage

Returns current volume and state, notifies it along the default source's and sink's info. Can optinally launch an equalizer application and can change volume or mute.

### Dependencies

- libpulse

Optional:
- A utility to control volume (default: [audiocontrol](https://github.com/dimgerasimou/binaries))
- An equalizer application (default: easyeffects)
