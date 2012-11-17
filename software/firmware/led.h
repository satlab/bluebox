#ifndef _LED_H_
#define _LED_H_

#include <avr/io.h>
#include "config.h"

/* LED numbers */
#define LED_POWER		0x01
#define LED_RECEIVE		0x02
#define LED_TRANSMIT		0x04
#define LED_ALL			(LED_POWER | LED_RECEIVE | LED_TRANSMIT)

/* Power LED pins */
#define PORT_LED_POWER		PORTF
#define DIR_LED_POWER		DDRF
#define PIN_LED_POWER		4

/* Receive LED pins */
#define PORT_LED_RECEIVE	PORTF
#define DIR_LED_RECEIVE		DDRF
#define PIN_LED_RECEIVE		1

/* Transmit LED pins */
#define PORT_LED_TRANSMIT	PORTF
#define DIR_LED_TRANSMIT	DDRF
#define PIN_LED_TRANSMIT	0

static inline void led_on(unsigned int leds)
{
	if (leds & LED_POWER)
		PORT_LED_POWER &= ~_BV(PIN_LED_POWER);
	if (leds & LED_RECEIVE)
		PORT_LED_RECEIVE &= ~_BV(PIN_LED_RECEIVE);
	if (leds & LED_TRANSMIT)
		PORT_LED_TRANSMIT &= ~_BV(PIN_LED_TRANSMIT);
}

static inline void led_off(unsigned int leds)
{
	if (leds & LED_POWER)
		PORT_LED_POWER |= _BV(PIN_LED_POWER);
	if (leds & LED_RECEIVE)
		PORT_LED_RECEIVE |= _BV(PIN_LED_RECEIVE);
	if (leds & LED_TRANSMIT)
		PORT_LED_TRANSMIT |= _BV(PIN_LED_TRANSMIT);
}

static inline void led_toggle(unsigned int leds)
{
	if (leds & LED_POWER)
		PORT_LED_POWER ^= _BV(PIN_LED_POWER);
	if (leds & LED_RECEIVE)
		PORT_LED_RECEIVE ^= _BV(PIN_LED_RECEIVE);
	if (leds & LED_TRANSMIT)
		PORT_LED_TRANSMIT ^= _BV(PIN_LED_TRANSMIT);
}

static inline void led_init(void)
{
	DIR_LED_POWER 	 |= _BV(PIN_LED_POWER);
	DIR_LED_RECEIVE  |= _BV(PIN_LED_RECEIVE);
	DIR_LED_TRANSMIT |= _BV(PIN_LED_TRANSMIT);
	led_off(LED_ALL);
}

#endif /* _LED_H_ */
