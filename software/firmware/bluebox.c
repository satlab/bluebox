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

static uint8_t led_status = 0;

struct bluebox_config conf;

ISR(TIMER1_COMPA_vect)
{
	/* Toggle LEDs */
}

void setup_hardware(void)
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	clock_prescale_set(clock_div_1);

	USB_Init();
}

int timer_init(unsigned int period_ms)
{
	uint32_t ocr1a;

	if (period_ms > 1000)
		return -ERANGE;

	ocr1a = ((uint32_t) period_ms * (F_CPU/256UL)) / 1000UL;

	/* Set CTC mode and /256 prescaler */
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS12);
	TCCR1C = 0;
	TCNT1 = 0;

	/* Set overflow every second */
	OCR1AH = ((ocr1a >> 8) & 0xff);
	OCR1AL = ((ocr1a >> 0) & 0xff);

	/* Enable output compare A interrupt */
	TIMSK1 = _BV(OCIE1A);

	return 0;
}

static inline void do_ledctl(int direction)
{
	if (direction == ENDPOINT_DIR_OUT) {
		Endpoint_Read_Control_Stream_LE(&led_status, sizeof(led_status));
		timer_init(led_status ? 100 : 1000);
	} else if (direction == ENDPOINT_DIR_IN) {
		Endpoint_Write_Control_Stream_LE(&led_status, sizeof(led_status));
	}
}

static inline void do_rf_register(int direction)
{
	uint32_t value;
	adf_reg_t reg;

	if (direction == ENDPOINT_DIR_OUT) {
		Endpoint_Read_Control_Stream_LE(&value, sizeof(value));
		reg.whole_reg = value;
		adf_write_reg(&reg);
	} else if (direction == ENDPOINT_DIR_IN) {
		reg = adf_read_reg(&reg);
		value = reg.whole_reg;
		Endpoint_Write_Control_Stream_LE(&value, sizeof(value));
	}
}

void EVENT_USB_Device_ControlRequest(void)
{
	switch (USB_ControlRequest.bmRequestType) {
	case (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE):

		Endpoint_ClearSETUP();

		switch (USB_ControlRequest.bRequest) {
		case REQUEST_LEDCTL:
			do_ledctl(ENDPOINT_DIR_OUT);
			break;
		}

		Endpoint_ClearIN();

		break;
	case (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE):

		Endpoint_ClearSETUP();

		switch (USB_ControlRequest.bRequest) {
		case REQUEST_LEDCTL:
			do_ledctl(ENDPOINT_DIR_IN);
			break;
		}

		Endpoint_ClearOUT();

		break;
	default:
		break;
	}
}

void bluebox_task(void)
{
	static uint8_t text[IN_EPSIZE] = "testing";

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(OUT_EPADDR);
	if (Endpoint_IsOUTReceived()) {
		if (Endpoint_IsReadWriteAllowed()) {
			memset(text, 0, sizeof(text));
			Endpoint_Read_Stream_LE(&text, sizeof(text), NULL);
		}
		Endpoint_ClearOUT();

		/* Clear pending data in IN endpoint */
		Endpoint_SelectEndpoint(IN_EPADDR);
		Endpoint_AbortPendingIN();
	}

	if (Endpoint_IsINReady()) {
		Endpoint_Write_Stream_LE(&text, sizeof(text), NULL);
		Endpoint_ClearIN();
	}
}

int main(void)
{
	setup_hardware();
	timer_init(1000);
	GlobalInterruptEnable();

	for (;;) {
		bluebox_task();
		USB_USBTask();
	}
}
