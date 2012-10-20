#include "bluebox.h"

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
	uint8_t SerialNumber[] = {0x11, 0x22, 0x33, 0x44};
	uint8_t ControlData[2] = {0, 0};

	switch (USB_ControlRequest.bRequest) {
	case 0x09:
		if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
			Endpoint_ClearSETUP();

			Endpoint_Read_Control_Stream_LE(ControlData, sizeof(ControlData));
			Endpoint_ClearIN();
		}

		break;
	case 0x01:
		if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
			Endpoint_ClearSETUP();

			switch (USB_ControlRequest.wValue) {
				case 0x301:
					Endpoint_Write_Control_Stream_LE(SerialNumber, sizeof(SerialNumber));
					break;
			}

			if (ControlData[1])
				Endpoint_Write_Control_Stream_LE(ControlData, sizeof(ControlData));

			Endpoint_ClearOUT();
		}

		break;
	}
}

uint8_t text[IN_EPSIZE] = "testing";

void BBTask(void)
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(OUT_EPADDR);
	if (Endpoint_IsOUTReceived()) {
		if (Endpoint_IsReadWriteAllowed()) {
			memset(text, 0, sizeof(text));
			Endpoint_Read_Stream_LE(&text, sizeof(text), NULL);
		}
		Endpoint_ClearOUT();
		LEDs_TurnOffLEDs(LEDS_LED1);
	}

	Endpoint_SelectEndpoint(IN_EPADDR);
	if (Endpoint_IsINReady()) {
		Endpoint_Write_Stream_LE(&text, sizeof(text), NULL);
		Endpoint_ClearIN();
		LEDs_TurnOnLEDs(LEDS_LED1);
	}
}

int main(void)
{
	SetupHardware();
	GlobalInterruptEnable();

	LEDs_TurnOnLEDs(LEDS_LED1);

	for (;;) {
		BBTask();
		USB_USBTask();
	}
}
