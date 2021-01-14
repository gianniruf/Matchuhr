/*
 * elobLcd.c
 *
 * Created: 01.10.2019 08:21:22
 *  Author: rufg
 */ 

//***INCLUDES***
#include "elobLcd.h"
#include <avr/io.h>
#define F_CPU 16000000	//16MHz
#include <util/delay.h>
#include <stdio.h>

//***DEFINES***
#define lcdRegData			(PORTL |= (1<<0))	//RS
#define lcdRegInstruction	(PORTL &= ~(1<<0))	//
#define lcdRead		(PORTL |= (1<<1))	//R/W
#define lcdWrite	(PORTL &= ~(1<<1))	//
#define lcdEnable	(PORTL |= (1<<2))	//E (Clock)
#define lcdDisable	(PORTL &= ~(1<<2))	//

#define lcdData0_on		(PORTL |= (1<<4))
#define lcdData0_off	(PORTL &= ~(1<<4))
#define lcdData1_on		(PORTL |= (1<<5))
#define lcdData1_off	(PORTL &= ~(1<<5))
#define lcdData2_on		(PORTL |= (1<<6))
#define lcdData2_off	(PORTL &= ~(1<<6))
#define lcdData3_on		(PORTL |= (1<<7))
#define lcdData3_off	(PORTL &= ~(1<<7))

//***FUNKTIONSPROTOTYPEN***
int lcdPuts(char, FILE *);
void lcdPutch(char);

void elobLcd_4bit_daten(unsigned char);	//Daten
void elobLcd_8bit_daten(unsigned char);	//Daten

//***GLOBALE VARIABLEN***
static FILE lcdstdout = FDEV_SETUP_STREAM(lcdPuts, NULL, _FDEV_SETUP_WRITE);


/*
 * Initialisiert LCD
 */
void elobLcd_init(void)
{
	_delay_ms(22);
	
	stdout = &lcdstdout;
	lcdRegInstruction;
	
	elobLcd_4bit_daten(0x30);
	_delay_ms(4.5);
	elobLcd_4bit_daten(0x30);
	_delay_ms(1);
	elobLcd_4bit_daten(0x30);
	_delay_ms(2);
	elobLcd_4bit_daten(0x20);
	_delay_ms(2);
	elobLcd_8bit_daten(0x28);
	_delay_ms(2);
	elobLcd_8bit_daten(0x08);
	_delay_ms(2);
	elobLcd_8bit_daten(0x0F);
	_delay_ms(2);
	elobLcd_8bit_daten(0x06);
	_delay_ms(2);
	
	elobLcd_clearDisplay();
	_delay_ms(2);
}

void elobLcd_4bit_daten(unsigned char daten)
{
	lcdEnable;
	lcdWrite;
	(daten & (1<<4))? lcdData0_on : lcdData0_off;
	(daten & (1<<5))? lcdData1_on : lcdData1_off;
	(daten & (1<<6))? lcdData2_on : lcdData2_off;
	(daten & (1<<7))? lcdData3_on : lcdData3_off;
	_delay_us(2);	//geht nicht anderst
	lcdDisable;
}

void elobLcd_8bit_daten(unsigned char daten)
{
	elobLcd_4bit_daten(daten);
	elobLcd_4bit_daten(daten << 4);	//LowNibble
}

void elobLcd_zeichen(unsigned char zeichen)
{
	lcdRegData;
	elobLcd_8bit_daten(zeichen);
}

void elobLcd_instruktion(unsigned char befehl)
{
	lcdRegInstruction;
	elobLcd_8bit_daten(befehl);
}

/**
 * sendet string der printf-funktion
 * @parm:	var:	aktuelles Zeichen
 * @param:	?
 * @return:	null = fertig
 */
int lcdPuts(char var, FILE *stream)
{
	lcdPutch(var);
	return 0;
}

/**
 * Ersetzt ASCII-Befehler
 */
void lcdPutch(char zeichen)
{
	switch(zeichen)
	{
		case '\n':	//new Line
			elobLcd_cursor_2Line();
			break;
		case '\b':	//back
			elobLcd_cursor_L();
			break;	
		case 9:		//TAB
			elobLcd_cursor_R();
		case ' ':	//Leerzeichen
			elobLcd_cursor_R();
			break;
		case 'ö':
			elobLcd_zeichen('o');
			elobLcd_zeichen('e');
			break;
		case 'ä':
			elobLcd_zeichen('a');
			elobLcd_zeichen('e');
			break;
		case 'ü':
			elobLcd_zeichen('u');
			elobLcd_zeichen('e');
			break;
		default:
			elobLcd_zeichen(zeichen);
			break;
	}
}

/**
 * Instruktionsbefehle
 */
void elobLcd_cursorHome(void)
{
	elobLcd_instruktion(0x02);
}
void elobLcd_clearDisplay(void)
{
	elobLcd_instruktion(0x01);
	_delay_ms(1.7);
}
void elobLcd_display_on(void)
{
	elobLcd_instruktion(0x0C);
}
void elobLcd_display_off(void)
{
	elobLcd_instruktion(0x08);
}
void elobLcd_Cursor_on(void)
{
	elobLcd_instruktion(0x0E);
}
void elobLcd_cursor_off(void)
{
	elobLcd_instruktion(0x0C);
}
void elobLcd_cursor_blinken(void)
{
	elobLcd_instruktion(0x0F);
}
void elobLcd_cursor_nBliken(void)
{
	elobLcd_instruktion(0x0E);
}
void elobLcd_display_R(void)
{
	elobLcd_instruktion(0x1C);
}
void elobLcd_display_L(void)
{
	elobLcd_instruktion(0x18);
}
void elobLcd_cursor_R(void)
{
	elobLcd_8bit_daten(0x14);
}
void elobLcd_cursor_L(void)
{
	elobLcd_instruktion(0x10);
}
void elobLcd_cursor_2Line(void)
{
	elobLcd_instruktion(0xC0);
}

/*
 * @param	line:		1, 2
 * @param	position:	0...16
 */
void elobLcd_cursor_toXY(unsigned char line, unsigned char position)
{
	int i;
	
	if(line == 1)
		elobLcd_cursorHome();
	if(line == 2)
		elobLcd_cursor_2Line();
	for (i = position; i > 0; i--)
	{
		elobLcd_cursor_R();
	}
}