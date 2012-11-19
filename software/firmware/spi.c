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

struct data_buffer data[NUM_BUFS];
uint8_t front = 0;
uint8_t back  = 1;

static volatile unsigned char spi_mode = SPI_MODE_IDLE;
static char preamble[CALLSIGN_LENGTH + FSM_LENGTH];

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

static inline __attribute__ ((pure)) int rx_frame_spi_length(uint8_t type)
{
	int bytes = CSP_OVERHEAD;

	if (type == SHORT_FRAME_MARKER)
		bytes += conf.do_rs ? SHORT_FRAME_LIMIT + RS_LENGTH : SHORT_FRAME_LIMIT;
	else if (type == LONG_FRAME_MARKER)
		bytes += conf.do_rs ? LONG_FRAME_LIMIT + RS_LENGTH : LONG_FRAME_LIMIT;

	if (conf.do_viterbi)
		bytes = (bytes + VITERBI_TAIL) * VITERBI_RATE;

	return bytes;
}

static inline __attribute__ ((pure)) int tx_frame_fsm(unsigned int data)
{
	int short_frame_limit = SHORT_FRAME_LIMIT + CSP_OVERHEAD;

	if (conf.do_rs)
		short_frame_limit += RS_LENGTH;

	if (conf.do_viterbi)
		short_frame_limit = (short_frame_limit + VITERBI_TAIL) * VITERBI_RATE;

	return (data <= short_frame_limit) ? SHORT_FRAME_MARKER : LONG_FRAME_MARKER;
}

void spi_rx_start(void)
{
	spi_mode = SPI_MODE_RX;
	swd_disable();

	led_on(LED_RECEIVE);

	data[front].progress = 0;
	data[front].size = DATA_LENGTH;

	spi_enable();
	spi_enable_it();
}

void spi_rx_done(void)
{
	spi_disable_it();
	spi_disable();
	swd_enable();

	led_off(LED_RECEIVE);
	spi_mode = SPI_MODE_IDLE;
}

void spi_tx_start(void)
{
	spi_mode = SPI_MODE_TX;
	swd_disable();

	strncpy(preamble, conf.callsign, CALLSIGN_LENGTH);
	preamble[CALLSIGN_LENGTH] = tx_frame_fsm(data[front].size);

	data[front].progress = 0;
	data[front].training = conf.training_bytes;
	
	spi_enable();
	spi_enable_it();
}

void spi_tx_done(void)
{
	spi_disable_it();
	spi_disable();
	swd_enable();

	spi_mode = SPI_MODE_IDLE;
}

bool spi_tx_allowed(void)
{
	bool allow = false;

	swd_disable();

	if (spi_mode == SPI_MODE_RX)
		goto out;

	spi_mode = SPI_MODE_TX;
	allow = true;

out:
	return allow;
}

void spi_rx_task(void)
{
	if (data[back].flags & FLAG_RX_READY) {
		Endpoint_SelectEndpoint(IN_EPADDR);
		Endpoint_AbortPendingIN();
		Endpoint_Write_Stream_LE(&data[back], sizeof(data[back]), NULL);
		Endpoint_ClearIN();
		data[back].flags &= ~FLAG_RX_READY;
	}
}

void flip_buffers(void)
{
	data[front].flags |= FLAG_RX_READY;
	back = front;
	front = !front;
}

ISR(INT6_vect)
{
	spi_rx_start();
}

ISR(SPI_STC_vect)
{
	static uint8_t errs, type, byte;

	if (spi_mode == SPI_MODE_TX) {
		if (data[front].training > 0) {
			spi_write_data(conf.training_symbol);
			data[front].training--;
		} else {
			if (data[front].progress < (CALLSIGN_LENGTH + FSM_LENGTH))
				byte = preamble[data[front].progress];
			else
				byte = data[front].data[data[front].progress - (CALLSIGN_LENGTH + FSM_LENGTH)];
			data[front].progress++;
			spi_write_data(byte);
		}

		if (data[front].progress > (data[front].size + CALLSIGN_LENGTH + FSM_LENGTH)) {
			spi_tx_done();
			adf_set_rx_mode();
		}
	} else {
		data[front].data[data[front].progress++] = spi_read_data();

		if (data[front].size == DATA_LENGTH) {
			if (data[front].progress == FSM_POSITION) {
				errs = frame_cuberrs(data[front].data);
				if (errs > SYNC_WORD_TOLERANCE * 2) {
					spi_rx_done();
					adf_set_threshold_free();
					return;
				}
			}

			if (data[front].progress == FSM_POSITION + 1) {
				type = frame_type(data[front].data[FSM_POSITION]);
				data[front].size = rx_frame_spi_length(type);
				data[front].progress = 0;
			}
		}

		if (data[front].progress >= data[front].size) {
			spi_rx_done();
			flip_buffers();
			adf_set_threshold_free();
			return;
		}
	}
}
