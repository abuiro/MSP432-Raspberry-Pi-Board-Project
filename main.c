#include "msp.h"
#include "header.h"


int main(void){
	int options;
	
	UARTInit();
	ADCInit();
	
	while(1)
		{
			TX("MSP432 Menu\n\n\r");
			TX("1. RGB Control \n\r");
			TX("2. Digital Input \n\r"); //using UART functions for the menu
			TX("3. Temperature Reading\n\r");
			
			options = RX();
			
			switch(options){
				case(1):
					rgb();
					break;
				case(2):
					digitalInput();
					break;
				case(3):
					tempReadings();
					break;
				default:
					TX("Not a valid option, select 1, 2, or 3");
					break;
			}
	}
}