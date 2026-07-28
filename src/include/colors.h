/* See LICENSE file for copyright and license details. */

#ifndef COLORS_H
#define COLORS_H

#include <stddef.h>

#include "config.h"

/*
 * Fallbacks for configurations predating the configurable escape format.
 * The defaults target dwm's status2d patch; see config.def.h.
 */
#ifndef CLR_FMT
#define CLR_FMT "^c%s^"
#endif

#ifndef CLR_NRM
#define CLR_NRM "^d^"
#endif

/* Room for a rendered escape. Generous, since CLR_FMT is user-supplied. */
#define CLR_LEN 64

/* Length of a "#RRGGBB" string, including the terminator. */
#define CLR_HEX_LEN 8

enum Color {
	clr_bat_crt = 0,
	clr_bat_low,
	clr_bat_nrm,
	clr_bat_chg,
	clr_bt,
	clr_date,
	clr_net_nrm,
	clr_net_err,
	clr_sys_pkg,
	clr_sys_nrm,
	clr_kbd,
	clr_mem,
	clr_pwr,
	clr_tim,
	clr_vol_nrm,
	clr_vol_mut,
	clr_cal,
	CLR_SIZE
};

/*
 * Makes the colour table available to clr_get(). Colours come from
 * clr_defaults in config.h, overridden by any matching entry in the X
 * resource database. Resolved values are cached for the session, so only
 * the first block to run in a session pays for an X connection.
 *
 * Never fails: if the cache is unusable and X is unreachable, the
 * configured defaults are used on their own.
 */
void clr_init(void);

/*
 * Returns the status-line escape for 'clr', or "" if it is unset.
 * The returned string points into static storage: it must not be freed
 * and stays valid for the lifetime of the process.
 */
const char *clr_get(enum Color clr);

/*
 * Returns the resolved "#RRGGBB" string for 'clr', or "" if it is unset.
 * Use this where a raw colour is needed rather than a status-line escape,
 * such as the Pango markup in the date block's calendar. Same storage
 * rules as clr_get().
 */
const char *clr_hex(enum Color clr);

#endif /* COLORS_H */
