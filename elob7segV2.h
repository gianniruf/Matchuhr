/*
 * elob7segV2.h
 *
 * Created: 02.04.2020 14:48:15
 *  Author: rufg
 */ 


#ifndef ELOB7SEGV2_H_
#define ELOB7SEGV2_H_

//***INCLUDES***

//***DEFINES***
#define ELOB7SEG_STRINGPOS_KOMMA 2
#define ELOB7SEG_STRINGPOS_HKOMMA 1
#define ELOB7SEG_STRINGPOS_DOPPPUNKT 0

//***STRUKTUREN***

//***VARIABLEN GLOBAL***
unsigned char elob7seg_werte[4];	//Diese Zeichen (CHAR)werden nacheinander ausgegeben.
unsigned char elob7seg_sonderzeichen[3];	//stelle Komma(0..3)|Hochkomma (1=leuchter)|doppelpunkt (1=leuchtet)

//***FUNKTIONSPROTOTYPEN***
/**
 * Initialisiert die 7Seg anzeige inklusive Timer
 */
void elob7seg_init(void);

/**
 * Füllt eine INT in das String "elob7seg[4]" ab
 * @param: int zahl, -999...9999
 * @return: void
 */
void elob7seg_intToString(int zahl); 

#endif /* ELOB7SEGV2_H_ */