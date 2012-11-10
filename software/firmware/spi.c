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

#define DATA_LENGTH	150
#define NUM_BUFS	2

uint8_t data[NUM_BUFS][DATA_LENGTH];
uint8_t d, curbuf;

void spi_rx_start(void)
{
	d = 0;
	spi_enable();
	spi_enable_it();
}

void spi_rx_done(void)
{
	spi_disable_it();
	spi_disable();
}

void data_done(void)
{
	Endpoint_SelectEndpoint(IN_EPADDR);
	Endpoint_AbortPendingIN();
	Endpoint_Write_Stream_LE(&data[curbuf], d, NULL);
	Endpoint_ClearIN();
	curbuf = !curbuf;
	PORTF |= _BV(4) | _BV(1);
	swd_enable();
}

/* Syncword detect interrupt */
ISR(INT6_vect)
{
	PORTF &= ~(_BV(4) | _BV(1));
	swd_disable();
	spi_rx_start();
}

ISR(SPI_STC_vect)
{
	data[curbuf][d++] = spi_read_data();
	if (d >= DATA_LENGTH) {
		spi_rx_done();
		data_done();
		adf_set_threshold_free();
	}
}
