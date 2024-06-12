#ifndef header_h
#define header_h
#include <stdio.h>

void UARTInit(void);
void TX(char text[]);
int RX(void);

void ADCInit(void);
float tempRead(void);

void rgb(void);
void digitalInput(void);
void tempReadings(void);


#endif