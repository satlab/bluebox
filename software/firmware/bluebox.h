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

#ifndef _BLUEBOX_H_
#define _BLUEBOX_H_

#include <stdlib.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Descriptors.h"

#include <LUFA/Drivers/USB/USB.h>

#define REQUEST_LEDCTL		0x01
#define REQUEST_REGISTER	0x0B
#define REQUEST_RXTX_MODE	0x0C
#define REQUEST_BOOTLOADER 	0xFF	

struct bluebox_config {
	uint32_t freq;
	int16_t csma_rssi;
	uint16_t speed;
	uint8_t modindex;
	uint8_t pa_setting;
	uint8_t afc_range;
	uint8_t afc_ki;
	uint8_t afc_kp;
	uint8_t afc_enable;
	uint8_t if_bw;
	uint8_t sync_word_tolerance;
};

void SetupHardware(void);
void EVENT_USB_Device_ControlRequest(void);

static inline void delay_ms(unsigned int ms)
{
	while (ms--)
		_delay_ms(1);
}

static inline void reboot(void)
{
	wdt_enable(WDTO_15MS);
	while (1);
}

#endif /* _BLUEBOX_H_ */
