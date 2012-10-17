#include "bluebox.h"

char stupid_gcc __attribute__ ((unused)) = 42;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	if (stupid_gcc != 42)
		printf("GCC is stupid\n");

	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
		USB_USBTask();
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	uint8_t SerialNumber[] = {0x11, 0x22, 0x33, 0x44};
	uint8_t ControlData[2] = { 0, 0 };

	switch (USB_ControlRequest.bRequest)
	{
		case 0x09:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				LEDs_ToggleLEDs(LEDS_LED1);

				Endpoint_ClearSETUP();

				Endpoint_Read_Control_Stream_LE(ControlData, sizeof(ControlData));
				Endpoint_ClearIN();
			}

			break;
		case 0x01:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				LEDs_ToggleLEDs(LEDS_LED1);

				Endpoint_ClearSETUP();

				switch (USB_ControlRequest.wValue)
				{
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

