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

#ifndef _SPI_H_
#define _SPI_H_

static inline void swd_init(void)
{
	/* Setup SWD interrupt on RISING edge */
	EICRB |= _BV(ISC60) | _BV(ISC61);
}

static inline void swd_enable(void)
{
	/* Enable external interrupt on PIN6 */
	EIMSK |= _BV(INT6);
}

static inline void swd_disable(void)
{
	/* Disable external interrupt on PIN6 */
	EIMSK &= ~(_BV(INT6));
}

/** Size of the circular receive buffer, must be power of 2 and max 256! */
#ifndef SPI_RX_BUFFER_SIZE
#define SPI_RX_BUFFER_SIZE 16
#endif

/** Size of the circular transmit buffer, must be power of 2 and max 256! */
#ifndef SPI_TX_BUFFER_SIZE
#define SPI_TX_BUFFER_SIZE 256
#endif

/* size of RX/TX buffers */
#define SPI_RX_BUFFER_MASK ( SPI_RX_BUFFER_SIZE - 1)
#define SPI_TX_BUFFER_MASK ( SPI_TX_BUFFER_SIZE - 1)

#if (SPI_RX_BUFFER_SIZE & SPI_RX_BUFFER_MASK)
#error RX buffer size is not a power of 2
#endif

#if (SPI_TX_BUFFER_SIZE & SPI_TX_BUFFER_MASK)
#error TX buffer size is not a power of 2
#endif

/* SPI Controller */
/* Masks */
#define MSK_SPI_MASTER            (1<<MSTR)
#define MSK_SPI_DATA_MODE         ((1<<CPOL)|(1<<CPHA))
#define MSK_SPI_CLOCK_RATE        ((1<<SPR1)|(1<<SPR0))
#define MSK_SPI_CONFIG            ((1<<DORD)|MSK_SPI_MASTER|MSK_SPI_DATA_MODE|MSK_SPI_CLOCK_RATE)

/* Pre-definitions for master/slave configuration */
#define SPI_MASTER                (1<<MSTR)
#define SPI_SLAVE                 (0<<MSTR)

/* Pre-definitions for bit configuration */
#define SPI_LSB_FIRST             (1<<DORD)
#define SPI_MSB_FIRST             (0<<DORD)

/* Pre-definitions for data (CPOL & CPHA) modes */
#define SPI_DATA_MODE_0           ((0<<CPOL)|(0<<CPHA))
#define SPI_DATA_MODE_1           ((0<<CPOL)|(1<<CPHA))
#define SPI_DATA_MODE_2           ((1<<CPOL)|(0<<CPHA))
#define SPI_DATA_MODE_3           ((1<<CPOL)|(1<<CPHA))

/* Pre-definitions for relationship between SCK and CLKIO (16-bit) */
/* (The 9th is the Double SPI Speed Bit: SPI2X of SPSR) */
#define SPI_CLKIO_BY_2            ((1<<(SPI2X+8))|(0<<SPR1)|(0<<SPR0))  // 0x100
#define SPI_CLKIO_BY_4            ((0<<(SPI2X+8))|(0<<SPR1)|(0<<SPR0))  // 0x000
#define SPI_CLKIO_BY_8            ((1<<(SPI2X+8))|(0<<SPR1)|(1<<SPR0))  // 0x101
#define SPI_CLKIO_BY_16           ((0<<(SPI2X+8))|(0<<SPR1)|(1<<SPR0))  // 0x001
#define SPI_CLKIO_BY_32           ((1<<(SPI2X+8))|(1<<SPR1)|(0<<SPR0))  // 0x102
#define SPI_CLKIO_BY_64           ((0<<(SPI2X+8))|(1<<SPR1)|(0<<SPR0))  // 0x102
#define SPI_CLKIO_BY_128          ((0<<(SPI2X+8))|(1<<SPR1)|(1<<SPR0))  // 0x103

#define SPI_MODE_RX               0
#define SPI_MODE_TX               1

#define spi_enable()              (SPCR |=  (1<<SPE))
#define spi_disable()             (SPCR &= ~(1<<SPE))
#define spi_enable_it()           (SPCR |=  (1<<SPIE))
#define spi_disable_it()          (SPCR &= ~(1<<SPIE))
#define spi_select_master_mode()  (SPCR |=  (1<<MSTR))
#define spi_select_slave_mode()   (SPCR &= ~(1<<MSTR))

#define spi_init_config(config)   { spi_init_bus( (((unsigned char)config) & MSK_SPI_MASTER) ); \
                                    SPCR &= ~MSK_SPI_CONFIG;                       \
                                    SPCR |= ((unsigned char)config) & MSK_SPI_CONFIG;         \
                                    SPSR |= (unsigned char)(config >> 8);                     }

#define spi_read_data()           (SPDR)
#define spi_get_byte()            (SPDR)
#define spi_write_data(ch)        (SPDR=ch)
#define spi_send_byte(ch)         (SPDR=ch)

#define spi_wait_spif()           { while (spi_tx_ready() == 0); }      // For any SPI rate
#define spi_wait_eor()            spi_wait_spif()                       // Wait end of reception
#define spi_wait_eot()            spi_wait_spif()                       // Wait end of transmission
#define spi_eor()                 ((SPSR & (1<<SPIF)) == (1<<SPIF))     // Check end of reception
#define spi_eot()                 ((SPSR & (1<<SPIF)) == (1<<SPIF))     // Check end of transmission

#define spi_get_colision_status() (SPSR & (1<<WCOL))
#define spi_tx_ready()            (SPSR & (1<<SPIF))
#define spi_rx_ready()            (spi_tx_ready())

#define spi_init_bus(master)      (                                             \
                                  (master==0) ?                                 \
                                  (DDRB|=(1<<DDB3), DDRB &= ~(1<<DDB1)) :       \
                                  (spi_init_ss(), DDRB|=(1<<DDB2)|(1<<DDB1))    \
                                  )

/* MASTER usage ONLY */
#define spi_init_ss()             (DDRB  |=  (1<<DDB0))  // -- Can be user re-defined 
#define spi_disable_ss()          (PORTB |=  (1<<PB0))   // -- Can be user re-defined 
#define spi_enable_ss()           (PORTB &= ~(1<<DDB0))  // -- Can be user re-defined 

#define spi_write_dummy()         { (SPDR = 0x00); spi_wait_spif(); }
#define spi_read_dummy()          ({  asm                                 \
                                      (                                   \
                                          "in  r0, %0"      "\n\t"        \
                                          :                               \
                                          : "I" (_SFR_IO_ADDR(SPSR))      \
                                          : "r0"                          \
                                      );                                  })

#define spi_ack_read()            spi_read_dummy()                                  
#define spi_ack_write()           spi_read_dummy()                                  
#define spi_ack_cmd()             spi_read_dummy()

void vTaskFindlength(void * pvParameters);
void spi_init(unsigned char config);
void spi_rx_start(void);
void spi_rx_done(void);
void spi_tx_start(void);
void spi_tx_done(void);
int spi_tx_wait(void);

extern uint8_t data[NUM_BUFS][DATA_LENGTH];
extern uint8_t front;

#endif /* _SPI_H_ */
