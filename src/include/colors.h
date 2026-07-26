/* See LICENSE file for copyright and license details. */

#ifndef COLORS_H
#define COLORS_H

#define CLR_NRM "^d^"

enum Color {
	clr_bat_crt = 0,
	clr_bat_low,
	clr_bat_nrm,
	clr_bat_chg,
	clr_bt,
	clr_date,
	clr_net_nrm,
	clr_net_err,
	clr_krn_pkg,
	clr_krn_nrm,
	clr_kbd,
	clr_mem,
	clr_pwr,
	clr_tim,
	clr_vol_nrm,
	clr_vol_mut,
	CLR_SIZE
};

/*
 * Makes the colour table available to clr_get(). Resolved colours are
 * cached for the session, so only the first block to run in a session
 * pays for an X connection.
 *
 * Never fails: if the cache is unusable and X is unreachable, every
 * colour is left unset and blocks render in the bar's default colour.
 */
void clr_init(void);

/*
 * Returns the dwm colour escape for 'clr', or "" if it was never loaded.
 * The returned string points into static storage: it must not be freed
 * and stays valid for the lifetime of the process.
 */
const char *clr_get(enum Color clr);

#endif /* COLORS_H */
