/*
             LUFA Library
     Copyright (C) Dean Camera, 2012.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "Descriptors.h"
#include "bluebox.h"

const USB_Descriptor_Device_t PROGMEM BlueBox_DeviceDescriptor = {
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(01.10),
	.Class                  = USB_CSCP_VendorSpecificClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	/* Note: these have not been allocated from OpenMoko! */
	.VendorID               = 0x1d50,
	.ProductID              = 0x6666,

	.ReleaseNumber          = VERSION_BCD(01.00),

	.ManufacturerStrIndex   = 0x01,
	.ProductStrIndex        = 0x02,
	.SerialNumStrIndex      = 0x03,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_Configuration_t PROGMEM BlueBox_ConfigurationDescriptor =
{
	.Config = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

		.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
		.TotalInterfaces        = 1,
		.ConfigurationNumber    = 1,
		.ConfigurationStrIndex  = NO_DESCRIPTOR,
		.ConfigAttributes       = USB_CONFIG_ATTR_RESERVED,
		.MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
	},

	.Interface = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

		.InterfaceNumber        = 0,
		.AlternateSetting       = 0,
		.TotalEndpoints         = 2,
		.Class                  = USB_CSCP_VendorSpecificClass,
		.SubClass               = 0x00,
		.Protocol               = 0x00,
		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.DataInEndpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = IN_EPADDR,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = IN_EPSIZE,
		.PollingIntervalMS      = 0x00,
	},

	.DataOutEndpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = OUT_EPADDR,
		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = OUT_EPSIZE,
		.PollingIntervalMS      = 0x00,
	},
};

const USB_Descriptor_String_t PROGMEM BlueBox_LanguageString = {
	.Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},
	.UnicodeString          = {LANGUAGE_ID_ENG}
};

const USB_Descriptor_String_t PROGMEM BlueBox_ManufacturerString = {
	.Header                 = {.Size = USB_STRING_LEN(7), .Type = DTYPE_String},
	.UnicodeString          = L"AAUSAT3"
};

const USB_Descriptor_String_t PROGMEM BlueBox_ProductString = {
	.Header                 = {.Size = USB_STRING_LEN(7), .Type = DTYPE_String},
	.UnicodeString          = L"BlueBox"
};

const USB_Descriptor_String_t PROGMEM BlueBox_SerialString = {
	.Header                 = {.Size = USB_STRING_LEN(5), .Type = DTYPE_String},
	.UnicodeString          = L"00001"
};

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
				    const uint8_t wIndex,
				    const void** const DescriptorAddress)
{
	const uint8_t DescriptorType   = (wValue >> 8);
	const uint8_t DescriptorNumber = (wValue & 0xFF);

	const void *Address = NULL;
	uint16_t Size = NO_DESCRIPTOR;

	switch (DescriptorType) {
	case DTYPE_Device:
		Address = &BlueBox_DeviceDescriptor;
		Size    = sizeof(USB_Descriptor_Device_t);
		break;
	case DTYPE_Configuration:
		Address = &BlueBox_ConfigurationDescriptor;
		Size    = sizeof(USB_Descriptor_Configuration_t);
		break;
	case DTYPE_String:
		switch (DescriptorNumber)
		{
			case 0x00:
				Address = &BlueBox_LanguageString;
				Size    = pgm_read_byte(&BlueBox_LanguageString.Header.Size);
				break;
			case 0x01:
				Address = &BlueBox_ManufacturerString;
				Size    = pgm_read_byte(&BlueBox_ManufacturerString.Header.Size);
				break;
			case 0x02:
				Address = &BlueBox_ProductString;
				Size    = pgm_read_byte(&BlueBox_ProductString.Header.Size);
				break;
			case 0x03:
				Address = &BlueBox_SerialString;
				Size    = pgm_read_byte(&BlueBox_SerialString.Header.Size);
				break;
		}

		break;
	}

	*DescriptorAddress = Address;
	return Size;
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	Endpoint_ConfigureEndpoint(IN_EPADDR,  EP_TYPE_BULK, IN_EPSIZE,  1);
	Endpoint_ConfigureEndpoint(OUT_EPADDR, EP_TYPE_BULK, OUT_EPSIZE, 1);
}
