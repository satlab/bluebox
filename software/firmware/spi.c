/*
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

#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "adf7021.h"
#include "bluebox.h"
#include "spi.h"
#include "led.h"

#define DATA_LENGTH	256
#define NUM_BUFS	2

uint8_t data[NUM_BUFS][DATA_LENGTH];
uint16_t bytes, target;
uint8_t front;

void spi_rx_start(void)
{
	swd_disable();
	led_on(LED_ALL);
	bytes = 0;
	target = DATA_LENGTH;
	spi_enable();
	spi_enable_it();
}

void spi_rx_done(void)
{
	spi_disable_it();
	spi_disable();
	swd_enable();
	led_off(LED_ALL);
}

void data_done(void)
{
	Endpoint_SelectEndpoint(IN_EPADDR);
	Endpoint_AbortPendingIN();
	Endpoint_Write_Stream_LE(&data[front], DATA_LENGTH, NULL);
	Endpoint_ClearIN();
	front = !front;
}

static inline uint8_t __attribute__ ((pure)) popcount(uint8_t num)
{
	uint8_t count;

	/* Clear the least significant bit set */
	for (count = 0; num; count++)
		num &= num - 1; 

	return count;
}

static inline uint8_t __attribute__ ((pure)) frame_type(uint8_t fsm)
{
	unsigned int diff_short, diff_long;

	diff_short = popcount(fsm ^ SHORT_FRAME_MARKER);
	diff_long  = popcount(fsm ^ LONG_FRAME_MARKER);

	/* Assume long frame if equal Hamming distance */
	fsm = (diff_short < diff_long) ? SHORT_FRAME_MARKER : LONG_FRAME_MARKER;

	return fsm;
}

static inline uint8_t __attribute__ ((pure)) frame_cuberrs(uint8_t *cub)
{
	int i;
	uint8_t errs = 0;

	for (i = 0; i < CUB_LENGTH; i++)
		errs += popcount(cub[i] ^ conf.callsign[CALLSIGN_LENGTH - CUB_LENGTH + i]);

	return errs;
}

static inline __attribute__ ((pure)) int rx_rs_length(uint8_t type)
{
	return RS_LENGTH;
}

static inline __attribute__ ((pure)) int rx_data_length(uint8_t type)
{
	return CSP_OVERHEAD + ((type == SHORT_FRAME_MARKER) ? SHORT_FRAME_LIMIT : LONG_FRAME_LIMIT);
}

static inline __attribute__ ((pure)) int rx_frame_spi_length(uint8_t type)
{
	int bytes = CSP_OVERHEAD;

	return 128;

	if (type == SHORT_FRAME_MARKER)
		bytes += conf.do_rs ? SHORT_FRAME_LIMIT + RS_LENGTH : SHORT_FRAME_LIMIT;
	else if (type == LONG_FRAME_MARKER)
		bytes += conf.do_rs ? LONG_FRAME_LIMIT + RS_LENGTH : LONG_FRAME_LIMIT;

	if (conf.do_viterbi)
		bytes = (bytes + VITERBI_TAIL) * VITERBI_RATE;

	bytes += CUB_LENGTH + FSM_LENGTH;

	return bytes;
}

static inline __attribute__ ((pure)) uint16_t training_ms_to_bytes(uint32_t ms, uint32_t bitrate)
{
	return (ms * bitrate) / 1000 / BITS_PER_BYTE;
}

ISR(INT6_vect)
{
	spi_rx_start();
}

ISR(SPI_STC_vect)
{
	data[front][bytes++] = spi_read_data();

	if (bytes == FSM_POSITION) {
		uint8_t errs = frame_cuberrs(&data[front][0]);
		if (errs > SYNC_WORD_TOLERANCE * 2) {
			spi_rx_done();
			return;
		}
	}

	/* Set frame length from FSM */
	if (bytes == FSM_POSITION + 1) {
		uint8_t type = frame_type(data[front][bytes-1]);
		target = rx_frame_spi_length(type);
	}

	if (bytes >= target) {
		spi_rx_done();
		data_done();
		adf_set_threshold_free();
		return;
	}
}
