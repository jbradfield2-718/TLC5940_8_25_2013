/*
 
 tlc5940.h
  
 Copyright 2010 Matthew T. Pandina. All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY MATTHEW T. PANDINA "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 EVENT SHALL MATTHEW T. PANDINA OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 */
 
#pragma once

#include <stdint.h>
#include <avr/io.h>

#define SIN_DDR DDRB
#define SIN_PORT PORTB
#define SIN_PIN PB3

#define SCLK_DDR DDRB
#define SCLK_PORT PORTB
#define SCLK_PIN PB5

#define BLANK_DDR DDRB
#define BLANK_PORT PORTB
#define BLANK_PIN PB2

// The following options are configurable from the Makefile
#ifndef DCPRG_DDR
#define DCPRG_DDR DDRD
#endif
#ifndef DCPRG_PORT
#define DCPRG_PORT PORTD
#endif
#ifndef DCPRG_PIN
#define DCPRG_PIN PD6
#endif

#ifndef VPRG_DDR
#define VPRG_DDR DDRD
#endif
#ifndef VPRG_PORT
#define VPRG_PORT PORTD
#endif
#ifndef VPRG_PIN
#define VPRG_PIN PD7
#endif

#ifndef XLAT_DDR
#define XLAT_DDR DDRB
#endif
#ifndef XLAT_PORT
#define XLAT_PORT PORTB
#endif
#ifndef XLAT_PIN
#define XLAT_PIN PB1
#endif

#ifndef TLC5940_MANUAL_DC_FUNCS
#define TLC5940_MANUAL_DC_FUNCS 1
#endif

#ifndef TLC5940_N
#define TLC5940_N 1
#endif

#ifndef ROW0_PORT
#define ROW0_PORT PORTC
#endif

#ifndef ROW0_PIN
#define ROW0_PIN PC0
#endif

#ifndef ROW1_PORT
#define ROW1_PORT PORTC
#endif

#ifndef ROW1_PIN
#define ROW1_PIN PC1
#endif

#ifndef ROW2_PORT
#define ROW2_PORT PORTC
#endif

#ifndef ROW2_PIN
#define ROW2_PIN PC2
#endif

#ifndef ROW3_PORT
#define ROW3_PORT PORTC
#endif

#ifndef ROW3_PIN
#define ROW3_PIN PC3
#endif

#ifndef ROW4_PORT
#define ROW4_PORT PORTC
#endif

#ifndef ROW4_PIN
#define ROW4_PIN PC4
#endif

#ifndef ROW5_PORT
#define ROW5_PORT PORTC
#endif

#ifndef ROW5_PIN
#define ROW5_PIN PC5
#endif

#ifndef ROW6_PORT
#define ROW6_PORT PORTD
#endif

#ifndef ROW6_PIN
#define ROW6_PIN PD0
#endif

#ifndef ROW7_PORT
#define ROW7_PORT PORTD
#endif

#ifndef ROW7_PIN
#define ROW7_PIN PD1
#endif

#ifndef ROW8_PORT
#define ROW8_PORT PORTD
#endif

#ifndef ROW8_PIN
#define ROW8_PIN PD2
#endif

#ifndef ROW9_PORT
#define ROW9_PORT PORTD
#endif

#ifndef ROW9_PIN
#define ROW9_PIN PD3
#endif

#ifndef ROW10_PORT
#define ROW10_PORT PORTD
#endif

#ifndef ROW10_PIN
#define ROW10_PIN PD4
#endif

#ifndef ROW11_PORT
#define ROW11_PORT PORTD
#endif

#ifndef ROW11_PIN
#define ROW11_PIN PD5
#endif
// --------------------------------------------------------

#define setOutput(ddr, pin) ((ddr) |= (1 << (pin)))
#define setLow(port, pin) ((port) &= ~(1 << (pin)))
#define setHigh(port, pin) ((port) |= (1 << (pin)))
#define pulse(port, pin) do { \
                         	setHigh((port), (pin)); \
                         	setLow((port), (pin)); \
                         } while (0)
#define outputState(port, pin) ((port) & (1 << (pin)))

#if (12 * TLC5940_N > 255)
#define dcData_t uint16_t
#else
#define dcData_t uint8_t
#endif

#if (24 * TLC5940_N > 255)
#define gsData_t uint16_t
#else
#define gsData_t uint8_t
#endif

#if (16 * TLC5940_N > 255)
#define channel_t uint16_t
#else
#define channel_t uint8_t
#endif

#define dcDataSize ((dcData_t)12 * TLC5940_N)
#define gsDataSize ((gsData_t)24 * TLC5940_N)
#define numChannels 15	// this was the original ((channel_t)16 * TLC5940_N)




extern uint8_t dcData[dcDataSize];
volatile uint8_t gsData[gsDataSize];
extern volatile uint8_t gsUpdateFlag;

static inline void TLC5940_SetGSUpdateFlag(void) {
	__asm__ volatile ("" ::: "memory");
	gsUpdateFlag = 1;
}
#if (TLC5940_MANUAL_DC_FUNCS)
void TLC5940_SetDC(channel_t channel, uint8_t value);
void TLC5940_SetAllDC(uint8_t value);
void TLC5940_ClockInDC(void);
#endif

void TLC5940_SetGS(channel_t channel, uint16_t value);
void TLC5940_SetAllGS(uint16_t value);
void TLC5940_Init(void);
