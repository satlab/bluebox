#ifndef _PTT_H_
#define _PTT_H_

#include <avr/io.h>

#if defined(BBSTANDARD)
/* RF control pins */
#define PORT_RF_CONTROL		PORTD
#define DIR_RF_CONTROL		DDRD
#define PIN_RF_TX		2
#define PIN_RF_RX		3
#define PIN_EXT_LNA     	5

/* PA_BIAS/EXT_LNA pins */
#define PORT_PALNA_CONTROL	PORTB
#define DIR_PALNA_CONTROL	DDRB
#define PIN_PA_BIAS_EN		4
#define PIN_EXT_PTT		7
#elif defined(BBMICRO)
/* RF control pins */
#define PORT_RF_CONTROL		PORTD
#define DIR_RF_CONTROL		DDRD
#define PIN_RF_TX		6
#define PIN_RF_RX		4
#endif


static inline void ptt_high(unsigned int delay)
{
#if defined(PIN_EXT_LNA)
	/* Power off external LNA */
	PORT_RF_CONTROL &= ~_BV(PIN_EXT_LNA);

	/* Wait for external LNA to switch off */
	delay_ms(50);
#endif

	/* Switch on-board RX/TX switch to transmit */
	PORT_RF_CONTROL &= ~_BV(PIN_RF_RX);
	PORT_RF_CONTROL |=  _BV(PIN_RF_TX);
#if defined(PIN_EXT_LNA)
	PORT_RF_CONTROL &= ~_BV(PIN_EXT_LNA);

	/* Power on on-board PA and disable on-board LNA */
	PORT_PALNA_CONTROL |= _BV(PIN_PA_BIAS_EN);

	/* Wait for on-board PA and LNA to settle */
	delay_ms(5);

	/* Power on external PA */
	PORT_PALNA_CONTROL |= _BV(PIN_EXT_PTT);
#endif

	/* Wait for external PA to settle */
	delay_ms(delay);
}

static inline void ptt_low(unsigned int delay)
{
#if defined(PIN_EXT_PTT)
	/* Power off external PA */
	PORT_PALNA_CONTROL &= ~_BV(PIN_EXT_PTT);

	/* Wait for external PA to settle */
	delay_ms(delay);

	/* Power on external LNA */
	PORT_RF_CONTROL |=  _BV(PIN_EXT_LNA);

	/* Power off on-board PA */
	PORT_PALNA_CONTROL &= ~_BV(PIN_PA_BIAS_EN);
#endif

	/* Switch on-board RX/TX switch to receive */
	PORT_RF_CONTROL &= ~_BV(PIN_RF_TX);
	PORT_RF_CONTROL |=  _BV(PIN_RF_RX);
#if defined(PIN_EXT_LNA)
	PORT_RF_CONTROL |=  _BV(PIN_EXT_LNA);
#endif
}

static inline void ptt_init(void)
{
	DIR_RF_CONTROL    |= _BV(PIN_RF_TX) | _BV(PIN_RF_RX);
#if defined(PIN_EXT_LNA)
	DIR_RF_CONTROL    |= _BV(PIN_EXT_LNA);
	DIR_PALNA_CONTROL |= _BV(PIN_PA_BIAS_EN) | _BV(PIN_EXT_PTT);
#endif
	ptt_low(0);
}

#endif /* _PTT_H_ */
