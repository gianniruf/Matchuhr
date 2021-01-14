/*
 * elobLcd.h
 *
 * Created: 01.10.2019 08:21:48
 *  Author: rufg
 */ 

#include <stdio.h>

#ifndef ELOBLCD_H_
#define ELOBLCD_H_

//***DEFINES***
#define lcdBacklight_on		(PORTL |= (1<<3))
#define lcdBacklight_off	(PORTL &= ~(1<<3))

//***FUNKTIONSPROTOTYPEN***
//***Initialialsierung
void elobLcd_init(void);

//***Grundfunktionen
void elobLcd_zeichen(unsigned char);	//gewünschtes Zeichen
void elobLcd_instruktion(unsigned char);	//befehl

//Kontrollfunktionen
void elobLcd_cursorHome(void);
void elobLcd_clearDisplay(void);
void elobLcd_display_on(void);
void elobLcd_display_off(void);
void elobLcd_Cursor_on(void);
void elobLcd_cursor_off(void);
void elobLcd_cursor_blinken(void);
void elobLcd_cursor_nBliken(void);
void elobLcd_display_R(void);
void elobLcd_display_L(void);
void elobLcd_cursor_R(void);
void elobLcd_cursor_L(void);
void elobLcd_cursor_2Line(void);
void elobLcd_cursor_toXY(unsigned char, unsigned char);	//Linie, Position

#endif /* ELOBLCD_H_ */