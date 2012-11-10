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

/* Control requests */
#define REQUEST_REGISTER	0x01
#define REQUEST_FREQUENCY 	0x02
#define	REQUEST_MODINDEX	0x03
#define	REQUEST_CSMA_RSSI	0x04
#define	REQUEST_POWER		0x05
#define	REQUEST_AFC		0x06
#define	REQUEST_IFBW		0x07
#define	REQUEST_TRAINING	0x08
#define	REQUEST_SYNCWORD	0x09
#define	REQUEST_RXTX_MODE	0x0A
#define REQUEST_BITRATE		0x0B
#define REQUEST_DATA		0x0C
#define REQUEST_BOOTLOADER 	0xFF

/* Board xtal frequency */
#define XTAL_FREQ 		16000000

/* Default settings */
#define FREQUENCY		437450000
#define TX_WAIT_TIMEOUT		120U
#define TX_TIMEOUT_DELAY	10U
#define RX_WAIT_TIMEOUT		120U
#define CSMA_TIMEOUT		60U
#define CSMA_RSSI		-70
#define BAUD_RATE		9600
#define MOD_INDEX		2
#define PA_SETTING		8
#define AFC_RANGE		10
#define AFC_KI			11
#define AFC_KP			4
#define AFC_ENABLE		1
#define IF_FILTER_BW		2
#define SYNC_WORD		0x4f5a33
#define SYNC_WORD_TOLERANCE	ADF_SYNC_WORD_ERROR_TOLERANCE_1
#define SYNC_WORD_LENGTH	ADF_SYNC_WORD_LEN_24

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
	uint32_t sw;
	uint8_t swtol;
	uint8_t swlen;
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
