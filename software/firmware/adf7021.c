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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "adf7021.h"
#include "bluebox.h"
#include "ptt.h"
#include "led.h"

static adf_conf_t rx_conf, tx_conf;
static adf_sysconf_t sys_conf;
static uint32_t adf_current_syncword;

extern struct bluebox_config conf;

enum {
	ADF_OFF,
	ADF_ON,
	ADF_TX,
	ADF_RX
} adf_state;

enum {
	ADF_PA_OFF,
	ADF_PA_ON
} adf_pa_state;

void adf_write_reg(adf_reg_t *reg)
{
	signed char i, j;
	unsigned char byte;

	ADF_PORT_SLE &= ~_BV(ADF_SLE);
	ADF_PORT_SCLK &= ~_BV(ADF_SCLK);

	/* Clock data out MSbit first */
	for (i=3; i>=0; i--) {
		byte = reg->byte[i];

		for (j=8; j>0; j--) {
			ADF_PORT_SCLK &= ~_BV(ADF_SCLK);
			if (byte & 0x80)
				ADF_PORT_SDATA |= _BV(ADF_SDATA);
			else 
				ADF_PORT_SDATA &= ~_BV(ADF_SDATA);
			ADF_PORT_SCLK |= _BV(ADF_SCLK);
			byte += byte;
		}
		ADF_PORT_SCLK &= ~_BV(ADF_SCLK);
	}

	/* Strobe the latch */
	ADF_PORT_SLE |= _BV(ADF_SLE);
	ADF_PORT_SLE |= _BV(ADF_SLE);
	ADF_PORT_SDATA &= ~_BV(ADF_SDATA);
	ADF_PORT_SDATA &= ~_BV(ADF_SLE);
}

adf_reg_t adf_read_reg(unsigned int readback_config)
{
	adf_reg_t register_value;
	signed char i, j;

	/* Write readback and ADC control value */
	register_value.whole_reg = ((readback_config & 0x1F) << 4);

	/* Address the readback setup register */
	register_value.whole_reg |= 7; 
	adf_write_reg(&register_value);
	register_value.whole_reg = 0;

	/* Read back value */
	ADF_PORT_SDATA &= ~_BV(ADF_SDATA);
	ADF_PORT_SCLK &= ~_BV(ADF_SCLK);
	ADF_PORT_SLE |= _BV(ADF_SLE);

	/* Clock in first bit and discard (DB16 is not used) */
	ADF_PORT_SCLK |= _BV(ADF_SCLK);
	unsigned char byte = 0;
	ADF_PORT_SCLK &= ~_BV(ADF_SCLK);

	/* Clock in data MSbit first */
	for (i=1; i>=0; i--) {
		for (j=8; j>0; j--) {
			ADF_PORT_SCLK |= _BV(ADF_SCLK);
			byte += byte;
			if (ADF_PORT_IN_SREAD & _BV(ADF_SREAD))
				byte |= 1;
			ADF_PORT_SCLK &= ~_BV(ADF_SCLK);
		}
		register_value.byte[i] = byte;
	}

	ADF_PORT_SCLK |= _BV(ADF_SCLK);
	ADF_PORT_SLE &= ~_BV(ADF_SLE);
	ADF_PORT_SCLK &= ~_BV(ADF_SCLK);

	return register_value;
}

void adf_set_power_on(unsigned long adf_xtal)
{
	/* Store locally the oscillator frequency */
	sys_conf.adf_xtal = adf_xtal;

	/* Ensure the ADF GPIO port is correctly initialised */
	ADF_PORT_DIR_SWD 	&= ~_BV(ADF_SWD);
	ADF_PORT_DIR_SCLK 	|=  _BV(ADF_SCLK);
	ADF_PORT_DIR_SREAD	&= ~_BV(ADF_SREAD);
	ADF_PORT_DIR_SDATA	|=  _BV(ADF_SDATA);
	ADF_PORT_DIR_SLE	|=  _BV(ADF_SLE);
	ADF_PORT_DIR_MUXOUT	&= ~_BV(ADF_MUXOUT);
	ADF_PORT_DIR_CE		|=  _BV(ADF_CE);
	
	ADF_PORT_CE 		|= _BV(ADF_CE);

	/* write R1, Turn on Internal VCO */
	sys_conf.r1.address_bits 	= 1;
	sys_conf.r1.r_counter 		= 1;
	sys_conf.r1.clockout_divide 	= 0;
	sys_conf.r1.xtal_doubler 	= 0;
	sys_conf.r1.xosc_enable 	= 1;
	sys_conf.r1.xtal_bias 		= 3;
	sys_conf.r1.cp_current 		= 3;
	sys_conf.r1.vco_enable 		= 1;
	sys_conf.r1.rf_divide_by_2	= 1;
	sys_conf.r1.vco_bias 		= 15;
	sys_conf.r1.vco_adjust 		= 1;
	sys_conf.r1.vco_inductor 	= 0;
	adf_write_reg(&sys_conf.r1_reg);

	/* write R15, set CLK_MUX to enable SPI */
	sys_conf.r15.address_bits 	= 15;
	sys_conf.r15.rx_test_mode  	= 0;
	sys_conf.r15.tx_test_mode 	= 0;
	sys_conf.r15.sd_test_mode 	= 0;
	sys_conf.r15.cp_test_mode 	= 0;
	sys_conf.r15.clk_mux 		= 7;
	sys_conf.r15.pll_test_mode 	= 0;
	sys_conf.r15.analog_test_mode 	= 0;
	sys_conf.r15.force_ld_high 	= 0;
	sys_conf.r15.reg1_pd 		= 0;
	sys_conf.r15.cal_override 	= 0;
	adf_write_reg(&sys_conf.r15_reg);

	/* write R14, enable test DAC */
	sys_conf.r14.address_bits	= 14;
	sys_conf.r14.test_tdac_en 	= 0;
	sys_conf.r14.test_dac_offset 	= 0;
	sys_conf.r14.test_dac_gain 	= 0;
	sys_conf.r14.pulse_ext 		= 0;
	sys_conf.r14.leak_factor 	= 0;
	sys_conf.r14.ed_peak_resp 	= 0;
	adf_write_reg(&sys_conf.r14_reg);

	adf_state = ADF_ON;
	adf_pa_state = ADF_PA_OFF;
}

void adf_set_power_off()
{
	/* Turn off chip enable */
	ADF_PORT_CE &= ~_BV(ADF_CE);

	adf_state = ADF_OFF;
	adf_pa_state = ADF_PA_OFF;
}

void adf_find_clocks(adf_conf_t *conf)
{
	/* Find desired F.dev. */
	unsigned char tx_freq_dev = (unsigned char) round( ((double)conf->desired.mod_index * 0.5 * conf->desired.data_rate * 65536.0) / (0.5 * sys_conf.adf_xtal));
	double freq_dev = (tx_freq_dev * sys_conf.adf_xtal) / 65536.0;

	/* Find K */
	unsigned int k = round(100000 / (freq_dev));

	/* Run a variable optimisation for Demod clock divider */
	int i_dem;
	int w_residual = INT16_MAX, residual;

	double demod_clk, data_rate_real;
	unsigned int cdr_clk_divide, disc_bw;

	for (i_dem = 1; i_dem < 15; i_dem++) {
		demod_clk = (double) sys_conf.adf_xtal / i_dem;
		disc_bw = round((k * demod_clk) / 400000);
		cdr_clk_divide = round(demod_clk / ((double)conf->desired.data_rate * 32));

		if (disc_bw > 660)
			continue;

		if (cdr_clk_divide > 255)
			continue;

		data_rate_real = (sys_conf.adf_xtal / ((double) i_dem * (double) cdr_clk_divide * 32.0));
		residual = abs((unsigned int) data_rate_real - conf->desired.data_rate);

		/* Search for a new winner */
		if (w_residual > residual) {
			w_residual = residual;
			conf->r3.dem_clk_divide = i_dem;
		}
	}

	/* Demodulator clock */
	demod_clk = (double) sys_conf.adf_xtal / conf->r3.dem_clk_divide;

	/* CDR clock */
	conf->r3.cdr_clk_divide = (unsigned int) round(demod_clk / ((double)conf->desired.data_rate * 32));

	/* Data rate and freq. deviation */
	conf->real.data_rate = (sys_conf.adf_xtal / ((double) conf->r3.dem_clk_divide * (double) conf->r3.cdr_clk_divide * 32.0));
	conf->real.freq_dev = (unsigned char) round( ((double)conf->desired.mod_index * 0.5 * conf->real.data_rate * 65536.0) / (0.5 * sys_conf.adf_xtal));

	/* Discriminator bandwidth */
	conf->r4.disc_bw = round((k * demod_clk) / 400000);

	/* Post demodulation bandwidth */
	conf->r4.post_demod_bw = round(((conf->real.data_rate * 0.75) * 3.141592654 * 2048.0) / demod_clk);

	/* K odd or even */
	if (k & 1) { 
		if (((k + 1) / 2) & 1) {
			conf->r4.rx_invert = 2;
			conf->r4.dot_product = 1;
		} else {
			conf->r4.rx_invert = 0;
			conf->r4.dot_product = 1;
		}
	} else {
		if ((k / 2) & 1) {
			conf->r4.rx_invert = 2;
			conf->r4.dot_product = 0;
		} else {
			conf->r4.rx_invert = 0;
			conf->r4.dot_product = 0;
		}
	}
}

void adf_init_rx_mode(unsigned int data_rate, uint8_t mod_index, unsigned long freq, uint8_t if_bw)
{
	/* Calculate the RX clocks */
	rx_conf.desired.data_rate = data_rate;
	rx_conf.desired.mod_index = mod_index;
	rx_conf.desired.freq = freq;
	adf_find_clocks(&rx_conf);

	/* Setup RX Clocks */
	rx_conf.r3.seq_clk_divide = round(sys_conf.adf_xtal / 100000.0);
	rx_conf.r3.agc_clk_divide = round((rx_conf.r3.seq_clk_divide * sys_conf.adf_xtal) / 10000.0);
	rx_conf.r3.bbos_clk_divide = 2; // 16
	rx_conf.r3.address_bits = 3;

	/* IF filter calibration */
	rx_conf.r5.if_filter_divider = (sys_conf.adf_xtal / 50000);
	rx_conf.r5.if_cal_coarse = 1;
	rx_conf.r5.address_bits = 5;

	/* write R0, turn on PLL */
	double n = ((freq-100000) / (sys_conf.adf_xtal * 0.5));
	unsigned long n_int = floor(n);
	unsigned long n_frac = round((n - floor(n)) * 32768);

	rx_conf.r0.rx_on = 1;
	rx_conf.r0.uart_mode = 1;
	rx_conf.r0.muxout = 2;
	rx_conf.r0.int_n = n_int;
	rx_conf.r0.frac_n = n_frac;
	rx_conf.r0.address_bits = 0;

	/* write R4, turn on demodulation */
	rx_conf.r4.demod_scheme = 1;
	rx_conf.r4.if_bw = if_bw;	// 0 = 12.5, 1 = 18.75, 2 = 25 KHz
	rx_conf.r4.address_bits = 4;
}

void adf_init_tx_mode(unsigned int data_rate, uint8_t mod_index, unsigned long freq)
{
	/* Calculate the RX clocks */
	tx_conf.desired.data_rate = data_rate;
	tx_conf.desired.mod_index = mod_index;
	tx_conf.desired.freq = freq;
	adf_find_clocks(&tx_conf);

	/* Setup default R3 values */
	tx_conf.r3.seq_clk_divide = round(sys_conf.adf_xtal / 100000.0);
	tx_conf.r3.agc_clk_divide = round((tx_conf.r3.seq_clk_divide * sys_conf.adf_xtal) / 10000.0);
	tx_conf.r3.bbos_clk_divide = 2; // 16
	tx_conf.r3.address_bits = 3;

	/* write R0, turn on PLL */
	double n = (freq / (sys_conf.adf_xtal * 0.5));
	unsigned long n_int = floor(n);
	unsigned long n_frac = round((n - floor(n)) * 32768);

	tx_conf.r0.rx_on = 0;
	tx_conf.r0.uart_mode = 1;
	tx_conf.r0.muxout = 2;
	tx_conf.r0.int_n = n_int;
	tx_conf.r0.frac_n = n_frac;
	tx_conf.r0.address_bits = 0;

	/* Set the calcualted frequency deviation */
	tx_conf.r2.tx_frequency_deviation = tx_conf.real.freq_dev;

	/* Set PA and modulation type */
	tx_conf.r2.power_amplifier = conf.pa_setting;    // 0 = OFF, 63 = MAX (WARNING NON LINEAR)
	tx_conf.r2.pa_bias = 3;	            // 0 = 5uA, 1 = 7uA, 2 = 9uA, 3 = 11 uA
	tx_conf.r2.pa_ramp = 7;	            // 0 = OFF, 1 = LOWEST, 7 = HIGHEST
	tx_conf.r2.pa_enable = 1;           // 0 = OFF, 1 = ON
	tx_conf.r2.modulation_scheme = 1;   // 0 = FSK, 1 = GFSK, 5 = RCFSK
	tx_conf.r2.address_bits = 2;

	/* Ensure rewrite of PA register */
	adf_pa_state = ADF_PA_OFF;
}

void adf_afc_on(unsigned char range, unsigned char ki, unsigned char kp)
{
	/* write R10, turn AFC on */
	sys_conf.r10.afc_en = 1;
	sys_conf.r10.afc_scaling_factor = 524; /* (2^24 * 500 / XTAL_FREQ) */
	sys_conf.r10.ki = ki;
	sys_conf.r10.kp = kp;
	sys_conf.r10.afc_range = range;
	sys_conf.r10.address_bits = 10;
	adf_write_reg(&sys_conf.r10_reg);
}

void adf_afc_off(void)
{
	sys_conf.r10.afc_en = 0;
	adf_write_reg(&sys_conf.r10_reg);
}

void adf_set_tx_power(char pasetting)
{
	tx_conf.r2.power_amplifier = pasetting;
	adf_write_reg(&tx_conf.r2_reg);
}

void adf_set_rx_sync_word(unsigned long word, unsigned char len, unsigned char error_tolerance)
{
	adf_reg_t register_value;

	/* write R11, configure sync word detect */
	adf_current_syncword = word;
	register_value.whole_reg = 11;
	register_value.whole_reg |= word << 8;
	register_value.whole_reg |= error_tolerance << 6;
	register_value.whole_reg |= len << 4;
	adf_write_reg(&register_value);

	/* write R12, start sync word detect */
	adf_set_threshold_free();
}

void adf_set_threshold_free(void)
{
	/* write R12, start sync word detect */
	sys_conf.r12.packet_length = 255;
	sys_conf.r12.swd_mode = 1;
	sys_conf.r12.lock_thres_mode = 1;
	sys_conf.r12.address_bits = 12;
	adf_write_reg(&sys_conf.r12_reg);
}

void adf_set_rx_mode(void)
{
	if (adf_state == ADF_TX) {
		if (rx_conf.r3_reg.whole_reg != tx_conf.r3_reg.whole_reg)
			adf_write_reg(&rx_conf.r3_reg);
		adf_write_reg(&rx_conf.r0_reg);
	} else {
		adf_write_reg(&rx_conf.r3_reg);
		adf_write_reg(&rx_conf.r5_reg);
		adf_write_reg(&rx_conf.r0_reg);
		adf_write_reg(&rx_conf.r4_reg);
	}

	led_off(LED_TRANSMIT);
	ptt_low(conf.ptt_delay_low);

	adf_state = ADF_RX;
}

void adf_set_tx_mode(void)
{
	/* Turn on PA the first time we transmit */
	if (adf_pa_state == ADF_PA_OFF) {
		adf_write_reg(&tx_conf.r2_reg);
		adf_pa_state = ADF_PA_ON;
	}

	ptt_high(conf.ptt_delay_high);
	led_on(LED_TRANSMIT);

	if (adf_state == ADF_RX) {
		if (rx_conf.r3_reg.whole_reg != tx_conf.r3_reg.whole_reg)
			adf_write_reg(&tx_conf.r3_reg);
		adf_write_reg(&tx_conf.r0_reg);
	} else {
		adf_write_reg(&tx_conf.r3_reg);
		adf_write_reg(&tx_conf.r0_reg);
	}

	adf_state = ADF_TX;
}

unsigned int adf_readback_version(void)
{
	adf_reg_t readback = adf_read_reg(0x1C);
	return readback.word.lower;
}

signed int adf_readback_rssi(void)
{
	unsigned char gain_correction[] = { 86, 0, 0, 0, 58, 38, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	adf_reg_t readback = adf_read_reg(0x14);

	char rssi = readback.byte[0] & 0x7F;
	int gc = (readback.word.lower & 0x780) >> 7;
	double dbm = ((rssi + gain_correction[gc]) * 0.5) - 130;

	return round(dbm);
}

int adf_readback_afc(void)
{
	adf_reg_t readback = adf_read_reg(0x10);
	return 100000 - round(readback.word.lower * ((uint32_t)XTAL_FREQ >> 18));
}

signed int adf_readback_temp(void)
{
	/* Enable ADC */
	adf_reg_t register_value;
	register_value.whole_reg = 8;
	register_value.whole_reg &= 1 << 8;

	adf_reg_t readback = adf_read_reg(0x16);
	return round(-40 + ((68.4 - (readback.byte[0] & 0x7F)) * 9.32));
}

float adf_readback_voltage(void)
{
	/* Enable ADC */
	adf_reg_t register_value;
	register_value.whole_reg = 8;
	register_value.whole_reg &= 1 << 8;
	adf_write_reg(&register_value);

	adf_reg_t readback = adf_read_reg(0x15);
	return (readback.byte[0] & 0x7F) / 21.1;
}

void adf_test_tx(int mode)
{
	adf_reg_t register_value;
	register_value.whole_reg = 15;
	register_value.whole_reg |= mode << 8;
	adf_write_reg(&register_value);
}

void adf_test_off(void)
{
	adf_reg_t register_value;
	register_value.whole_reg = 15;
	adf_write_reg(&register_value);
	adf_state = ADF_RX;
}

void adf_configure(void)
{
	adf_set_rx_sync_word(conf.sw, conf.swlen, conf.swtol);
	adf_init_rx_mode(conf.bitrate, conf.modindex, conf.rx_freq, conf.if_bw);
	adf_init_tx_mode(conf.bitrate, conf.modindex, conf.tx_freq);
	adf_afc_on(conf.afc_range, conf.afc_ki, conf.afc_kp);
	adf_set_rx_mode();
}

void adf_reset(void)
{
	adf_test_off();
	adf_set_power_off();
	delay_ms(100);
	adf_set_power_on(XTAL_FREQ);
	adf_configure();
}

