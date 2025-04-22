# dwmblocks-blocks

This repository contains blocks, executables that display a single part of a status line. These can be used by a status line application, e.g. [dwmblocks](https://github.com/dimgerasimou/dwmasyncblocks).

If the window manager and status application are configured correctly, the blocks support colour and are clickable. For dwm, it needs to be patched with [statuscmd](https://dwm.suckless.org/patches/statuscmd/) and [status2d](https://dwm.suckless.org/patches/status2d/)

They are written using C, mostly using system libraries.

A nerd font must definetly be used in the window manager to render the icons properly, as well as Font Awesome.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for details.

## Blocks

Here is a list of the blocks, with a summary of their functions and dependencies:

### battery

#### Usage

Reports the battery level, status and optionally the power manager's status. Reads the battery info from `/sys/class/power_supply/BAT@`. 

Check if the correct path is set in the `config.h` file.

#### Dependencies

Optional:
 - optimus-manager

### bluetooth

#### Usage

Returns the blueooth state, can toggle the bluetooth state and optionally open a TUI interface with the bluetooth settings.

#### Dependencies

- bluez

Optional:
- A bluetooth TUI manager (by default bluetuith running on st)

### date

#### Usage

Returns current day, notify a cute monthly calendar and optionaly launch a calendar through
a browser.

#### Dependencies

Optional:
- A browser (by default: zen-browser)

### internet

#### Usage

Returns the state, notify the properties of the ethernet and wifi adapters. Can toggle wifi state, optionally spawn a utility to connect to wifi and open a TUI utility.

#### Dependencies

- libnm

Optional:
- A TUI interface (default: nmtui through network-manager package running on st)
- A wifi connection prompt (default: [dmenu-wifi-prompt](https://github.com/dimgerasimou/binaries))

### kernel

#### Usage

Returns current kernel version and the number of packages to be updated, notifies the number of aur packages or pacman packages to be upgraded and can perform a system upgrade.

#### Dependencies

- An AUR package manager (default: paru)
- A utility to check pacman updates (default: checkupdates from package pacman-contrib)

Optional:
- A utility to perform system upgrade (default: paru running on st)

### keyboard

#### Usage

Returns the current keyboard layout and optionally switches language on click.

#### Dependencies

- libx11
- libxkbcommon

Optional:
- A keyboard layout switcher (default: [keyboard.sh](https://github.com/dimgerasimou/binaries))

### memory

#### Usage

Returns the memory that is currently used and optionally runs a task manager.

#### Dependencies

Optionally:
- A task manager (default: htop running on st)

### power

#### Usage

Prints a power menu that can: shutdown, restart, lock, restart the statusbar and optionally
pause the clipboard, delete the clipboard contents and switch the power mode from optimus manager.

#### Dependencies

- xmenu
- [dwmblocks](https://github.com/dimgerasimou/dwmasyncblocks)
- A lock screen utility (default: [slock](https://github.com/dimgerasimou/slock))

Optional:
- clipmenu
- optimus-manager

### time

#### Usage

Returns the current time in 24h format.

#### Dependencies

No dependecies.

### volume

#### Usage

Returns current volume and state, notifies it along the default source's and sink's info. Can optionally launch a equalizer application and can change the volume or mute the volume.

#### Dependencies

- libpulse

Optional:
- A utility to control volume (default: [audiocontrol](https://github.com/dimgerasimou/binaries))
- An equalizer application (default: easyeffects)
