/*
 * tastenmatrix-7Seg.c
 *
 * @Author:		Gianni Ruf
 * @Version:	1.0
 **********************************
 * Datum			Version	Name			Änderung
 * 9. Sep 2019		1.0		Gianni Ruf		Erstellung
 */ 

// ***INCLUDES***
#include "tastenmatrix-7Seg.h"
#include <avr/io.h>

// ***DEFINES***
#define en7seg_off	(PORTG &= ~(1<<2))
#define en7seg_on	(PORTG |= (1<<2))
#define datMuliplexer_set	(PORTG = (stelle<<0))
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
#define ledSegMinus		(PORTH = 0b01000000)
#define ledSegDOP_on	(PORTG |= (1<<4))
#define ledSegDOP_off	(PORTG &= ~(1<<4))
#define ledSegHKom_on	(PORTG |= (1<<3))
#define ledSegHKom_off	(PORTG &= ~(1<<3))



#define TM1	(PORTD & (1<<4))
#define TM2	(PORTD & (1<<5))
#define TM3	(PORTD & (1<<6))


unsigned char zifferextraktion(unsigned int zahl, unsigned char position);

/**
* Gibt zahlen auf der 4Fach 7Segmentanzeige aus
* @param: ziffer 0...9
* @param: hochkomma 0,1
* @param: doppelpunkt 0,1
*/
void ausgabe7seg(int zahl, unsigned char hKom, unsigned char DoppP, unsigned char komma)
{
	en7seg_off;
	
	static unsigned char stelle;
	static unsigned char anzStellen;
	
	//entferung des vordersten Null
	if(zahl < 1000)
		anzStellen = 3;
	else
		anzStellen = 4;	
	
	stelle++;
	if (stelle >= anzStellen)
	{
		stelle = 0;
	}
	
	//NEG ZAHL
	if(zahl < 0)
	{
		zahl /= 10;
		komma -= 1;
	}
		
	unsigned char ziffer;	
		
	if ((zahl > 0) || (stelle < 3))
	{
		ziffer = zifferextraktion(zahl, stelle);
	}
	
		
	datMuliplexer_set;
	
	switch(ziffer)
	{
		case 0:
			ledSeg0;
			break;
		case 1:
			ledSeg1;
			break;
		case 2:
			ledSeg2;
			break;
		case 3:
			ledSeg3;
			break;
		case 4:
			ledSeg4;
			break;
		case 5:
			ledSeg5;
			break;
		case 6:
			ledSeg6;
			break;
		case 7:
			ledSeg7;
			break;
		case 8:
			ledSeg8;
			break;
		case 9:
			ledSeg9;
			break;
		default:
			ledSegMinus;
			break;
	}	
	
	if ((zahl < 0) && (stelle == 3))
	{
		ledSegMinus;
	}
		
	if (hKom)
	{
		ledSegHKom_on;
	} 
	else
	{
		ledSegHKom_off;
	}
		
	if (DoppP)
	{
		ledSegDOP_on;
	} 
	else
	{
		ledSegDOP_off;
	}
		
	if (stelle == komma && stelle > 0)	//komma am schluss ist überflüssig
	{
		komma_on;
	}
	else
	{
		komma_off;
	}
	
		
	en7seg_on;
	
}

/**
* berechnet ziffer
* @param: zahl 0...9999
* @return: ziffer 0...9
*/
unsigned char zifferextraktion(unsigned int zahl, unsigned char position)
{
	unsigned int ziffer;
	
	switch(position)
	{
		case 3:
			ziffer = zahl / 1000;
			break;
		case 2:
			ziffer = zahl / 100;
			break;
		case 1:
			ziffer = zahl / 10;
			break;
		case 0:
			ziffer = zahl / 1;
			break;
		default:
			ziffer = 8;
			break;
		
	}
	
	return (ziffer % 10);
	
}


/**
* @param: taster 0...11
*/
void eingabeTastenMatrix(void)
{
	unsigned char stelle = 0;
	datMuliplexer_set;
	tm1.seg1 = TM1;
	tm2.seg1 = TM2;
	tm3.seg1 = TM3;
	
	stelle = 1;
	datMuliplexer_set;
	tm1.seg2 = TM1;
	tm2.seg2 = TM2;
	tm3.seg2 = TM3;
	
	stelle = 2;
	datMuliplexer_set;
	tm1.seg3 = TM1;
	tm2.seg3 = TM2;
	tm3.seg3 = TM3;
	
	stelle = 3;
	datMuliplexer_set;
	tm1.seg4 = TM1;
	tm2.seg4 = TM2;
	tm3.seg4 = TM3;
}

