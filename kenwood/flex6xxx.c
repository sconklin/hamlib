/*
 *  Hamlib FlexRadio backend - 6K series rigs
 *  Copyright (c) 2002-2009 by Stephane Fillod
 *  Copyright (C) 2010,2011,2012,2013 by Nate Bargmann, n0nb@arrl.net
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  See the file 'COPYING.LIB' in the main Hamlib distribution directory for
 *  the complete text of the GNU Lesser Public License version 2.1.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include <hamlib/rig.h>
#include "kenwood.h"
#include "bandplan.h"
#include "flex.h"
#include "token.h"
#include "cal.h"

#define F6K_MODES (RIG_MODE_CW|RIG_MODE_SSB|RIG_MODE_AM|RIG_MODE_FM|RIG_MODE_PKTLSB|RIG_MODE_PKTUSB)

#define F6K_FUNC_ALL (RIG_FUNC_VOX)

#define F6K_VFO (RIG_VFO_A|RIG_VFO_B)
#define F6K_VFO_OP (RIG_OP_UP|RIG_OP_DOWN)

#define F6K_ANTS (RIG_ANT_1|RIG_ANT_2|RIG_ANT_3)


static rmode_t flex_mode_table[KENWOOD_MODE_TABLE_MAX] = {
    [0] = RIG_MODE_LSB,
    [1] = RIG_MODE_USB,
    [2] = RIG_MODE_CW,
    [3] = RIG_MODE_FM,
    [4] = RIG_MODE_AM,
    [5] = RIG_MODE_PKTLSB, // DIGL
    [6] = RIG_MODE_PKTUSB, // DIGU
};

static struct kenwood_priv_caps f6k_priv_caps  = {
	.cmdtrm =  EOM_KEN,
	.mode_table = flex_mode_table,
};


/* F6K specific function declarations */
//int f6k_get_mode(RIG *rig, vfo_t vfo, rmode_t *mode, pbwidth_t *width);
//int f6k_set_mode(RIG *rig, vfo_t vfo, rmode_t mode, pbwidth_t width);
//int f6k_set_vfo(RIG *rig, vfo_t vfo);
//int f6k_set_ext_level(RIG *rig, vfo_t vfo, token_t token, value_t val);
//int f6k_get_ext_level(RIG *rig, vfo_t vfo, token_t token, value_t *val);
//int f6k_set_rit(RIG * rig, vfo_t vfo, shortfreq_t rit);
//int f6k_set_xit(RIG * rig, vfo_t vfo, shortfreq_t rit);
//int f6k_set_split_mode(RIG *rig, vfo_t vfo, rmode_t tx_mode, pbwidth_t tx_width);
//int f6k_get_split_mode(RIG *rig, vfo_t vfo, rmode_t *tx_mode, pbwidth_t *tx_width);
//int f6k_get_level(RIG *rig, vfo_t vfo, setting_t level, value_t *val);
//int f6k_get_func(RIG *rig, vfo_t vfo, setting_t func, int *status);

/* Private helper functions */
int set_rit_xit(RIG *rig, shortfreq_t rit);


/*
 * F6K rig capabilities.
 */
const struct rig_caps f6k_caps = {
	.rig_model =		RIG_MODEL_F6K,
	.model_name =		"6xxx",
	.mfg_name =		"FlexRadio",
	.version =		"20130717",
	.copyright =		"LGPL",
	.status =		RIG_STATUS_BETA,
	.rig_type =		RIG_TYPE_TRANSCEIVER,
	.ptt_type =		RIG_PTT_RIG,
	.dcd_type =		RIG_DCD_NONE,
	.port_type =		RIG_PORT_NETWORK,
	//.serial_rate_min =	4800,
	//.serial_rate_max =	38400,
	//.serial_data_bits =	8,
	//.serial_stop_bits =	2,
	//.serial_parity =	RIG_PARITY_NONE,
	//.serial_handshake =	RIG_HANDSHAKE_NONE,
	//.write_delay =		0,	/* Timing between bytes */
	//.post_write_delay =	100,	/* Timing between command strings */
	// -spc- TODO set this reasonably
	.timeout =		600,	/* FA and FB make take up to 500 ms on band change */
	.retry =		3,

	.has_get_func =		RIG_FUNC_NONE, /* has VOX but not implemented here */
	.has_set_func =		RIG_FUNC_NONE,
	.has_get_level =	RIG_LEVEL_NONE,
	.has_set_level =	RIG_LEVEL_NONE,
	.has_get_parm =		RIG_PARM_NONE,
	.has_set_parm =		RIG_PARM_NONE,	/* FIXME: parms */
	.level_gran =		{},		/* FIXME: granularity */
	.parm_gran =		{},
	//.extlevels = 		elecraft_ext_levels,
	//.extparms =		kenwood_cfg_params,
	.preamp =		{ RIG_DBLST_END, },
	.attenuator =		{ RIG_DBLST_END, },
	.max_rit =		Hz(0),
	.max_xit =		Hz(0),
	.max_ifshift =		Hz(0),
	.vfo_ops =		RIG_OP_NONE,
	.targetable_vfo =	RIG_TARGETABLE_FREQ|RIG_TARGETABLE_MODE,
	.transceive =		RIG_TRN_RIG,
	.bank_qty =		0,
	.chan_desc_sz =		0,

	.chan_list =		{ RIG_CHAN_END },

	.rx_range_list1 =  {
		{kHz(30), MHz(77), F6K_MODES, -1, -1, F6K_VFO, F6K_ANTS},
		{MHz(135), MHz(165), F6K_MODES, -1,- 1, F6K_VFO, F6K_ANTS},
		RIG_FRNG_END,
	  }, /* rx range */
	.tx_range_list1 =  {
		FRQ_RNG_HF(1, F6K_MODES, mW(10), W(100), F6K_VFO, F6K_ANTS),
		FRQ_RNG_6m(1, F6K_MODES, mW(10), W(100), F6K_VFO, F6K_ANTS),
		FRQ_RNG_2m(1, F6K_MODES, mW(10), W(100), F6K_VFO, F6K_ANTS),
		RIG_FRNG_END,
	  }, /* tx range */

	.rx_range_list2 =  {
		{kHz(30), MHz(77), F6K_MODES, -1, -1, F6K_VFO, F6K_ANTS},
		{ MHz(135), MHz(165), F6K_MODES, -1, -1, F6K_VFO, F6K_ANTS},
		RIG_FRNG_END,
	  }, /* rx range */
	.tx_range_list2 =  {
		FRQ_RNG_HF(2, F6K_MODES, mW(10), W(100), F6K_VFO, F6K_ANTS),
		FRQ_RNG_6m(2, F6K_MODES, mW(10), W(100), F6K_VFO, F6K_ANTS),
		FRQ_RNG_2m(2, F6K_MODES, mW(10), W(100), F6K_VFO, F6K_ANTS),
		RIG_FRNG_END,
	  }, /* tx range */

	.tuning_steps =  {
		 {F6K_MODES, 1},
		 RIG_TS_END,
		},

	/* mode/filter list, remember: order matters! */
	.filters =  {
		{RIG_MODE_SSB, kHz(2.7)},
		{RIG_MODE_SSB, kHz(3.3)},
		{RIG_MODE_SSB, kHz(1.8)},
		{RIG_MODE_SSB, kHz(1.6)},
		{RIG_MODE_SSB, kHz(4.0)},
		{RIG_MODE_SSB, RIG_FLT_ANY},
		{RIG_MODE_CW, kHz(0.4)},
		{RIG_MODE_CW, kHz(1.5)},
		{RIG_MODE_CW, Hz(50)},
		{RIG_MODE_CW, kHz(3.0)},
		{RIG_MODE_CW, RIG_FLT_ANY},
		{RIG_MODE_PKTUSB|RIG_MODE_PKTLSB, kHz(1.5)},
		{RIG_MODE_PKTUSB|RIG_MODE_PKTLSB, kHz(3.0)},
		{RIG_MODE_PKTUSB|RIG_MODE_PKTLSB, kHz(0.1)},
		{RIG_MODE_PKTUSB|RIG_MODE_PKTLSB, RIG_FLT_ANY},
		{RIG_MODE_AM, kHz(6)},
		{RIG_MODE_AM, kHz(14)},
		{RIG_MODE_AM, kHz(5.6)},
		{RIG_MODE_AM, kHz(20.0)},
		{RIG_MODE_AM, RIG_FLT_ANY},
		{RIG_MODE_FM, kHz(13)}, /* TBC */
		RIG_FLT_END,
	},
	.priv =  (void *)&f6k_priv_caps,

	.rig_init =		kenwood_init,
	.rig_cleanup =		kenwood_cleanup,
	.rig_open =		flexradio_open,
	.set_freq =		kenwood_set_freq,
	.get_freq =		kenwood_get_freq,
	.set_mode =		kenwood_set_mode,
	.get_mode =		kenwood_get_mode,
	.set_vfo =		kenwood_set_vfo,
	.get_vfo =		kenwood_get_vfo_if,
	//.set_split_mode =	kenwood_set_split_mode,
	//.get_split_mode =	kenwood_get_split_mode,
	.set_split_vfo =	kenwood_set_split_vfo,
	.get_split_vfo =	kenwood_get_split_vfo_if,
	.get_ptt =		kenwood_get_ptt,
	.set_ptt =		kenwood_set_ptt,
	 // -spc- stopped
	 //.set_ant =		kenwood_set_ant_no_ack,
	 //.get_ant =		kenwood_get_ant,
};
