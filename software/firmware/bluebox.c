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

#include "bluebox.h"

static uint8_t led_status = 0;

void SetupHardware(void)
{
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	clock_prescale_set(clock_div_1);

	LEDs_Init();
	USB_Init();
}

void EVENT_USB_Device_ControlRequest(void)
{
	switch (USB_ControlRequest.bRequest) {
	case REQUEST_LEDCTL:
		switch (USB_ControlRequest.bmRequestType) {
		case (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE):
			Endpoint_ClearSETUP();

			Endpoint_Read_Control_Stream_LE(&led_status, sizeof(led_status));

			if (led_status)
				LEDs_TurnOnLEDs(LEDS_LED1);
			else
				LEDs_TurnOffLEDs(LEDS_LED1);

			Endpoint_ClearIN();

			break;
		case (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE):
			Endpoint_ClearSETUP();

			Endpoint_Write_Control_Stream_LE(&led_status, sizeof(led_status));
			Endpoint_ClearOUT();

			break;
		default:
			break;
		}
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
	SetupHardware();
	GlobalInterruptEnable();

	LEDs_TurnOnLEDs(LEDS_LED1);

	for (;;) {
		bluebox_task();
		USB_USBTask();
	}
}
