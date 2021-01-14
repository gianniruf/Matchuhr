/*
 * tastenmatrix.h
 *
 * Created: 23.03.2020 19:13:01
 *  Author: rufg
 */ 


#ifndef TASTENMATRIX_H_
#define TASTENMATRIX_H_


//***INCLUDES***

//***DEFINES***
//***status***
#define matrix_back		(zeile0.old & (1<<4))
#define matrix_0		(zeile0.old & (1<<5))
#define matrix_enter	(zeile0.old & (1<<6))

#define matrix_1		(zeile1.old & (1<<4))
#define matrix_2		(zeile1.old & (1<<5))
#define matrix_3		(zeile1.old & (1<<6))

#define matrix_4		(zeile2.old & (1<<4))
#define matrix_5		(zeile2.old & (1<<5))
#define matrix_6		(zeile2.old & (1<<6))

#define matrix_7		(zeile3.old & (1<<4))
#define matrix_8		(zeile3.old & (1<<5))
#define matrix_9		(zeile3.old & (1<<6))

//***flanken pos***
#define matrix_back_pos		(zeile0.flanke_pos & (1<<4))
#define matrix_0_pos		(zeile0.flanke_pos & (1<<5))
#define matrix_enter_pos	(zeile0.flanke_pos & (1<<6))

#define matrix_1_pos		(zeile1.flanke_pos & (1<<4))
#define matrix_2_pos		(zeile1.flanke_pos & (1<<5))
#define matrix_3_pos		(zeile1.flanke_pos & (1<<6))

#define matrix_4_pos		(zeile2.flanke_pos & (1<<4))
#define matrix_5_pos		(zeile2.flanke_pos & (1<<5))
#define matrix_6_pos		(zeile2.flanke_pos & (1<<6))

#define matrix_7_pos		(zeile3.flanke_pos & (1<<4))
#define matrix_8_pos		(zeile3.flanke_pos & (1<<5))
#define matrix_9_pos		(zeile3.flanke_pos & (1<<6))

//***flanken neg***
#define matrix_back_neg		(zeile0.flanke_neg & (1<<4))
#define matrix_0_neg		(zeile0.flanke_neg & (1<<5))
#define matrix_enter_neg	(zeile0.flanke_neg & (1<<6))

#define matrix_1_neg		(zeile1.flanke_neg & (1<<4))
#define matrix_2_neg		(zeile1.flanke_neg & (1<<5))
#define matrix_3_neg		(zeile1.flanke_neg & (1<<6))

#define matrix_4_neg		(zeile2.flanke_neg & (1<<4))
#define matrix_5_neg		(zeile2.flanke_neg & (1<<5))
#define matrix_6_neg		(zeile2.flanke_neg & (1<<6))

#define matrix_7_neg		(zeile3.flanke_neg & (1<<4))
#define matrix_8_neg		(zeile3.flanke_neg & (1<<5))
#define matrix_9_neg		(zeile3.flanke_neg & (1<<6))

//***auswahl für Umrechnung***
#define MATRIX_SEL_STATE	0
#define MATRIX_SEL_POS		1
#define MATRIX_SEL_NEG		2

//***STRUKTUREN***
struct taster_matrix{
		unsigned char flanke_pos;	//pos. flanke
		unsigned char flanke_neg;	//neg. flanke
		unsigned char old;			//zustand vom letzten mal
		unsigned char multiplex;	//0..3
};

//***VARIABLEN GLOBAL***
struct taster_matrix zeile0;
struct taster_matrix zeile1;
struct taster_matrix zeile2;
struct taster_matrix zeile3;

//***FUNKTIONSPROTOTYPEN***

/**
 * berechnet Zahl aus bisheriger, mit Flanken
 * @param	zahl	bisherige Zahl
 * @param	posNeg	auf pos/neg Flanke, gem. DEFINEs
 * @return	zahl	bisherige Zahl neu berechnet
 */
unsigned int tastenmatrix_matrixToInt(unsigned int , unsigned char);

/**
 * Erkennt flanken/Zustände an Tastenmatrix
 *@param	void
 *@return	void
 */
void flankenerkennung_matrix(void);

/**
 * initialisiert matrix
 * @param	void
 * @return	void
 */
void tastenmatix_init(void);
//***INTERRUPTS***


#endif /* TASTENMATRIX_H_ */