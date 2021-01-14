/*
 * tastenmatrix-7seg.h
 *
 * @Author:		Gianni Ruf
 * @Version:	1.0
 **********************************
 * Datum			Version	Name			Änderung
 * 9. Sep 2019		1.0		Gianni Ruf		Erstellung
 */ 


#include <avr/io.h>

#ifndef TASTENMATRIX_7SEG_H_
#define TASTENMATRIX_7SEG_H_

// ***DEFINES***


// ***STRUCKTUREN***
struct tastenmatrix 
{
	unsigned char seg1;
	unsigned char seg2;
	unsigned char seg3;
	unsigned char seg4;
};

struct tastenmatrix tm1, tm2, tm3;

// ***FUNKTIONSPROTOTYPEN***
void ausgabe7seg (int, unsigned char, unsigned char, unsigned char);	//zahl, Hochkomma, doppelpunkt, stelle komma
void eingabeTastenMatrix (void);		//Taster


#endif /* TASTENMATRIX-7SEG_H_ */