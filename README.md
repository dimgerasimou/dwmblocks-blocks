# dwmblocks-blocks

This repository contains blocks, executables that display a single part of a status line. These can be used by a status line application, e.g. [dwmblocks](https://github.com/dimgerasimou/dwmasyncblocks).

If the window manager and status application are configured correctly, the blocks support colour and are clickable. For dwm, it needs to be patched with [statuscmd](https://dwm.suckless.org/patches/statuscmd/) and [status2d](https://dwm.suckless.org/patches/status2d/).

They are written in C, mostly using system libraries.

A nerd font must definitely be used in the window manager to render the icons properly, as well as Font Awesome.

## Build / install

Configuration lives in `config.h`, which is created from `config.def.h` on the first build. Edit it to match your setup, then run:

```bash
make
make install
```

Blocks are installed to `$(BLOCKDIR)`, which defaults to `~/.local/bin/dwmblocks`. Point your dwmblocks configuration at that directory.

Useful variables:

| Variable   | Default                  | Purpose                                    |
|------------|--------------------------|--------------------------------------------|
| `PREFIX`   | `$(HOME)/.local`         | Installation prefix                        |
| `BINDIR`   | `$(PREFIX)/bin`          | Binary directory                           |
| `BLOCKDIR` | `$(BINDIR)/dwmblocks`    | Where the blocks themselves are installed  |
| `DESTDIR`  | *(empty)*                | Staging root, for packaging                |
| `CC`       | `cc`                     | Compiler                                   |
| `CFLAGS`   | `-Os`                    | Optimisation and extra flags               |
| `BLOCKS`   | all blocks               | Which blocks to build and install          |
| `COLOR`    | `1`                      | Set to `0` for uncoloured build output     |

The warning flags are held in a separate `WARNINGS` variable, so overriding `CFLAGS` does not switch them off. Dependency include paths are passed as `-isystem` rather than `-I`, so the strict warning set applies to this project's code and not to third-party headers.

Other targets:

```bash
make time          # build a single block
make debug         # rebuild everything with ASan/UBSan and -fanalyzer
make test          # build and run the test suite under sanitizers
make clean
make uninstall
```

Each block links only against the libraries it actually uses, so a missing `libnm` or `libpulse` only prevents that one block from building. To skip blocks entirely, edit the `BLOCKS` list in the `Makefile` or override it:

```bash
make BLOCKS="time date battery"
```

### Dependencies

Required for all blocks:

- C compiler (gcc/clang) and make
- `pkg-config`
- `libnotify`
- `libX11`

Per block, additionally: `libxkbfile` (keyboard), `dbus-1` (bluetooth), `libnm` and `glib-2.0` (internet), `libpulse` (volume). Runtime dependencies are listed per block below.

## Colours

Colours are read at runtime from the X resource database, so changing a colour does not require a rebuild. There is no `colorscheme.h`; that file was removed and is no longer needed.

Resolved colours are cached in `$XDG_RUNTIME_DIR/dwmblocks-colors`, so only the first block to run in a session opens an X connection. The cache is rebuilt automatically when `~/.Xresources` is newer than it, and `$XDG_RUNTIME_DIR` is cleared at logout. To force a rebuild mid-session:

```bash
rm -f "$XDG_RUNTIME_DIR/dwmblocks-colors"
```

Each colour is looked up first as `dwmblocks.<name>`, then as `*<name>`. Values must be exactly `#RRGGBB`. Anything missing or malformed is skipped, and the block renders in the status bar's default colour.

Example `~/.Xresources`:

```
dwmblocks.clr_bat_crt:  #F38BA8
dwmblocks.clr_bat_low:  #FAB387
dwmblocks.clr_bat_nrm:  #A6E3A1
dwmblocks.clr_bat_chg:  #A6E3A1
dwmblocks.clr_bt:       #89B4FA
dwmblocks.clr_date:     #CBA6F7
dwmblocks.clr_net_nrm:  #94E2D5
dwmblocks.clr_net_err:  #F38BA8
dwmblocks.clr_krn_pkg:  #F9E2AF
dwmblocks.clr_krn_nrm:  #89DCEB
dwmblocks.clr_kbd:      #B4BEFE
dwmblocks.clr_mem:      #F5C2E7
dwmblocks.clr_pwr:      #F38BA8
dwmblocks.clr_tim:      #FAB387
dwmblocks.clr_vol_nrm:  #A6E3A1
dwmblocks.clr_vol_mut:  #6C7086
```

Apply with `xrdb -merge ~/.Xresources`.

Building with `-DNO_COLOR` compiles the colour lookup out entirely; every block then emits plain, uncoloured text and no longer needs an X connection for colours.

If X is unreachable, blocks warn once on stderr and render without colour rather than failing.

The calendar drawn by the date block uses Pango markup rather than the status bar's colour escapes, so its accent colour is a compile-time setting, found in the date section of `config.h`:

```c
#define CAL_ACCENT "#F38BA8"
```

## Icons and menus

Every icon and xmenu prompt lives in `config.h` alongside the commands it belongs with, so changing a glyph never means editing source. Look for `icons_battery`, `icon_time`, `menu_power`, and friends under the matching `#ifdef` block.

Icons are Nerd Font glyphs, so a patched font must be configured in the window manager for them to render.

## Blocks

Here is a list of the blocks, with a summary of their functions and dependencies.

### battery

#### Usage

Reports the battery level, status and optionally the power manager's status. The battery is located automatically by scanning `/sys/class/power_supply` for the first device whose `type` is `Battery`, so no path needs configuring. With no battery present the block renders the empty icon rather than disappearing.

Left click notifies the current capacity and status.

#### Dependencies

Optional:
- optimus-manager (enable with `POWER_MANAGEMENT` in `config.h`)

### bluetooth

#### Usage

Returns the bluetooth state, can toggle the bluetooth state and optionally open a TUI interface with the bluetooth settings.

Left click opens the TUI; middle click toggles the adapter. Only `hci0` and `hci1` are checked.

#### Dependencies

- bluez
- dbus

Optional:
- A bluetooth TUI manager (by default bluetuith running on st)

### date

#### Usage

Returns the current day, notifies a cute monthly calendar and optionally launches a calendar through a browser.

Left click shows the calendar; right click opens the browser.

#### Dependencies

Optional:
- A browser (by default: zen-browser)

### internet

#### Usage

Returns the state, notifies the properties of the ethernet and wifi adapters. Can toggle wifi state, optionally spawn a utility to connect to wifi and open a TUI utility.

Left click notifies device info; right click opens an xmenu with the wifi actions.

#### Dependencies

- libnm
- glib-2.0
- xmenu (for the right click menu)

Optional:
- A TUI interface (default: nmtui through network-manager package running on st)
- A wifi connection prompt (default: [dmenu-wifi-prompt](https://github.com/dimgerasimou/binaries))

### kernel

#### Usage

Returns the current kernel version and the number of packages to be updated, notifies the number of AUR or pacman packages to be upgraded, and can perform a system upgrade.

Left click notifies the update counts; right click runs the upgrade command.

Note that `checkupdates` exits 2 and `paru` exits 1 when there is nothing to update, so the exit status is ignored and only the line count is used.

Both update commands query the network, so their results are cached in `$XDG_RUNTIME_DIR/dwmblocks-updates` for `update_cache_ttl` seconds (one hour by default; set it in `config.h`). A left click always bypasses the cache and refreshes. If you run a pacman hook, writing `"<aur> <pacman>"` to that file is a cheaper way to keep the count current.

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
- libxkbfile

Optional:
- A keyboard layout switcher (default: [dwm-xkbnext](https://github.com/dimgerasimou/binaries))

### memory

#### Usage

Returns the memory that is currently used, computed as `MemTotal - MemAvailable` from `/proc/meminfo`, and optionally runs a task manager.

#### Dependencies

Optional:
- A task manager (default: htop running on st)

### power

#### Usage

Prints a power menu that can: shutdown, restart, lock, restart the statusbar and optionally pause the clipboard, delete the clipboard contents and switch the power mode from optimus manager.

#### Dependencies

- xmenu
- [dwmblocks](https://github.com/dimgerasimou/dwmasyncblocks)
- A lock screen utility (default: [slock](https://github.com/dimgerasimou/slock))

Optional:
- clipmenu (enable with `CLIPBOARD` in `config.h`)
- optimus-manager (enable with `POWER_MANAGEMENT` in `config.h`)

### time

#### Usage

Returns the current time in 24h format.

#### Dependencies

No dependencies.

### volume

#### Usage

Returns the current volume and state, notifies it along with the default source's and sink's info. Can optionally launch an equalizer application and can change or mute the volume.

#### Dependencies

- libpulse

Optional:
- A utility to control volume (default: [dwm-audio](https://github.com/dimgerasimou/binaries))
- An equalizer application (default: easyeffects)

## Development

Blocks never abort on the render path: a failure warns on stderr and prints a placeholder, because a block that exits silently leaves the bar showing stale text.

Click handling is table-driven. A block declares which buttons it answers and `dispatch()` in `utils` does the rest:

```c
static const struct Button buttons[] = {
	{ 1, on_left },
	{ 3, on_right },
};

dispatch(buttons, LEN(buttons), NULL);
```

The build uses a strict warning set (`-Wconversion`, `-Wsign-conversion`, `-Wshadow`, `-Wformat=2`, and friends) and the tree builds warning-free. Please keep it that way.

```bash
make test    # unit tests, run under AddressSanitizer and UBSan
make debug   # full rebuild with sanitizers and, on GCC, -fanalyzer
```

Tests live in `src/tests/`. `make test` builds and runs each one; a non-zero exit fails the build. `test-calendar` checks the date arithmetic against `mktime` for every day between 2020 and 2030.

CI builds every combination of the `POWER_MANAGEMENT`, `CLIPBOARD` and `NO_COLOR` toggles under both GCC and Clang. Config branches that are never compiled are exactly the ones that rot, so please keep the matrix green rather than trimming it.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for details.
