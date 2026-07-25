#ifndef COLORS_H
#define COLORS_H

#include <stdlib.h>

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

char * clr_get(enum Color clr);
void clr_init(const enum Color clrs[], size_t clrs_num);

#endif /* COLORS_H */
