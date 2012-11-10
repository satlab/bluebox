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

#include <avr/wdt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <LUFA/Common/Common.h>
//#include <LUFA/Drivers/USB/USB.h>

#include "bluebox.h"

uint32_t boot_key ATTR_NO_INIT;

#define FLASH_SIZE_BYTES		0x8000
#define BOOTLOADER_SEC_SIZE_BYTES	0x2000
#define MAGIC_BOOT_KEY			0xDC42ACCA
#define BOOTLOADER_START_ADDRESS	(FLASH_SIZE_BYTES - BOOTLOADER_SEC_SIZE_BYTES)

void bootloader_jump_check(void) ATTR_INIT_SECTION(3);
void bootloader_jump_check(void)
{
	/* If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader */
	if ((MCUSR & (1<<WDRF)) && (boot_key == MAGIC_BOOT_KEY)) {
		boot_key = 0;
		((void (*)(void))BOOTLOADER_START_ADDRESS)(); 
	}
}

void jump_to_bootloader(void)
{
	/* Disable all interrupts */
	cli();

	/* Wait two seconds for the USB detachment to register on the host */
	for (uint8_t i = 0; i < 128; i++)
		_delay_ms(16);

	/* Set the bootloader key to the magic value and force a reset */
	boot_key = MAGIC_BOOT_KEY;
	reboot();
}
