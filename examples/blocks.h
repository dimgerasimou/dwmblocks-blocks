/*
 * Example status bar configuration for dwmblocks-blocks.
 *
 * Two forms are given below, since dwmblocks forks differ. Use whichever
 * matches yours and delete the other; the intervals and signals are the
 * point, not the syntax.
 *
 * Intervals are chosen so that each block costs about what it is worth:
 *
 *   time      60   the clock only shows hours and minutes
 *   date     3600  the day changes once a day; the hourly tick is slack
 *   battery    30  fast enough to notice a drain, cheap to read
 *   memory     10  a plain read of /proc/meminfo
 *   keyboard    0  signal only; nothing changes it but the user
 *   volume      0  signal only; the volume script signals after changing
 *   bluetooth  60  a D-Bus round trip, so not more often
 *   internet   10  cheap, and the signal strength moves
 *   system    600  the counts are cached anyway, see below
 *   power       0  a button, never updates on its own
 *
 * Blocks with interval 0 are updated only by a signal. Send one with
 * `pkill -RTMIN+<signal> dwmblocks` after the relevant event: the volume
 * script should signal 8, the layout switcher 4, and so on.
 *
 * The system block queries the network, so it caches its counts (see
 * update_cache_ttl in config.h). Pair it with this pacman hook, dropped in
 * /etc/pacman.d/hooks/, so the count drops to zero right after an upgrade
 * rather than at the next interval:
 *
 *   [Trigger]
 *   Operation = Install
 *   Operation = Upgrade
 *   Operation = Remove
 *   Type = Package
 *   Target = *
 *
 *   [Action]
 *   Description = Refresh the dwmblocks system block
 *   When = PostTransaction
 *   Exec = /usr/bin/pkill -RTMIN+9 dwmblocks
 *
 * The block already detects the transaction on its own by watching
 * update_watch_path, so the hook only needs to make it re-run.
 *
 * BLOCKDIR below must match where `make install` put the binaries; the
 * default is ~/.local/bin/dwmblocks.
 */

#define BLOCKDIR "/home/user/.local/bin/dwmblocks"

/* ------------------------------------------------------------------
 * Form A: vanilla dwmblocks, an array of Block structs.
 * ------------------------------------------------------------------ */
static const Block blocks[] = {
	/* icon  command                     interval  signal */
	{ "",    BLOCKDIR "/internet",         10,      1 },
	{ "",    BLOCKDIR "/bluetooth",        60,      2 },
	{ "",    BLOCKDIR "/memory",           10,      3 },
	{ "",    BLOCKDIR "/keyboard",          0,      4 },
	{ "",    BLOCKDIR "/volume",            0,      5 },
	{ "",    BLOCKDIR "/battery",          30,      6 },
	{ "",    BLOCKDIR "/date",           3600,      7 },
	{ "",    BLOCKDIR "/time",             60,      8 },
	{ "",    BLOCKDIR "/system",          600,      9 },
	{ "",    BLOCKDIR "/power",             0,     10 },
};

static char delim[] = " ";
static unsigned int delimLen = 5;

/* ------------------------------------------------------------------
 * Form B: dwmasyncblocks, an X macro of (icon, command, interval, signal).
 * ------------------------------------------------------------------ */
#define CLICKABLE_BLOCKS 1
#define LEADING_DELIMITER 1
#define DELIMITER " "
#define TRIM_TRAILING_SPACES 0

#define BLOCK(NAME) "$HOME/.local/bin/dwmblocks/" #NAME

const Block blocks[] = {
	/*Command             Update Interval   Update Signal */
	{ BLOCK("volume"),    0,                10 },
	{ BLOCK("memory"),    6,                12 },
	{ BLOCK("keyboard"),  0,                3  },
	{ BLOCK("system"),    360,              4  },
	{ BLOCK("date"),      300,              6  },
	{ BLOCK("time"),      1,                5  },
	{ BLOCK("internet"),  5,                1  },
	{ BLOCK("battery"),   5,                2  },
	{ BLOCK("bluetooth"), 5,                15 },
	{ BLOCK("power"),     0,                14 },
};
