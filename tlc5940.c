/*
 
 tlc5940.c
 
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
 
#include <avr/interrupt.h>

#include "tlc5940.h"
#include "ledarray.h"

#if (3 * 16 * TLC5940_N > 255)
#define channel3_t uint16_t
#else
#define channel3_t uint8_t
#endif

uint8_t dcData[dcDataSize];
volatile uint8_t gsData[gsDataSize];
volatile uint8_t gsUpdateFlag;
volatile uint8_t notClockedinFlag;
volatile uint8_t row;
volatile uint8_t clear_array_flag;
volatile uint8_t xlatNeedsPulse;




#if (TLC5940_MANUAL_DC_FUNCS)
void TLC5940_SetDC(channel_t channel, uint8_t value) {	
	channel = numChannels - 1 - channel;
	channel_t i = (channel3_t)channel * 3 / 4;
	
	switch (channel % 4) {
		case 0:
			dcData[i] = (dcData[i] & 0x03) | (uint8_t)(value << 2);
			break;
		case 1:
			dcData[i] = (dcData[i] & 0xFC) | (value >> 4);
			i++;
			dcData[i] = (dcData[i] & 0x0F) | (uint8_t)(value << 4);
			break;
		case 2:
			dcData[i] = (dcData[i] & 0xF0) | (value >> 2);
			i++;
			dcData[i] = (dcData[i] & 0x3F) | (uint8_t)(value << 6);
			break;
		default: // case 3:
			dcData[i] = (dcData[i] & 0xC0) | (value);
			break;
	}
}

void TLC5940_SetAllDC(uint8_t value) {
	uint8_t tmp1 = (uint8_t)(value << 2);
	uint8_t tmp2 = (uint8_t)(tmp1 << 2);
	uint8_t tmp3 = (uint8_t)(tmp2 << 2);
	tmp1 |= (value >> 4);
	tmp2 |= (value >> 2);
	tmp3 |= value;
	
	dcData_t i = 0;
	do {
		dcData[i++] = tmp1;              // bits: 05 04 03 02 01 00 05 04
		dcData[i++] = tmp2;              // bits: 03 02 01 00 05 04 03 02
		dcData[i++] = tmp3;              // bits: 01 00 05 04 03 02 01 00
	} while (i < dcDataSize);
}

void TLC5940_ClockInDC(void) {
	setHigh(DCPRG_PORT, DCPRG_PIN);
	setHigh(VPRG_PORT, VPRG_PIN);
	
	for (dcData_t i = 0; i < dcDataSize; i++) {
		SPDR = dcData[i];
		while (!(SPSR & (1 << SPIF)));
	}
	pulse(XLAT_PORT, XLAT_PIN);
}
#endif

void TLC5940_SetGS(channel_t channel, uint16_t value) {
	channel = numChannels - 1 - channel;
	channel3_t i = (channel3_t)channel * 3 / 2;
	
	switch (channel % 2) {
		case 0:
			gsData[i] = (value >> 4);
			i++;
			gsData[i] = (gsData[i] & 0x0F) | (uint8_t)(value << 4);			
			break;
		default: // case 1:
			gsData[i] = (gsData[i] & 0xF0) | (value >> 8);
			i++;
			gsData[i] = (uint8_t)value;
			break;
	}
}

void TLC5940_SetAllGS(uint16_t value) {
	uint8_t tmp1 = (value >> 4);
	uint8_t tmp2 = (uint8_t)(value << 4) | (tmp1 >> 4);
	gsData_t i = 0;
	do {
		gsData[i++] = tmp1;              // bits: 11 10 09 08 07 06 05 04
		gsData[i++] = tmp2;              // bits: 03 02 01 00 11 10 09 08
		gsData[i++] = (uint8_t)value;    // bits: 07 06 05 04 03 02 01 00
	} while (i < gsDataSize);
}

void TLC5940_Init(void) {
	setOutput(SCLK_DDR, SCLK_PIN);
	setOutput(DCPRG_DDR, DCPRG_PIN);
	setOutput(VPRG_DDR, VPRG_PIN);
	setOutput(XLAT_DDR, XLAT_PIN);
	setOutput(BLANK_DDR, BLANK_PIN);
	setOutput(SIN_DDR, SIN_PIN);
	
	setLow(SCLK_PORT, SCLK_PIN);
	setLow(DCPRG_PORT, DCPRG_PIN);
	setHigh(VPRG_PORT, VPRG_PIN);
	setLow(XLAT_PORT, XLAT_PIN);
	setHigh(BLANK_PORT, BLANK_PIN);
	
	rowselect(0);			// Selects row 0 to begin
	gsUpdateFlag = 0;
	
	// Enable SPI, Master, set clock rate fck/2
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR = (1 << SPI2X);
	
	// CTC with OCR0A as TOP
	TCCR0A = (1 << WGM01);
	
	// clk_io/1024 (From prescaler)
	TCCR0B = ((1 << CS02) | (1 << CS00));
	
	// Generate an interrupt every 4096 clock cycles
	OCR0A = 3;
	
	// Enable Timer/Counter0 Compare Match A interrupt
	TIMSK0 |= (1 << OCIE0A);
}

void rowselect(uint8_t row)
{
	switch(row)
	{
		case 0:
		setLow(ROW11_PORT,ROW11_PIN);
		setHigh(ROW0_PORT,ROW0_PIN);
		break;
		
		case 1:
		setLow(ROW0_PORT,ROW0_PIN);
		setHigh(ROW1_PORT,ROW1_PIN);
		break;
		
		case 2:
		setLow(ROW1_PORT,ROW1_PIN);
		setHigh(ROW2_PORT,ROW2_PIN);
		break;
		
		case 3:
		setLow(ROW2_PORT,ROW2_PIN);
		setHigh(ROW3_PORT,ROW3_PIN);
		break;
		
		case 4:
		setLow(ROW3_PORT,ROW3_PIN);
		setHigh(ROW4_PORT,ROW4_PIN);
		break;
		
		case 5:
		setLow(ROW4_PORT,ROW4_PIN);
		setHigh(ROW5_PORT,ROW5_PIN);
		break;
		
		case 6:
		setLow(ROW5_PORT,ROW5_PIN);
		setHigh(ROW6_PORT,ROW6_PIN);
		break;
		
		case 7:
		setLow(ROW6_PORT,ROW6_PIN);
		setHigh(ROW7_PORT,ROW7_PIN);
		break;
		
		case 8:
		setLow(ROW7_PORT,ROW7_PIN);
		setHigh(ROW8_PORT,ROW8_PIN);
		break;
		
		case 9:
		setLow(ROW8_PORT,ROW8_PIN);
		setHigh(ROW9_PORT,ROW9_PIN);
		break;
		
		case 10:
		setLow(ROW9_PORT,ROW9_PIN);
		setHigh(ROW10_PORT,ROW10_PIN);
		break;
		
		case 11:
		setLow(ROW10_PORT,ROW10_PIN);
		setHigh(ROW11_PORT,ROW11_PIN);
		break;
	}
}	

void latchin(uint8_t xlatNeedsPulse)
{
	setHigh(BLANK_PORT, BLANK_PIN);
	
	if (outputState(VPRG_PORT, VPRG_PIN)) {
		setLow(VPRG_PORT, VPRG_PIN);
		if (xlatNeedsPulse) {
			pulse(XLAT_PORT, XLAT_PIN);
			xlatNeedsPulse = 0;
		}
		pulse(SCLK_PORT, SCLK_PIN);
	} else if (xlatNeedsPulse) {
		pulse(XLAT_PORT, XLAT_PIN);
		setLow(VPRG_PORT, VPRG_PIN);
		xlatNeedsPulse = 0;
	}
	setLow(BLANK_PORT, BLANK_PIN);
}	

// This interrupt will get called every 4096 clock cycles...do we truly want to latch in the new data every time?
ISR(TIMER0_COMPA_vect) {
	//static uint8_t xlatNeedsPulse = 0;
	/*
	if (clear_array_flag)
	{
		refresh_array();
	}
	*/
	//latchin(xlatNeedsPulse);
	
	
	// Latches in new data only if entire row has been latched in...supposedly.
	
	if (gsUpdateFlag) {
		
		row++;
		if (row == numrows)
		{
			rowselect(numrows-1);
			row=0;
		}
		else
		{
			rowselect(row-1);				// Need to select row -1, as it is immediately incremented in main routine	
		}
		
		for (gsData_t i = 0; i < gsDataSize; i++) {			// uses SPI to send the gs data to the TLC5940
			SPDR = gsData[i];
			while (!(SPSR & (1 << SPIF)));
		}	
		latchin(xlatNeedsPulse);
		xlatNeedsPulse = 0;
		gsUpdateFlag = 0;
		notClockedinFlag = 1;
		TLC5940_SetAllGS(0);			// Clear gs data prior to displaying next array
					}
	
		
}




