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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define XTAL_FREQ 		16000000

#define PA_SETTING		8

/* Default settings */
#define FREQUENCY		437450000
#define TX_WAIT_TIMEOUT		120U
#define TX_TIMEOUT_DELAY	10U
#define RX_WAIT_TIMEOUT		120U
#define CSMA_TIMEOUT		60U
#define CSMA_RSSI		-70
#define BAUD_RATE		2400
#define MOD_INDEX		8
#define AFC_RANGE		10
#define AFC_KI			11
#define AFC_KP			4
#define AFC_ENABLE		1
#define IF_FILTER_BW		2
#define SYNC_WORD_TOL		3

#define BITS_PER_BYTE		8

#endif /* _CONFIG_H_ */
