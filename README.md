# dwmblocks-blocks

This repository contains blocks, that can be used by a status line, e.g. dwmblocks. These blocks
are clickable for dwm, and support color (for dwm patched with status 2d). They are written using C,
mostly using system libraries. The complete library usage is WIP as well as customization for a wider
range of devices and drivers.

A nerd font must definetly be used in the window manager to render properly, as well as ttf-font-awesome.

Here is a list of the blocks, with a summary of its function and dependencies:

## battery

### Usage

Reports the battery level, status and the power manager's status.

### Dependencies

- optimus-manager

## bluetooth

### Usage

Prints a menu with options for bluetooth, or optionaly open the tui settings. Returns the bluetooth state as an icon.

### Dependencies

- st
- bluteuith (AUR) (or bluetui)
- bluez
- bluez-utils

## date

### Usage

Returns current day, can send via notify a cute monthly calendar or optionaly launch google calendar in firefox.

### Dependencies

- firefox

## internet

### Usage

Returns state as an icon, can notify about connection settings or optionaly launch either tui settings or the dmenu-wifiprompt script.

### Dependencies

- st
- libnm (compile time)
- network-manager
- dmenu-wifi-prompt

## kernel

### Usage

Returns current kernel version and the number of packages to be updated, notifies more analiticly about their source, macman or aur.

### Dependencies

- paru
- pacman-contrib (checkupdates)

## keyboard

### Usage

Returns the current keyboard layout.

### Dependencies

Compile time:

- libx11
- libxkbcommon
- keyboard.sh

## memory

### Usage

Returns the memory that is currently used. Can execute htop.

### Dependencies

- st
- htop

## power

### Usage

Prints a power menu, with some extra functionality.

### Dependencies

- xmenu
- clipmenu
- dwmblocks
- slock
- optimus-manager

## time

### Usage

Returns the current time in 24h format.

### Dependencies

No dependecies.

## volume

### Usage

Returns current volume and state, notifies it along the default source's and sink's info. Can launch easyeffects and can change volume or mute.

### Dependencies

- audiocontrol (in this repo)
- easyeffects
- libpulse (compile time)