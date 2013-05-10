/*
 * Copyright (c) 2008 Johan Christiansen
 * Copyright (c) 2012 Jeppe Ledet-Pedersen <jlp@satlab.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _ADF7021_H_
#define _ADF7021_H_

#include <stdint.h>
#include <stdbool.h>

#if defined(BBSTANDARD)
#define ADF_PORT_SWD		PORTE
#define ADF_PORT_IN_SWD		PINE
#define ADF_PORT_DIR_SWD	DDRE
#define ADF_SWD 		6

#define ADF_PORT_SCLK		PORTC
#define ADF_PORT_IN_SCLK	PINC
#define ADF_PORT_DIR_SCLK	DDRC
#define ADF_SCLK 		7

#define ADF_PORT_SREAD		PORTC
#define ADF_PORT_IN_SREAD	PINC
#define ADF_PORT_DIR_SREAD	DDRC
#define ADF_SREAD 		6

#define ADF_PORT_SDATA		PORTB
#define ADF_PORT_IN_SDATA	PINB
#define ADF_PORT_DIR_SDATA	DDRB
#define ADF_SDATA 		6

#define ADF_PORT_SLE		PORTB
#define ADF_PORT_IN_SLE		PINB
#define ADF_PORT_DIR_SLE	DDRB
#define ADF_SLE 		5

#define ADF_PORT_MUXOUT		PORTD
#define ADF_PORT_IN_MUXOUT	PIND
#define ADF_PORT_DIR_MUXOUT	DDRD
#define ADF_MUXOUT 		6

#define ADF_PORT_CE		PORTD
#define ADF_PORT_IN_CE		PIND
#define ADF_PORT_DIR_CE		DDRD
#define ADF_CE 			7
#elif defined(BBMICRO)
#define ADF_PORT_SWD		PORTB
#define ADF_PORT_IN_SWD		PINB
#define ADF_PORT_DIR_SWD	DDRB
#define ADF_SWD 		5

#define ADF_PORT_SCLK		PORTC
#define ADF_PORT_IN_SCLK	PINC
#define ADF_PORT_DIR_SCLK	DDRC
#define ADF_SCLK 		2

#define ADF_PORT_SREAD		PORTC
#define ADF_PORT_IN_SREAD	PINC
#define ADF_PORT_DIR_SREAD	DDRC
#define ADF_SREAD 		4

#define ADF_PORT_SDATA		PORTC
#define ADF_PORT_IN_SDATA	PINC
#define ADF_PORT_DIR_SDATA	DDRC
#define ADF_SDATA 		5

#define ADF_PORT_SLE		PORTC
#define ADF_PORT_IN_SLE		PINC
#define ADF_PORT_DIR_SLE	DDRC
#define ADF_SLE 		6

#define ADF_PORT_MUXOUT		PORTD
#define ADF_PORT_IN_MUXOUT	PIND
#define ADF_PORT_DIR_MUXOUT	DDRD
#define ADF_MUXOUT 		1

#define ADF_PORT_CE		PORTD
#define ADF_PORT_IN_CE		PIND
#define ADF_PORT_DIR_CE		DDRD
#define ADF_CE 			5
#endif

typedef union
{
	unsigned long whole_reg;
	struct {
		unsigned int lower;
		unsigned int upper;
	} word;
	unsigned char byte[4];
} adf_reg_t;

typedef struct {
	struct {
		double data_rate;
		double mod_index;
		unsigned long freq;
	} desired;
	struct {
		double data_rate;
		double mod_index;
		unsigned long freq;
		unsigned int freq_dev;
	} real;
	union {
		adf_reg_t r0_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned int  frac_n : 15;
			unsigned int  int_n : 8;
			unsigned char rx_on : 1;
			unsigned char uart_mode : 1;
			unsigned char muxout : 3;
		} r0;
	};
	union {
		adf_reg_t r2_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char modulation_scheme : 3;
			unsigned char pa_enable : 1;
			unsigned char pa_ramp : 3;
			unsigned char pa_bias : 2;
			unsigned char power_amplifier : 6;
			unsigned int  tx_frequency_deviation : 9;
			unsigned char tx_data_invert : 2;
			unsigned char rcosine_alpha : 1;
		} r2;
	};
	union {
		adf_reg_t r3_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char bbos_clk_divide: 2;
			unsigned char dem_clk_divide : 4;
			unsigned char cdr_clk_divide : 8;
			unsigned char seq_clk_divide : 8;
			unsigned char agc_clk_divide : 6;
		} r3;
	};
	union {
		adf_reg_t r4_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char demod_scheme : 3;
			unsigned char dot_product : 1;
			unsigned char rx_invert : 2;
			unsigned int  disc_bw : 10;
			unsigned int  post_demod_bw : 10;
			unsigned char if_bw : 2;
		} r4;
	};
	union {
		adf_reg_t r5_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char if_cal_coarse : 1;
			unsigned int  if_filter_divider : 9;
			unsigned char if_filter_adjust : 6;
			unsigned char ir_phase_adjust_mag : 4;
			unsigned char ir_phase_adjust_direction : 1;
			unsigned char ir_gain_adjust_mac : 5;
			unsigned char ir_gain_adjust_iq : 1;
			unsigned char ir_gain_adjust_updn : 1;
		} r5;
	};
	union {
		adf_reg_t r6_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char if_fine_cal : 1;
			unsigned char if_cal_lower_tone : 8;
			unsigned char if_cal_upper_tone : 8;
			unsigned char if_cal_dwell_time : 7;
			unsigned char ir_cal_source_drive : 2;
			unsigned char ir_cal_source_div_2 : 1;
		} r6;
	};
} adf_conf_t;

typedef struct {
	unsigned long adf_xtal;
	union {
		adf_reg_t r1_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char r_counter : 3;
			unsigned char clockout_divide : 4;
			unsigned char xtal_doubler : 1;
			unsigned char xosc_enable : 1;
			unsigned char xtal_bias : 2;
			unsigned char cp_current : 2;
			unsigned char vco_enable : 1;
			unsigned char rf_divide_by_2 : 1;
			unsigned char vco_bias : 4;
			unsigned char vco_adjust : 2;
			unsigned char vco_inductor : 1;
		} r1;
	};
	union {
		adf_reg_t r10_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char afc_en : 1;
			unsigned int  afc_scaling_factor : 12;
			unsigned char ki : 4;
			unsigned char kp : 3;
			unsigned char afc_range : 8;
		} r10;
	};
	union {
		adf_reg_t r12_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char lock_thres_mode : 2;
			unsigned char swd_mode : 2;
			unsigned char packet_length : 8;
		} r12;
	};
	union {
		adf_reg_t r14_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char test_tdac_en : 1;
			unsigned int test_dac_offset : 16;
			unsigned char test_dac_gain : 4;
			unsigned char pulse_ext : 2;
			unsigned char leak_factor : 3;
			unsigned char ed_peak_resp : 2;
		} r14;
	};
	union {
		adf_reg_t r15_reg;
		struct {
			unsigned char address_bits : 4;
			unsigned char rx_test_mode : 4;
			unsigned char tx_test_mode : 3;
			unsigned char sd_test_mode : 3;
			unsigned char cp_test_mode : 3;
			unsigned char clk_mux : 3;
			unsigned char pll_test_mode : 4;
			unsigned char analog_test_mode : 4;
			unsigned char force_ld_high : 1;
			unsigned char reg1_pd : 1;
			unsigned char cal_override : 2;
		} r15;
	};
} adf_sysconf_t;

void adf_write_reg(adf_reg_t *reg);
adf_reg_t adf_read_reg(unsigned int readback_config);

void adf_set_power_on(unsigned long adf_xtal);
void adf_set_power_off(void);

void adf_init_rx_mode(unsigned int data_rate, uint8_t mod_index, unsigned long freq, uint8_t if_bw);
void adf_init_tx_mode(unsigned int data_rate, uint8_t mod_index, unsigned long freq);
void adf_set_rx_mode(void);
void adf_set_tx_mode(void);

void adf_afc_on(unsigned char range, unsigned char ki, unsigned char kp);
void adf_afc_off(void);
void adf_set_tx_power(char pasetting);

#define ADF_SYNC_WORD_LEN_12	        0
#define ADF_SYNC_WORD_LEN_16	        1
#define ADF_SYNC_WORD_LEN_20	        2
#define ADF_SYNC_WORD_LEN_24	        3

#define ADF_SYNC_WORD_ERROR_TOLERANCE_0	0
#define ADF_SYNC_WORD_ERROR_TOLERANCE_1	1
#define ADF_SYNC_WORD_ERROR_TOLERANCE_2	2
#define ADF_SYNC_WORD_ERROR_TOLERANCE_3	3

void adf_set_rx_sync_word(unsigned long word, unsigned char len, unsigned char error_tolerance);
void adf_set_threshold_free(void);

#define ADF_TEST_TX_MODE_CARRIER_ONLY	1
#define ADF_TEST_TX_MODE_TONE_HIGH	2
#define ADF_TEST_TX_MODE_TONE_LOW	3
#define ADF_TEST_TX_MODE_PATTERN_1010	4
#define ADF_TEST_TX_MODE_PATTERN_PN9	5
#define ADF_TEST_TX_MODE_SYNC_BYTE	6

void adf_test_tx(int mode);
void adf_test_off(void);

unsigned int adf_readback_version(void);
signed int adf_readback_rssi(void);
signed int adf_readback_afc(void);
signed int adf_readback_temp(void);
float adf_readback_voltage(void);
void adf_configure(void);
void adf_reset(void);

#endif /* _ADF7021_H_ */
