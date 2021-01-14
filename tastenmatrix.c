/*
 * tastenmatrix.c
 *
 * Created: 23.03.2020 19:13:35
 *  Author: rufg
 */ 

//***INCLUDES***
#include "tastenmatrix.h"
#include <avr/io.h>

//***DEFINES***
#define matrix_en	(PORTG |=(1<<2))
#define matrix_dis	(PORTG &= ~(1<<2))

//***STRUKTUREN***

//***VARIABLEN GLOBAL***

//***FUNKTIONSPROTOTYPEN***
void tastenmatrix_vorbereitung(void);
void tastenmatrix_read(struct taster_matrix *);
void tastenmatrix_setMuliplex(unsigned char wert);
unsigned int tastenmatrix_zahlberechnungPos(unsigned int zahl);
unsigned int tastenmatrix_zahlberechnungNeg(unsigned int zahl);

//***INTERRUPTS***

//***FUNKTIONEN***

/**
 * Erkennt flanken/Zustände an Tastenmatrix
 *@param	void
 *@return	void
 */
void flankenerkennung_matrix()
{
	tastenmatrix_vorbereitung();
	
	tastenmatrix_read(&zeile0);
	tastenmatrix_read(&zeile1);
	tastenmatrix_read(&zeile2);
	tastenmatrix_read(&zeile3);
}

/**
 * initialisiert matrix
 * @param	void
 * @return	void
 */
void tastenmatix_init(void)
{
	zeile0.multiplex = 0;
	zeile1.multiplex = 1;
	zeile2.multiplex = 2;
	zeile3.multiplex = 3;
}

/**
 * Bereitet Matrix für Auslesung vor
 */
void tastenmatrix_vorbereitung(void)
{
	DDRD &= (0b111<<4);	//Input
	DDRG = 0xFF;		//Output
}

/**
 * Liest Zeilenweise Matrix aus
 * @param: pointer auf Struktur von Zeile
 */
void tastenmatrix_read(struct taster_matrix * Zeile)
{
	tastenmatrix_setMuliplex(Zeile->multiplex);
	unsigned char state_zeile = 0;
	matrix_en;
	state_zeile = (PIND & (0b111<<4));	//speichert aktuellen Zustand (sicher dass keine änderung während durchlauf)
	
	(Zeile->flanke_pos) = state_zeile & (~(Zeile->old));
	(Zeile->flanke_neg) = (~(state_zeile)) & Zeile->old;
	(Zeile->old) = state_zeile;
	matrix_dis;
}

/**
 * setzt Multiplexer
 * @param: wert mulitplexwert 0..3
 */
void tastenmatrix_setMuliplex(unsigned char wert)
{
	PORTG &= ~(0b11<<0);	//setzt auf Null
	PORTG |= (wert<<0);
}

/**
 * berechnet Zahl aus bisheriger, mit Flanken
 * @param	zahl	bisherige Zahl
 * @param	posNeg	auf pos/neg Flanke, gem. DEFINEs
 * @return	zahl	bisherige Zahl neu berechnet
 */
unsigned int tastenmatrix_matrixToInt(unsigned int zahl, unsigned char posNeg)
{
	if (posNeg == MATRIX_SEL_POS)
	{
		zahl = tastenmatrix_zahlberechnungPos(zahl);
	} 
	else if(posNeg == MATRIX_SEL_NEG)
	{
		zahl = tastenmatrix_zahlberechnungNeg(zahl);
	}
	return zahl;
}

/**
 * berechnet neue Zahl wenn pos-Flanke
 * @param	zahl	bisherige Zahl
 * @return	zahl	neue Zahl
 */
unsigned int tastenmatrix_zahlberechnungPos(unsigned int zahl)
{
	if (matrix_back_pos)
		zahl /= 10;
	if(matrix_0_pos)
	{
		zahl *= 10;
		zahl += 0;
	}
	if(matrix_1_pos)
	{
		zahl *= 10;
		zahl += 1;
	}
	if(matrix_2_pos)
	{
		zahl *= 10;
		zahl += 2;
	}
	if(matrix_3_pos)
	{
		zahl *= 10;
		zahl += 3;
	}
	if(matrix_4_pos)
	{
		zahl *= 10;
		zahl += 4;
	}
	if(matrix_5_pos)
	{
		zahl *= 10;
		zahl += 5;
	}
	if(matrix_6_pos)
	{
		zahl *= 10;
		zahl += 6;
	}
	if(matrix_7_pos)
	{
		zahl *= 10;
		zahl += 7;
	}
	if(matrix_8_pos)
	{
		zahl *= 10;
		zahl += 8;
	}
	if(matrix_9_pos)
	{
		zahl *= 10;
		zahl += 9;
	}
	return zahl;
}

/**
 * berechnet neue Zahl wenn neg-Flanke
 * @param	zahl	bisherige Zahl
 * @return	zahl	neue Zahl
 */
unsigned int tastenmatrix_zahlberechnungNeg(unsigned int zahl)
{
	if (matrix_back_neg)
	zahl /= 10;
	if(matrix_0_neg)
	{
		zahl *= 10;
		zahl += 0;
	}
	if(matrix_1_neg)
	{
		zahl *= 10;
		zahl += 1;
	}
	if(matrix_2_neg)
	{
		zahl *= 10;
		zahl += 2;
	}
	if(matrix_3_neg)
	{
		zahl *= 10;
		zahl += 3;
	}
	if(matrix_4_pos)
	{
		zahl *= 10;
		zahl += 4;
	}
	if(matrix_5_neg)
	{
		zahl *= 10;
		zahl += 5;
	}
	if(matrix_6_neg)
	{
		zahl *= 10;
		zahl += 6;
	}
	if(matrix_7_neg)
	{
		zahl *= 10;
		zahl += 7;
	}
	if(matrix_8_neg)
	{
		zahl *= 10;
		zahl += 8;
	}
	if(matrix_9_neg)
	{
		zahl *= 10;
		zahl += 9;
	}
	return zahl;
}