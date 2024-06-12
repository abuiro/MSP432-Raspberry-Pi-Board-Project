#include "msp.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <msp432p401r.h>
#include "header.h"

//269 Functions

//initializes UART
void UARTInit(void)
{
	EUSCI_A0 ->CTLW0 |= 1;
	EUSCI_A0 ->MCTLW = 0;
	EUSCI_A0 ->CTLW0 |= 0x80;
	EUSCI_A0 ->BRW = 0x34;
	EUSCI_A0 ->CTLW0 &= ~0x01;
	P1->SEL0 |= 0x0C;
	P1->SEL1 &= ~0x0C;
	return;
}

void TX(char text[])
{
	int i =0;
	while(text[i] != '\0')
	{
		EUSCI_A0 ->TXBUF = text[i];
		while((EUSCI_A0 ->IFG & 0x02) == 0)
		{
			//wait until character sent
		}
		i++;
	}
	return;
}

int RX(void)
{
	int i = 0;
	char command[2];
	char x;
	while(1)
{
			if((EUSCI_A0 ->IFG & 0x01) !=0) //data in RX buffer
			{
					command[i] = EUSCI_A0 ->RXBUF;
					EUSCI_A0 ->TXBUF = command[i]; //echo
					while((EUSCI_A0 ->IFG & 0x02)==0); //wait
					if(command[i] == '\r')
					{
							command[i] = '\0';
							break;
					}
					else
					{
							i++;
					}
			}
	}

	x = atoi(command);

	TX("\n\r");

	return x;
}


//ADC Functions
void ADCInit(void)
{
	//Ref_A settings
	REF_A ->CTL0 &= ~0x8; //enable temp sensor
	REF_A ->CTL0 |= 0x30; //set ref voltage
	REF_A ->CTL0 &= ~0x01; //enable ref voltage
	//do ADC stuff
	ADC14 ->CTL0 |= 0x10; //turn on the ADC
	ADC14 ->CTL0 &= ~0x02; //disable ADC
	ADC14 ->CTL0 |=0x4180700; //no prescale, mclk, 192 SHT
	ADC14 ->CTL1 &= ~0x1F0000; //configure memory register 0
	ADC14 ->CTL1 |= 0x800000; //route temp sense
	ADC14 ->MCTL[0] |= 0x100; //vref pos int buffer
	ADC14 ->MCTL[0] |= 22; //channel 22
	ADC14 ->CTL0 |=0x02; //enable adc
	return;
}

float tempRead(void)
{
	float temp; //temperature variable
	uint32_t cal30 = TLV ->ADC14_REF2P5V_TS30C; //calibration constant
	uint32_t cal85 = TLV ->ADC14_REF2P5V_TS85C; //calibration constant
	float calDiff = cal85 - cal30; //calibration difference
	ADC14 ->CTL0 |= 0x01; //start conversion
	while((ADC14 ->IFGR0) ==0)
	{
		//wait for conversion
	}
	temp = ADC14 ->MEM[0]; //assign ADC value
	temp = (temp - cal30) * 55; //math for temperature per manual
	temp = (temp/calDiff) + 30; //math for temperature per manual
	return temp; //return temperature in degrees C
}



void rgb(){
	int comb;
	int toggle;
	int blinks;
	int i;
	
	TX("Enter combination of RGB (1-7):");
	comb = RX();
	if(comb < 1 || comb > 7){
		comb = 7;
	}
	TX("Enter Toggle Time:");
	toggle = RX();
	TX("Enter Number of Blinks:");
	blinks = RX();
	
	P2->SEL1 &= ~7; //activate the LEDS for 111
	P2->SEL0 &= ~7;
	P2->DIR |= 7;
	P2->OUT |= 7;	//stop outputting values for the pins
	
	//set up timer32
	TIMER32_1 -> LOAD = ((3000000 * toggle) - 1); //load value onto timer
	TIMER32_1 -> CONTROL |= 0x42; //no interrupt, periodic mode
	
	TX("Blinking LEDs...\n\r");
	
	P2 -> OUT = comb; //LED Lights turn on 
	
	for (i = 0; i < ((blinks*2)-1); i++){
		TIMER32_1 -> CONTROL |= 0x80; //enable the timer
		while((TIMER32_1 -> RIS & 1) != 1){ //if else statement with modulus used first, this made the toggle and blink for LED better
		//waiting until timer goes down
		}
		
		P2 -> OUT ^= comb; //1 toggle LED cycle
		
		TIMER32_1 -> INTCLR &= ~0x01; //set intclr to 0
		TIMER32_1 -> CONTROL &= ~0x80; //disable timer
		TIMER32_1 -> LOAD = ((3000000 * toggle) -1);
	
	
}
	TIMER32_1 -> INTCLR &= ~0x01; //set intclr to 0 outside of for loop
	P2 -> OUT &= ~0x01; //turn off pin 2

	TX("Done\n\r");

}

void digitalInput(void)
{
	int buttonOne;
	int buttonTwo;
	
	P1->SEL0 &= ~0x12;		
	P1->SEL1 &= ~0x12;		// Set Port 1, Pins 1 and 4 -> 0x12 = 10010, 1 and 2
	P1->DIR |= ~0x12;			// Makes Pins 1 and 4 the outputs
	
	P1->REN |= 0x12;			// Internal resistor for pin 1 and 4
	P1->OUT |= 0x12;			// Set internal resistor to pull up resistor
	buttonOne = P1->IN & 0x02;
	buttonTwo = P1->IN & 0x10;
	
	if((buttonOne == 0) && (buttonTwo == 0)){ //both buttons are LOW
		TX("Both buttons are pressed\n\r");
	} else if (buttonOne == 0){  //button 1 is LOW
		TX("Button 1 Pressed\n\r");
	} else if (buttonTwo == 0){ //button 2 is LOW
		TX("Button 2 Pressed\n\r");
	} else{
		TX("No Button Pressed\n\r"); //no button is pressed
	}
}

void tempReadings(void)
{
	int numTemp;
	float cel;
	float farenheit;
	int i;
	char readings[100];
	
	//setup SysTick
	SysTick -> LOAD = 3000000 - 1;
	SysTick -> CTRL |= 0x04;
	
	TX("Enter Number of Temperature Reading(1-5):");
	numTemp = RX();
	if(numTemp < 1 || numTemp > 5){
		numTemp = 5;
	}
		
	for (i = 1; i <= numTemp; i++)
	{
		cel = tempRead();
		farenheit = ((cel *(9.0/5.0)) + 32);
		sprintf(readings, "Reading %d: %0.2f C & %0.2f F\n\r", i, cel, farenheit); //print the readings in celcius and farenheit
		TX(readings);
	
		SysTick -> CTRL |= 0x1; //enable the timer
		while((SysTick -> CTRL & 0x10000) == 0)
		{ //while the COUNTFLAG is not set
			//wait
		}
		SysTick -> CTRL &= ~0x1; //disable the timer
		SysTick -> LOAD = 3000000 - 1; //starts a 1 second delay

	}
	SysTick -> CTRL &= ~0x1; //disable systick
	P1 -> OUT &= ~0x01; //Turn off pin 1
	
}