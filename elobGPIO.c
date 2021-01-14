/*
 * elobGPIO.c
 *
 * Created: 18.03.2020 17:38:20
 *  Author: rufg
 */ 

//***INCLUDES***
#include "elobGPIO.h"
#include <avr/io.h>

//***DEFINES***

//***STRUKTUREN***

//***VARIABLEN GLOBAL***

//***FUNKTIONSPROTOTYPEN***

//***INTERRUPTS***

//***FUNKTIONEN***
/**
 * Erkennt flanken
 * @param:	pointer auf Struktur flanke von gewünschtem PIN
 */

void flankenerkennung_digi(struct flanken_digi *myPIN)
{
	unsigned char state_pin = 0;
	state_pin = *(myPIN->pin);	//speichert aktuellen Zustand (sicher dass keine änderung während durchlauf)
		
	(myPIN->flanke_pos) = state_pin & (~(myPIN->old));
	(myPIN->flanke_neg) = (~(state_pin)) & myPIN->old;
	(myPIN->old) = state_pin;
}

/**
 * lässt LEDs blinken
 * @param:	pointer auf Struktur blinken mit gewünschtr/em LED
 */
void ledBlinken(struct blinken *myLED)
{
	if(myLED->frequenz_pv > myLED->frequenz_sp)
		myLED->frequenz_pv = 0;
	if(myLED->frequenz_pv < (myLED->frequenz_sp / myLED->dutycycle))
		LEDs |= myLED->led_position;
	else
		LEDs &= ~myLED->led_position;
	myLED->frequenz_pv++;
}

/**
 * initialisiert Digitals I/O
 */
void elobDigiIO_init()
{
	DDRA = 0xFF;	//Output LED
	DDRC = 0x00;	//Input SW
	DDRJ = 0x00;	//Input	TST
}