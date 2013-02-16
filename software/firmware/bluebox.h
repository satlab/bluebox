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

/* Git revision */
#ifndef FW_REVISION
#define FW_REVISION "unknown"
#endif

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
#define REQUEST_TX		0x0C
#define REQUEST_RX		0x0D
#define REQUEST_TX_FREQUENCY	0x0E
#define REQUEST_RX_FREQUENCY	0x0F
#define REQUEST_SERIALNUMBER	0xFC
#define REQUEST_FWREVISION	0xFD
#define REQUEST_RESET		0xFE
#define REQUEST_DFU		0xFF

/* Board xtal frequency */
#define XTAL_FREQ 		16000000

/* Default settings */
#define FREQUENCY		437450000
#define TX_WAIT_TIMEOUT		120U
#define TX_TIMEOUT_DELAY	10U
#define RX_WAIT_TIMEOUT		120U
#define CSMA_RSSI		-50
#define BAUD_RATE		2400
#define MOD_INDEX		8
#define PA_SETTING		8
#define AFC_RANGE		10
#define AFC_KI			11
#define AFC_KP			4
#define AFC_ENABLE		1
#define IF_FILTER_BW		2
#define SYNC_WORD		0x4f5a33
#define SYNC_WORD_TOLERANCE	ADF_SYNC_WORD_ERROR_TOLERANCE_1
#define SYNC_WORD_BITS		ADF_SYNC_WORD_LEN_24
#define TRAINING_SYMBOL		0x55
#define TRAINING_MS		200
#define TRAINING_INTER_MS	200
#define PTT_DELAY_HIGH		100
#define PTT_DELAY_LOW		100

/* AAUSAT3 packet format */
#define CALLSIGN		"OZ3CUB"
#define CALLSIGN_LENGTH		6
#define SYNC_WORD_LENGTH	3
#define CUB_LENGTH		(CALLSIGN_LENGTH - SYNC_WORD_LENGTH)
#define FSM_POSITION		(CALLSIGN_LENGTH - SYNC_WORD_LENGTH)
#define SHORT_FRAME_MARKER	0xA6
#define LONG_FRAME_MARKER	0x59
#define FSM_LENGTH		1
#define SHORT_FRAME_LIMIT	25
#define LONG_FRAME_LIMIT	86
#define RS_LENGTH		32
#define VITERBI_RATE		2
#define VITERBI_TAIL		1
#define CSP_OVERHEAD		6
#define BITS_PER_BYTE		8

/* Buffer configuration */
#define TOTAL_LENGTH		512
#define DATA_LENGTH		(TOTAL_LENGTH - sizeof(uint16_t) * 5 - sizeof(uint8_t) * 1)
#define NUM_BUFS		2

/* This must be 512 bytes */
struct data_buffer {
	uint16_t size;
	uint16_t progress;
	int16_t rssi;
	int16_t freq;
	uint8_t flags;
	uint16_t training;
	uint8_t data[DATA_LENGTH];
};

/* Data buffer flags */
#define FLAG_RX_READY		0x01
#define FLAG_TX_READY		0x02

/* Config flags */
#define CONF_FLAG_NONE		0x00
#define CONF_FLAG_RECONFIGURE	0x01

struct bluebox_config {
	uint8_t flags;
	uint32_t tx_freq;
	uint32_t rx_freq;
	int16_t csma_rssi;
	uint16_t bitrate;
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
	uint8_t do_rs;
	uint8_t do_viterbi;
	uint8_t training_symbol;
	uint16_t training_ms;
	uint16_t training_inter_ms;
	char callsign[CALLSIGN_LENGTH];
	uint32_t tx;
	uint32_t rx;
	uint16_t ptt_delay_high;
	uint16_t ptt_delay_low;
	char *fw_revision;
};

extern struct bluebox_config conf;
extern uint32_t serialno __attribute__((section(".eeprom")));

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

static inline __attribute__ ((pure)) uint16_t training_ms_to_bytes(uint32_t ms, uint32_t bitrate)
{
	return (ms * bitrate) / 1000 / BITS_PER_BYTE;
}

#endif /* _BLUEBOX_H_ */
