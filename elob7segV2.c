/*
 * elob7segV2.c
 *
 * Created: 02.04.2020 14:48:30
 *  Author: rufg
 */ 

//***INCLUDES***
#include "elob7segV2.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

//***DEFINES***
#define en7seg_off	(PORTG &= ~(1<<2))
#define en7seg_on	(PORTG |= (1<<2))
#define datMuliplexer_set(stelle)	(PORTG = (stelle<<0))
#define komma_on	(PORTH |= (1<<7))
#define komma_off	(PORTH &= ~(1<<7))

#define ledSeg_off	(PORTH = 0b00000000)
#define ledSeg0	(PORTH = 0b00111111)
#define ledSeg1	(PORTH = 0b00000110)
#define ledSeg2	(PORTH = 0b01011011)
#define ledSeg3	(PORTH = 0b01001111)
#define ledSeg4	(PORTH = 0b01100110)
#define ledSeg5	(PORTH = 0b01101101)
#define ledSeg6	(PORTH = 0b01111101)
#define ledSeg7	(PORTH = 0b00000111)
#define ledSeg8	(PORTH = 0b01111111)
#define ledSeg9	(PORTH = 0b01101111)
#define ledSegA	(PORTH = 0b01110111)
#define ledSegB	(PORTH = 0b01111100)
#define ledSegC	(PORTH = 0b00111001)
#define ledSegD	(PORTH = 0b01011110)
#define ledSegE	(PORTH = 0b01111001)
#define ledSegF	(PORTH = 0b01110001)
#define ledSegMinus		(PORTH = 0b01000000)
#define ledSegDOP_on	(PORTG |= (1<<4))
#define ledSegDOP_off	(PORTG &= ~(1<<4))
#define ledSegHKom_on	(PORTG |= (1<<3))
#define ledSegHKom_off	(PORTG &= ~(1<<3))

//***STRUKTUREN***

//***VARIABLEN GLOBAL***
unsigned int anzTimer3Interrupts_pv;

//***FUNKTIONSPROTOTYPEN***
void elob7seg_output(void);
void elob7seg_setWerte(unsigned char stelle);
void elob7seg_setSonderzeichen(unsigned char stelle);
void timer3_init(void);

//***INTERRUPTS***
ISR(TIMER3_OVF_vect)
{
		elob7seg_output();
}

/**
 * Initialisiert die 7Seg anzeige inklusive Timer
 */
void elob7seg_init(void)
{
	timer3_init();
}

/**
 * Füllt eine INT in das String "elob7seg[4]" ab
 * @param: int zahl, -999...9999
 * @return: void
 */
void elob7seg_intToString(int zahl)
{
	int zahl_save = zahl;
	if (zahl < -999 || zahl > 9999)
	{
		elob7seg_werte[0] = '-';
		elob7seg_werte[1] = '-';
		elob7seg_werte[2] = '-';
		elob7seg_werte[3] = '-';
	} 
	else
	{
		elob7seg_werte[0] = (abs(zahl) % 10 + 48);
		if(abs(zahl_save) >= 10)
		{
			zahl = abs(zahl) / 10;
			elob7seg_werte[1] = (abs(zahl) % 10 + 48);
			if (abs(zahl_save) >= 100)
			{
				zahl /= 10;
				elob7seg_werte[2] = (abs(zahl) % 10 + 48);
				if (abs(zahl_save) >= 1000)
				{
					zahl /= 10;
					elob7seg_werte[3] = (abs(zahl) % 10 + 48);
				}
				else
					elob7seg_werte[3] = 0;
			}
			else
				elob7seg_werte[2] = 0;
			
		}
		else
			elob7seg_werte[1] = 0;
			
			
		if (zahl_save < 0)
			elob7seg_werte[3] = '-';
	}
}

/**
 * Gibt Werte auf 7Seg aus
 * wird von Interrupt aufgerufen
 * multiplext selbständig
 * @param: strings müssen korrekt ausgefüllt sein!!!
 */
void elob7seg_output(void)
{
	//init
	DDRG &= ~(0b11111);	//Clear all
	DDRG |= 0x03;	//Output Hochkomma/Grad
	DDRG |= (3<<2);	//Multiplex, Enable
	DDRH = 0xFF;	//Output 7Seg
	
	static unsigned char stelle;
	stelle++;
	if(stelle > 3)
		stelle = 0;
	
	en7seg_off;
	datMuliplexer_set(stelle);
	elob7seg_setWerte(stelle);
	elob7seg_setSonderzeichen(stelle);
	en7seg_on;
}

/**
 * initialisiert timer 3
 */
void timer3_init(void)
{
	TCCR3B = 0b01;	//ClockDivid 1
	TIMSK3 = 0b1;	//overflow Interrupt Enable
}

/**
* setzt die Zeichen
*/
void elob7seg_setWerte(unsigned char stelle)
{
	switch (elob7seg_werte[stelle])
	{
		case '0':
			ledSeg0;
			break;
		case '1':
			ledSeg1;
			break;
		case '2':
			ledSeg2;
			break;
		case '3':
			ledSeg3;
			break;
		case '4':
			ledSeg4;
			break;
		case '5':
			ledSeg5;
			break;
		case '6':
			ledSeg6;
			break;
		case '7':
			ledSeg7;
			break;
		case '8':
			ledSeg8;
			break;
		case '9':
			ledSeg9;
			break;
		case 'A':
			ledSegA;
			break;
		case 'B':
			ledSegB;
			break;
		case 'C':
			ledSegC;
			break;
		case  'D':
			ledSegD;
			break;
		case 'E':
			ledSegE;
			break;
		case 'F':
			ledSegF;
			break;
		case '-':
			ledSegMinus;
			break;
		default:
			ledSeg_off;
			break;
	}
}

/**
 * setzt sämtiliche Sonderzeichen
 */
void elob7seg_setSonderzeichen(unsigned char stelle)
{
	if (elob7seg_sonderzeichen[ELOB7SEG_STRINGPOS_HKOMMA])
	{
		ledSegHKom_on;
	}
	else
	{
		ledSegHKom_off;
	}
	
	if (elob7seg_sonderzeichen[ELOB7SEG_STRINGPOS_DOPPPUNKT])
	{
		ledSegDOP_on;
	}
	else
	{
		ledSegDOP_off;
	}
	
	if (stelle == elob7seg_sonderzeichen[ELOB7SEG_STRINGPOS_KOMMA] && stelle > 0)	//komma am schluss ist überflüssig
	{
		komma_on;
	}
	else
	{
		komma_off;
	}
}