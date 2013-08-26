#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "tlc5940.h"
#include "ledarray.h"



volatile uint8_t row = 0;
volatile uint8_t column = 0;
volatile uint8_t notClockedinFlag = 1;
volatile uint8_t clear_array_flag = 0;
volatile uint8_t swap_array_flag = 0;				// When flag is set, swaps array for nextarray.
volatile uint16_t array[numrows][numcolumns];
volatile uint8_t xlatNeedsPulse;
channel_t i = 0;



int main(void) {
	TLC5940_Init();
	array_setup();
	
	#if (TLC5940_MANUAL_DC_FUNCS)
	TLC5940_SetAllDC(63);
	TLC5940_ClockInDC();
	#endif

	// Default all channels to off
	TLC5940_SetAllGS(0);
	
	// Enable Global Interrupts
	sei();
	
	
	row=0;
	column=0;	
	
			
	for (;;) {
		
				// Should not be updating array every time one row is completed...!
		xlatNeedsPulse = 1;
		
		while(notClockedinFlag)	// Checks if we have clocked in all data for current cycle (current row).
		{
			
			
			
			for (column = 0; column < numcolumns; column++)
			{
				TLC5940_SetGS(i-1, array[row][column]*4000);			// this is setting the GS data into the array
				i = (i + 1) % numChannels;
		
				if (i == 0)								// i==0 means we have set GS for all channels on this row...clock in to display row!
				{
					if (row <= numrows-2)
					{
				
						//row++;
						notClockedinFlag = 0;
					}
					
					else if (row == numrows-1)			// finished last row.  Update entire array.
					{

						//row = 0;
						notClockedinFlag = 0;			// Stop updating the gs arrays until row displayed.
						//clear_array_flag = 1;
						swap_array_flag = 1;
						update_array();
						
					}
			
					
					gsUpdateFlag = 1;
					break;
				}	
			
			}
		}			
		
		
		
	
	
			}
}

		




	

