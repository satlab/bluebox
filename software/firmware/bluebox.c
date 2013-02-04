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

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include "bluebox.h"
#include "adf7021.h"
#include "bootloader.h"
#include "spi.h"
#include "led.h"
#include "ptt.h"

#define rf_config_single(_type, _name) 						\
	_type _name; 								\
	if (direction == ENDPOINT_DIR_OUT) {					\
		Endpoint_Read_Control_Stream_LE(&_name, sizeof(_name));		\
		conf._name = _name;						\
		adf_configure();						\
	} else if (direction == ENDPOINT_DIR_IN) {				\
		Endpoint_Write_Control_Stream_LE(&conf._name, sizeof(conf._name)); \
	}

struct bluebox_config conf = {
	.freq = FREQUENCY,
	.csma_rssi = CSMA_RSSI,
	.bitrate = BAUD_RATE,
	.modindex = MOD_INDEX,
	.pa_setting = PA_SETTING,
	.afc_range = AFC_RANGE,
	.afc_ki = AFC_KI,
	.afc_kp = AFC_KP,
	.afc_enable = AFC_ENABLE,
	.if_bw = IF_FILTER_BW,
	.sw = SYNC_WORD,
	.swtol = SYNC_WORD_TOLERANCE,	
	.swlen = SYNC_WORD_BITS,
	.do_rs = true,
	.do_viterbi = true,
	.callsign = CALLSIGN,
	.training_ms = TRAINING_MS,
	.training_inter_ms = TRAINING_INTER_MS,
	.training_symbol = TRAINING_SYMBOL,
	.tx = 0,
	.rx = 0,
	.fw_revision = FW_REVISION,
};

uint32_t serialno __attribute__((section(".eeprom")));

static void setup_hardware(void)
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	clock_prescale_set(clock_div_1);

	ptt_init();
	USB_Init();
	led_init();

	/* Initialize SPI */
	spi_init_config(SPI_SLAVE | SPI_MSB_FIRST);
}

static void flash_leds(void)
{
	led_on(LED_ALL);
	delay_ms(75);
	led_off(LED_ALL);
	delay_ms(100);
	led_on(LED_ALL);
	delay_ms(75);
	led_off(LED_ALL);
}

static void callsign_init(char *cs)
{
	int i;

	for (i = 0; i < CALLSIGN_LENGTH; i++) {
		conf.callsign[i] = cs[i];
		if (i < SYNC_WORD_LENGTH)
			conf.sw |= (uint32_t)conf.callsign[i] << 8 * (SYNC_WORD_LENGTH - i - 1);
	}
}

static void do_register(int direction, unsigned int regnum)
{
	uint32_t value;
	adf_reg_t reg;

	if (direction == ENDPOINT_DIR_OUT) {
		Endpoint_Read_Control_Stream_LE(&value, sizeof(value));
		value = (value & ~0xf) | (regnum & 0xf);
		reg.whole_reg = value;
		adf_write_reg(&reg);
	} else if (direction == ENDPOINT_DIR_IN) {
		reg = adf_read_reg(regnum);
		value = reg.whole_reg;
		Endpoint_Write_Control_Stream_LE(&value, sizeof(value));
	}
}

static void do_rxtx_mode(int direction, unsigned int wValue)
{
	if (wValue != 0)
		adf_set_tx_mode();
	else
		adf_set_rx_mode();
}

static void do_frequency(int direction, unsigned int vWalue)
{
	rf_config_single(uint32_t, freq);
}

static void do_modindex(int direction, unsigned int vWalue)
{
	rf_config_single(uint8_t, modindex);
}

static void do_csma_rssi(int direction, unsigned int vWalue)
{
	rf_config_single(int16_t, csma_rssi);
}

static void do_power(int direction, unsigned int vWalue)
{
	rf_config_single(uint8_t, pa_setting);
}

struct afc_request {
	uint8_t state;
	uint8_t range;
	uint8_t p;
	uint8_t i;
} __attribute__ ((packed));
	
static void do_acf(int direction, unsigned int vWalue)
{
	struct afc_request req;

	if (direction == ENDPOINT_DIR_OUT) {
		Endpoint_Read_Control_Stream_LE(&req, sizeof(req));
		conf.afc_enable = !!req.state;
		conf.afc_range = req.range ? req.range : conf.afc_range;
		conf.afc_kp = req.p ? req.p : conf.afc_kp;
		conf.afc_ki = req.i ? req.i : conf.afc_ki;
		adf_configure();
	} else if (direction == ENDPOINT_DIR_IN) {
		req.state = conf.afc_enable;
		req.range = conf.afc_range;
		req.p = conf.afc_kp;
		req.i = conf.afc_ki;
		Endpoint_Write_Control_Stream_LE(&req, sizeof(req));
	}
}

static void do_ifbw(int direction, unsigned int vWalue)
{
	rf_config_single(uint8_t, if_bw);
}

static void do_training(int direction, unsigned int vWalue)
{
	rf_config_single(uint16_t, training_ms);
}

static void do_syncword(int direction, unsigned int vWalue)
{
	rf_config_single(uint32_t, sw);
}

static void do_bitrate(int direction, unsigned int vWalue)
{
	rf_config_single(uint16_t, bitrate);
}

static void do_tx(int direction, unsigned int vWalue)
{
	rf_config_single(uint32_t, tx);
}

static void do_rx(int direction, unsigned int vWalue)
{
	rf_config_single(uint32_t, rx);
}

static void do_fw_revision(int direction, unsigned int vWalue)
{
	char fwrev[9];

	if (direction == ENDPOINT_DIR_IN) {
		memset(fwrev, ' ', 9);
		snprintf(fwrev, 9, "%s", conf.fw_revision);
		fwrev[8] = '\0';
		Endpoint_Write_Control_Stream_LE(fwrev, 8);
	}
}

static void do_serialnumber(int direction, unsigned int vWalue)
{
	uint32_t snum;

	if (direction == ENDPOINT_DIR_OUT) {
		Endpoint_Read_Control_Stream_LE(&snum, sizeof(snum));
		eeprom_write_dword(&serialno, snum);
	} else if (direction == ENDPOINT_DIR_IN) {
		snum = eeprom_read_dword(&serialno);
		Endpoint_Write_Control_Stream_LE(&snum, sizeof(snum));
	}
}

static void do_control_request(int direction)
{
	switch (USB_ControlRequest.bRequest) {
	case REQUEST_REGISTER:
		do_register(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_RXTX_MODE:
		do_rxtx_mode(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_FREQUENCY:
		do_frequency(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_MODINDEX:
		do_modindex(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_CSMA_RSSI:
		do_csma_rssi(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_POWER:
		do_power(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_AFC:
		do_acf(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_IFBW:
		do_ifbw(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_TRAINING:	
		do_training(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_SYNCWORD:	
		do_syncword(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_BITRATE:
		do_bitrate(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_TX:
		do_tx(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_RX:
		do_rx(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_SERIALNUMBER:
		do_serialnumber(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_FWREVISION:
		do_fw_revision(direction, USB_ControlRequest.wValue);
		break;
	case REQUEST_RESET:
		reboot();
		break;
	case REQUEST_DFU:
		jump_to_bootloader();
		break;
	}
}

static bool csma_tx_allowed(void)
{
	int rssi;
	static unsigned int quarantine = 0;

	if (quarantine > 0) {
		quarantine--;
		delay_ms(1);
	}

	rssi = adf_readback_rssi();

	if (rssi > conf.csma_rssi)
		quarantine = 100;

	return (rssi <= conf.csma_rssi);
}

static void bluebox_task(void)
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(OUT_EPADDR);
	if (Endpoint_IsOUTReceived() && csma_tx_allowed() && spi_tx_prepare()) {
		if (Endpoint_IsReadWriteAllowed()) {
			if (!(data[front].flags & FLAG_TX_READY)) {
				Endpoint_Read_Stream_LE(&data[front], sizeof(data[front]), NULL);
				data[front].flags |= FLAG_TX_READY;
				adf_set_tx_mode();
				spi_tx_start();
			} else {
				Endpoint_Read_Stream_LE(&data[back], sizeof(data[back]), NULL);
				data[back].flags |= FLAG_TX_READY;
			}
			Endpoint_ClearOUT();
		}
	}
}

void EVENT_USB_Device_ControlRequest(void)
{
	switch (USB_ControlRequest.bmRequestType) {
	case (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE):
		Endpoint_ClearSETUP();
		do_control_request(ENDPOINT_DIR_OUT);
		Endpoint_ClearIN();
		break;
	case (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE):
		Endpoint_ClearSETUP();
		do_control_request(ENDPOINT_DIR_IN);
		Endpoint_ClearOUT();
		break;
	default:
		break;
	}
}

int main(void)
{
	setup_hardware();
	GlobalInterruptEnable();

	callsign_init(conf.callsign);

	flash_leds();

	adf_set_power_on(XTAL_FREQ);
	adf_configure();
	adf_set_rx_mode();

	swd_init();
	swd_enable();

	for (;;) {
		spi_rx_task();
		bluebox_task();
		USB_USBTask();
	}
}
