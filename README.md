# statusblocks

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

Blocks are installed to `$(BLOCKDIR)`, which defaults to `~/.local/bin/statusblocks`. Point your dwmblocks configuration at that directory; `examples/blocks.h` is a ready-made starting point with suggested intervals and signals for every block, plus the pacman hook that pairs with the system block.

Useful variables:

| Variable   | Default                  | Purpose                                    |
|------------|--------------------------|--------------------------------------------|
| `PREFIX`   | `$(HOME)/.local`         | Installation prefix                        |
| `BINDIR`   | `$(PREFIX)/bin`          | Binary directory                           |
| `BLOCKDIR` | `$(BINDIR)/statusblocks` | Where the blocks themselves are installed  |
| `DESTDIR`  | *(empty)*                | Staging root, for packaging                |
| `CC`       | `cc`                     | Compiler                                   |
| `CFLAGS`   | `-Os`                    | Optimisation and extra flags               |
| `BLOCKS`   | all blocks               | Which blocks to build and install          |
| `COLOR`    | `1`                      | Set to `0` for uncoloured build output     |

The warning flags are held in a separate `WARNINGS` variable, so overriding `CFLAGS` does not switch them off. Dependency include paths are passed as `-isystem` rather than `-I`, so the strict warning set applies to this project's code and not to third-party headers.

Other targets:

```bash
make time          # build a single block
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
- `libX11` (only for reading colours from Xresources; not needed with `-DNO_COLOR`)

Per block, additionally: `libxkbfile` (keyboard), `dbus-1` (bluetooth), `libnm` and `glib-2.0` (internet), `libpulse` (volume).

The blocks read `/proc` and `/sys`, so they are Linux-only. Runtime dependencies are listed per block below and in the External programs section.

## Colours

Colours have sensible defaults in `config.h`, so the bar is themed out of the box with no further setup:

```c
static const char *const clr_defaults[] = {
	"#F38BA8",  /* clr_bat_crt  battery critical */
	"#FAB387",  /* clr_bat_low  battery low      */
	...
};
```

Set an entry to `""` to leave that colour unset, so the block renders in the status bar's own colour. A compile-time check fails the build if the list and `enum Color` ever fall out of step.

Any entry in the X resource database overrides the matching default at runtime, so a colour can be changed without rebuilding. Each colour is looked up first as `statusblocks.<name>`, then as `*<name>`, and values must be exactly `#RRGGBB`; anything malformed is ignored in favour of the default.

```
statusblocks.clr_bat_crt:  #F38BA8
statusblocks.clr_date:     #CBA6F7
statusblocks.clr_tim:      #FAB387
statusblocks.clr_cal:      #F38BA8
```

Apply with `xrdb -merge ~/.Xresources`.

Resolved colours are cached in `$XDG_RUNTIME_DIR/statusblocks-colors`, so only the first block to run in a session opens an X connection. The cache is rebuilt automatically when `~/.Xresources` is newer than it, or when the block binary is (so editing `clr_defaults` and rebuilding takes effect immediately). `$XDG_RUNTIME_DIR` is cleared at logout. To force a rebuild by hand:

```bash
rm -f "$XDG_RUNTIME_DIR/statusblocks-colors"
```

If X is unreachable the configured defaults are used on their own, and nothing is cached, so colours appear as soon as X is available.

### Using a different status bar

The blocks are ordinary programs that print a line, so they work with any bar that can run a command. Only the colour escape is bar-specific, and it is two macros in `config.h`:

| Bar                          | `CLR_FMT`               | `CLR_NRM`  |
|------------------------------|-------------------------|------------|
| dwm + status2d *(default)*   | `"^c%s^"`               | `"^d^"`    |
| Pango markup (waybar, i3bar) | `"<span color='%s'>"`   | `"</span>"`|
| polybar native               | `"%%{F%s}"`             | `"%%{F-}"` |
| none                         | `"%.0s"`                | `""`       |

`CLR_FMT` must contain exactly one `%s`, which receives a `#RRGGBB` string; the compiler checks this. Building with `-DNO_COLOR` compiles the colour lookup out entirely and removes the X dependency.

## Paths

Paths to helper scripts are single strings in `config.h` and are expanded at runtime:

```c
static const char path_volume_control[] = "~/.local/bin/dwm-audio";
```

A leading `~` or `~/` becomes `$HOME`, and `$VAR` or `${VAR}` anywhere in the string is replaced by that environment variable. A `$` that is not followed by a variable name is left alone. An unset variable is an error: the action is skipped and a message is written to stderr, rather than running something from a half-built path.

## External programs

Most blocks display something with no help, but the click actions often shell out. Every such setting in `config.h` is marked `Requires:` with what it needs. In summary:

Blocks that open a TUI use `$TERM` if it names a real program, then `$TERMINAL`, then `term_cmd` from `config.h`. `$TERM` normally holds a terminfo name such as `st-256color` rather than a program, so in practice `$TERMINAL` or `term_cmd` is what gets used.

| Block     | Needed for display | Needed for clicks                                    |
|-----------|--------------------|------------------------------------------------------|
| time      | —                  | —                                                     |
| date      | —                  | a browser (default: zen-browser)                      |
| memory    | —                  | a terminal and htop                                   |
| battery   | —                  | optimus-manager, only with `POWER_MANAGEMENT`         |
| keyboard  | —                  | a layout-switching script of your own                 |
| bluetooth | bluez running      | a bluetooth TUI (default: bluetuith)                  |
| internet  | NetworkManager     | xmenu, a TUI, and a wifi prompt script of your own    |
| volume    | PulseAudio/PipeWire| a volume script of your own; an equalizer for middle click |
| system    | a package manager  | a terminal and an upgrade command                     |
| power     | —                  | xmenu, a locker (default: slock), dwmblocks           |

The three "script of your own" entries default to helpers from [dimgerasimou/binaries](https://github.com/dimgerasimou/binaries). Substitute your own: any executable accepting the same arguments will do, for instance a `setxkbmap` wrapper for the keyboard block or a `wpctl` wrapper for volume. A block whose program is missing still displays correctly; the click does nothing and writes to stderr, which dwmblocks discards, so run the block by hand in a terminal to see the message.

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

### system

#### Usage

Returns the number of packages to be updated and the current kernel version, notifies the counts from each configured update source, and can perform a system upgrade.

Left click notifies the update counts; right click runs the upgrade command.

The block counts updates from two configurable sources and is not tied to Arch: set `cmd_updates_primary`, `cmd_updates_secondary`, their labels, and `update_watch_path` in `config.h`. Equivalents for apt, dnf and xbps are given in the comments there; set a command to `""` to disable that source. Each command should print one line per pending update, and the exit status is ignored, because several package managers exit non-zero precisely when there is nothing to update.

Both update commands hit the network — `checkupdates` syncs the repository databases into a temporary path, and `paru -Qua` queries the AUR — so their results are cached in `$XDG_RUNTIME_DIR/statusblocks-updates`. The cache is discarded when any of these happens:

- `update_cache_ttl` seconds have passed (one hour by default; set it to `0` in `config.h` to disable caching and query on every run)
- the pacman local database at `pacman_local_db` is newer than the cache
- the block is left-clicked, which always refreshes

The second rule is what makes a post-transaction hook work. dwmblocks re-executes the block on a signal rather than delivering the signal to it, so the block cannot tell a signalled run from a scheduled one. It can, however, see that a transaction happened, because installing or removing anything updates the mtime of `/var/lib/pacman/local`. A hook that signals dwmblocks therefore makes the count drop to zero on the very next run, with no cooperation needed from the hook itself:

```ini
[Trigger]
Operation = Install
Operation = Upgrade
Operation = Remove
Type = Package
Target = *

[Action]
Description = Refresh the statusblocks system block
When = PostTransaction
Exec = /usr/bin/pkill -RTMIN+<n> dwmblocks
```

This also avoids the obvious alternative of having the hook delete the cache file: hooks run as root and would not have a usable `$XDG_RUNTIME_DIR` for the logged-in user.

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

Every setting in `config.h` gates a branch of code, so when changing one, build with the other combinations too — an unselected branch is exactly the kind that quietly stops compiling.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](./LICENSE) file for details.
