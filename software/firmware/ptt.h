#ifndef _PTT_H_
#define _PTT_H_

#include <avr/io.h>
#include "config.h"

/* RF control pins */
#define PORT_RF_CONTROL		PORTD
#define DIR_RF_CONTROL		DDRD
#define PIN_RF_TX		2
#define PIN_RF_RX		3

/* PA_BIAS/EXT_LNA pins */
#define PORT_PALNA_CONTROL	PORTB
#define DIR_PALNA_CONTROL	DDRB
#define PIN_PA_BIAS_EN		4
#define PIN_EXT_PTT		7

static inline void ptt_high(void)
{
	PORT_RF_CONTROL &= ~_BV(PIN_RF_RX);
	PORT_RF_CONTROL |=  _BV(PIN_RF_TX);

	PORT_PALNA_CONTROL |= _BV(PIN_PA_BIAS_EN);
	PORT_PALNA_CONTROL |= _BV(PIN_EXT_PTT);
}

static inline void ptt_low(void)
{
	PORT_PALNA_CONTROL &= ~_BV(PIN_EXT_PTT);
	PORT_PALNA_CONTROL &= ~_BV(PIN_PA_BIAS_EN);

	PORT_RF_CONTROL &= ~_BV(PIN_RF_TX);
	PORT_RF_CONTROL |=  _BV(PIN_RF_RX);
}

static inline void ptt_init(void)
{
	DIR_RF_CONTROL    |= _BV(PIN_RF_TX) | _BV(PIN_RF_RX);
	DIR_PALNA_CONTROL |= _BV(PIN_PA_BIAS_EN) | _BV(PIN_EXT_PTT);
	ptt_low();
}

#endif /* _PTT_H_ */
