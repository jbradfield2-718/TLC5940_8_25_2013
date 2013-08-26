/*
 * ledarray.c
 *
 * Created: 7/7/2013 12:20:27 AM
 *  Author: Justin
 */ 
#include "ledarray.h"
#include "tlc5940.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>
#include <stdlib.h>
#define F_CPU 16000000UL

volatile uint16_t array[numrows][numcolumns];
extern uint8_t swap_array_flag;
extern uint8_t clear_array_flag;
volatile uint8_t row;
volatile uint8_t column;
volatile uint8_t flag = 0;
volatile uint8_t xlatNeedsPulse;
uint16_t nextarray[numrows][numcolumns];
uint8_t reset_game_flag = 0;

void array_setup()
{
	// CTC with OCR0A as TOP
	TCCR1B |= (1 << WGM12);
	TCNT1 |= 0;
	
	// clk_io/1024 (From prescaler)
	TCCR1B |= ((1 << CS12) | (1 << CS10));
	
	// Generate an interrupt @ 1 Hz
	OCR1A = 20000;
	
	// Enable Timer/Counter1 Compare Match B interrupt
	TIMSK1 |= (1 << OCIE1A);
	
	// for loop to init nextarray to zeroes prior to first use
	row = 0; column = 0;
	for (column = 0; column < numcolumns; column++)
	{
		for (row = 0; row < numrows; row++)
		{
			nextarray[row][column] = 0;
		}
	}
	row = 0; column = 0;
	
	seed_array();
}

/* Calculates the next array outside of ISR */
void update_array()
{
//	uint8_t x = 0;			// Use local variables x and y to increment the row and column when updating
//	uint8_t y = 0;
/*	if (flag ==0)
	{
		for (y = 0; y < numcolumns; y++)
		{
			for (x = 0; x < numrows; x++)
			{
				if(x == 10 && y==3)
			{nextarray[x][y] = 4000;}
				else
				{
					nextarray[x][y] = 0;
				}
			}
		}
		flag = 1;
	}
	
	else if (flag ==1)
	{x = 0; y = 0;
		for (y = 0; y < numcolumns; y++)
		{
			for (x = 0; x < numrows; x++)
			{
				if(x == 5  )
			{nextarray[x][y] = 4000;}
				else
				
				{
					nextarray[x][y] = 0;
				}
			}
		}
		flag = 0;
	}
	//refresh_array();				// Only after the entire array has been updated to we want to */
life();
	
}

// Should clear entire buffer of all 16 channels to zero prior to writing the new array data to  the LED array

void life()
{
	uint8_t num_neighbors;
	if (reset_game_flag == 1)
		{
			reset_game();
		}
	
	for (uint8_t y = 0; y < numcolumns; y++)
		{
			for (uint8_t x = 0; x < numrows; x++)
			{
				num_neighbors = calc_neighbors(x,y);
				
				// Current array live cell cases-----------------------------------
				if (array[x][y] == 1)
				{
					if(num_neighbors < 2)
					{
						nextarray[x][y] = 0;
					}
					if(num_neighbors == 2 || num_neighbors == 3)
					{
						nextarray[x][y] = 1;
					}
					if(num_neighbors > 3)
					{
						nextarray[x][y] = 0;
					}
				}
				// Current array dead cell cases-----------------------------------
				else
				{
					if(num_neighbors == 3)
					{
						nextarray[x][y] = 1;
					}
				}
			}
		}
		
		// Restarts if the array is frozen
		if (memcmp(array, nextarray, sizeof (array)) == 0)
		{
			reset_game_flag = 1;
			
		}
		
	
				
}

void reset_game()
{
	for(uint8_t i=0; i<500; i++)
		{_delay_ms(2);}
	seed_array();
	reset_game_flag = 0;
}

// Works with life subroutine to calculate the number of neighbors each cell has in the current array
int calc_neighbors(current_row, current_column)
{
	// Calc boundry conditions first...the four vertices of the array
	// Lower Right
	if(current_row == 0 && current_column == 0)
	{
		return array[current_row][current_column +1] + array[current_row][numcolumns-1] + array[current_row+1][numcolumns-1] + array[current_row+1][current_column+1] +
				array[current_row+1][current_column] + array[numrows-1][current_column] + array[numrows-1][current_column+1] + array[numrows-1][numcolumns-1];
	}
	// Lower Left
	if(current_row == 0 && current_column == numcolumns-1)
	{
		return array[current_row][current_column -1] + array[current_row+1][current_column] + array[current_row+1][current_column-1] + array[current_row][0] +
				array[current_row+1][0] + array[numrows-1][current_column] + array[numrows-1][current_column-1] + array[numrows-1][0];
	}
	//upper right
	if(current_row == numrows-1 && current_column == 0)
	{
		return array[current_row][current_column +1] + array[current_row-1][current_column-1] + array[current_row-1][current_column] + array[current_row][numcolumns-1] +
				array[current_row-1][numcolumns-1] + array[0][current_column] + array[0][current_column+1] + array[0][numcolumns-1];
	}
	//upper left
	if(current_row == numrows-1 && current_column == numcolumns-1)
	{
		return array[current_row][current_column -1] + array[current_row-1][current_column-1] + array[current_row-1][current_column] + array[0][current_column] +
				array[0][current_column-1] + array[current_row][0] + array[current_row-1][0] + array[0][0];
	}
	
	// Now fall through to the edges not at vertices..
	//Bottom edge
	if(current_row == 0)
	{
		return array[current_row][current_column +1] + array[current_row+1][current_column+1] + array[current_row+1][current_column] + array[current_row+1][current_column-1] +
				array[current_row][current_column-1] + array[numrows-1][current_column+1] + array[numrows-1][current_column] + array[numrows-1][current_column-1];
	}
	//Top edge
	if(current_row == numrows-1)
	{
		return array[current_row][current_column +1] + array[current_row-1][current_column+1] + array[current_row-1][current_column] + array[current_row-1][current_column-1] +
				array[current_row][current_column-1] + array[0][current_column+1] + array[0][current_column] + array[0][current_column-1];
	}
	//Right edge
	if(current_column == 0)
	{
		return array[current_row+1][current_column] + array[current_row+1][current_column+1] + array[current_row][current_column+1] + array[current_row-1][current_column+1] +
				array[current_row-1][current_column] + array[current_row+1][numcolumns-1] + array[current_row][numcolumns-1] + array[current_row-1][numcolumns-1];
	}
	//Left edge
	if(current_column == numcolumns-1)
	{
		return array[current_row+1][current_column] + array[current_row+1][current_column-1] + array[current_row][current_column-1] + array[current_row-1][current_column-1] +
				array[current_row-1][current_column] + array[current_row+1][0] + array[current_row][0] + array[current_row-1][0];
	}
	
	// Fall through to base case in center of Board...
	return array[current_row+1][current_column +1] + array[current_row+1][current_column] + array[current_row+1][current_column-1] + array[current_row][current_column-1] +
				array[current_row-1][current_column-1] + array[current_row-1][current_column] + array[current_row-1][current_column+1] + array[current_row][current_column+1];
}

void seed_array()
{
	srand(TCNT0);			// Seeds random numbers from Timer 0.
	
	for (uint8_t y = 0; y < numcolumns; y++)			// Creates the random initial state of the array with 1s and 0s.
		{
			for (uint8_t x = 0; x < numrows; x++)
			{
				if(rand() % 2 == 0)
				{
					array[x][y] = 1;
				}
				else
				{
					array[x][y] = 0;
				}
			}
		}
	
}

void refresh_array()
{
	for (int i=0; i < numChannels; i++)
	{
		TLC5940_SetAllGS(0);
		for (gsData_t i = 0; i < gsDataSize; i++) {
			SPDR = gsData[i];
			while (!(SPSR & (1 << SPIF)));
		}
		latchin(1);					// Latches it in		
	}
	clear_array_flag = 0;
	row = 0; column = 0;			// reset row and column to start writing to array from bottom left.
}

	
// The only thing this ISR does is swaps the current array with the next array.
// Need to check if the entire array has been written to the screen prior to refreshing.  This should be fast.

ISR(TIMER1_COMPA_vect) {
	if (swap_array_flag)
	{
		memcpy(&array, &nextarray, sizeof(array));		// Swaps nextarray with array
		swap_array_flag = 0;
		//TLC5940_SetAllGS(0);
		refresh_array();
		
		
	
	}
		
	}
	
	

