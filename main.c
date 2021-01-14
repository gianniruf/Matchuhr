/*
 * Matchuhr_V3.c
 *
 * Created: 2-Apr-2020
 * Author : rufg
 * 
 * Version3 von Matchuhr
 *
 * Neue Funktionen:
 *	- Anz Abschnitte, Dauer der Abschnitte einstellbar (und somit Variabel)
 */ 

//***INCLUDES***
#include <avr/io.h>
#define F_CPU 16000000	//16MHz
#include <avr/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "elobLcd.h"
#include "elobRGB.h"
#include "elob7SegV2.h"
#include "elobGPIO.h"
#include "tastenmatrix.h"
#include "adc.h"

//***DEFINES***
#define T_STRAFE5	500	//5min
#define T_STRAFE2	200	//2min
#define T_TIMEOUT	30	//30s
#define T_DELAY_TASTERUMKEHRUNG	1000	//1s
#define T_SIRENE_LANG	80	//0.8s
#define T_SIRENE_KURZ	30	//0.3s
#define audSirene_on	(PORTD |= (1<<7))
#define audSirene_off	(PORTD &= ~(1<<7))

enum abschnitte{ABSCHNITT = 0, PAUSE, OT, PAUSEOT, SO, ENDE, TIMEOUT};

//***STRUKTUREN***

//***VARIABLEN GLOBAL***
unsigned char T_ABSCHNITTE = 0;	//ganze positive Zahlen, 1..9
unsigned int T_ABSCHNITTSZEIT = 0;	//MMSS
unsigned int T_PAUSE = 0;		//MMSS
unsigned int T_OVERTIME = 0;	//MMSS
unsigned int T_PAUSE_OVERTIME = 0;	//MMSS
unsigned char OVERTIME_REQUESTET;	// 1 = Overtime requested (inkl. Shootout)
unsigned int anzTimer2Interrupts_pv;
unsigned char eineSekunde;		//1 = eine Sekunde
unsigned char zehnMilisekunden;	//1=Zehn Milisekunden
unsigned char endeAbschnitt = 0;	//Ende des Abschnittes

//***FUNKTIONSPROTOTYPEN***

/**
 * Initialisiert das Ganze 
 * !!! inkl. Zeiten --> lange Verweildauer (bis init_zeiten abgeschlossen)!!!!
 */
void init_system(void);


void timer2_init(void);

/**
 * Initialisiert alle Zeiten (Spielzeit, anzAbschnitte, ...)
 */
void init_zeiten(void);

/**
 * Berechnet die Zeiten neu
 * @param: uInt bisherige Zeit, MMSS
 * @return:	uInt neue Zeit, MMSS
 */
unsigned int zeitBerechnung(unsigned int zeit_old);

/**
 * setzt/löscht Werte von, Toren und Strafen
 * @param: pointer Tore H
 * @param: pointer Tore G
 * @param: pointer Strafzeit H
 * @param: pointer Strafzeit G
 */
void tasterauswertung(int* toreH, int* toreG, int* zeitStrafeH, int* zeitStrafeG);

/**
 * Organisiert das Shootout
 * selbständige Tasterauswertung
 * selbständige Ausgabe
 * @param:	pointer auf anz. Tore H
 * @param:	pointer auf anz. Tore G
 * @param:	erster Aufruf, 1 = erster aufruf
 */
void shootout(int* toreH, int*toreG, unsigned char ersterAufruf);

//***INTERRUPTS***
ISR(TIMER2_COMPA_vect)
{
	anzTimer2Interrupts_pv++;
	if (anzTimer2Interrupts_pv > 1000)
	{
		anzTimer2Interrupts_pv = 0;
		eineSekunde = 1;
	}
	if(!(anzTimer2Interrupts_pv % 1))
	zehnMilisekunden = 1;
}
//=============================================
//=============================================
/**
 * MAIN-ROUTINE
 */
int main(void)
{
    //***Init***
	taster.pin = &PINJ;
	schalter.pin = &PINC;
	poti.result_MAX = 1;
	poti.result_MIN = 0;
	poti.channel = ADC_CHANNEL_POTI;
	init_system();
	elob7seg_sonderzeichen[0] = 1;	//Doppelpunkt
		
	//***Variablen lokal***
	//RGB
	//0..255 oder gem. DEFINES
	unsigned char rgb_rot = 0;
	unsigned char rgb_gruen = 0;
	unsigned char rgb_blau = 0;
	//Zeiten
	//Zeitformat MMSS
	unsigned char tAbschnitt = 1;
	unsigned int tSpielzeit;
	unsigned int tSpielLaeuft = 0;	//1 = zeit läuft
	unsigned int tStrafeH = 0;
	unsigned int tStrafeG = 0;
	unsigned int tSpielzeit_old;	//speicherung der Zeit
	//Zustand
	unsigned char neuerAbschnitt = 1;	//1=neuer Abschnitt
	unsigned char status = ABSCHNITT;	//gem. ENUM "abschnitte"
	unsigned char status_old = 0;		//alter Zustand
	//Spielstand
	unsigned int toreH = 0;	//anz. Tore
	unsigned int toreG = 0;
	//diverses
	unsigned int puffer = 0;	//leerer Speicher
	unsigned char audSirene = 0;
	
	//Initialisierung beendet
	audSirene = T_SIRENE_KURZ;
	
	//***SUPERLOOP***
	while(1)
	{
		//**Inputs**
		if(zehnMilisekunden)
		{
			zehnMilisekunden = 0;
			flankenerkennung_digi(&taster);
			if(status != SO)
				tasterauswertung(&toreH, &toreG, &tStrafeH, &tStrafeG);
			if (tst0_pos)
				tSpielLaeuft ^= 1;	//toggle
			if(tst0_pos && neuerAbschnitt)
			{
				neuerAbschnitt = 0;
				anzTimer2Interrupts_pv = 999;
			}
			if (tst7_pos)
			{
				status_old = status;
				status = TIMEOUT;
				tSpielzeit_old = tSpielzeit;
				tSpielzeit = T_TIMEOUT;
				tSpielLaeuft = 1;
			}
			elob7seg_intToString(tSpielzeit);
			if(audSirene)
			{
				audSirene--;
				audSirene_on;
			}
			else
				audSirene_off;
		}
		//Korrektur der Anzeige, damit null auch angezeigt (Stelle 0..2)
		for (int i = 0; i < 3; i++)
		{
			if(elob7seg_werte[i] == 0)
				elob7seg_werte[i] = '0';
		}
		if (eineSekunde && (status != ENDE && status != SO ))
		{
			eineSekunde = 0;
			if(tSpielLaeuft)
				tSpielzeit = zeitBerechnung(tSpielzeit);
			
			
			elobLcd_clearDisplay();
			printf("H:%i", toreH);
			elobLcd_cursor_toXY(1,7);
			if((status == ABSCHNITT) || (status == PAUSE) ||(status == PAUSEOT))
				printf("%i", tAbschnitt);
			if(status == OT)
				printf("+");
			if(status == SO)
				printf("x");
			elobLcd_cursor_toXY(1,12);
			printf("G:%i", toreG);
			
			puffer = endeAbschnitt;
	
			if (tStrafeH && (status != ENDE) && (status != SO))
			{
				if(tSpielLaeuft && ((status == ABSCHNITT) || (status == OT)))
					tStrafeH = zeitBerechnung(tStrafeH);
				elobLcd_cursor_toXY(2,0);
				printf("%i:%i%i", tStrafeH/100, (tStrafeH/10)%10, tStrafeH %10);
			}
			if (tStrafeG && (status != ENDE) && (status != SO))
			{
				if(tSpielLaeuft && ((status == ABSCHNITT) || (status == OT)))
					tStrafeG = zeitBerechnung(tStrafeG);
				elobLcd_cursor_toXY(2,12);
				printf("%i:%i%i", tStrafeG/100, (tStrafeG/10)%10, tStrafeG %10);
			}
			
			endeAbschnitt = puffer;
		}
		
		//**State-Machine**
		switch(status)
		{
			case ABSCHNITT:
				if (neuerAbschnitt)
				{
					tSpielzeit = T_ABSCHNITTSZEIT;
					tSpielLaeuft = 0;
				}
				//RGB
				if (tSpielLaeuft)
				{
					rgb_gruen = RGB_HELL;
					rgb_blau = rgb_rot = RGB_DUKEL;
				} 
				else
				{
					rgb_rot = RGB_HELL;
					rgb_blau = rgb_gruen = RGB_DUKEL;
				}
				//Transition
				if (endeAbschnitt)
				{
					neuerAbschnitt = 1;
					endeAbschnitt = 0;
					if (tAbschnitt < T_ABSCHNITTE)
						status = PAUSE;
					if (tAbschnitt >= T_ABSCHNITTE)
					{
						if(!OVERTIME_REQUESTET)
							status = ENDE;
						if(OVERTIME_REQUESTET && (toreG == toreH))
							status = PAUSEOT;
						if(OVERTIME_REQUESTET && (toreH != toreG))
							status = ENDE;
					}
					audSirene = T_SIRENE_LANG;
				}
				break;
				
			case PAUSE:
				if (neuerAbschnitt)
				{
					tSpielzeit = T_PAUSE;
					tSpielLaeuft = 1;
					neuerAbschnitt = 0;
				}
				//RGB
				rgb_blau = RGB_HELL;
				rgb_rot = rgb_gruen = RGB_DUKEL;
				//Transition
				if (endeAbschnitt)
				{
					neuerAbschnitt = 1;
					endeAbschnitt = 0;
					tAbschnitt++;
					status = ABSCHNITT;
					audSirene = T_SIRENE_KURZ;
				}
				break;
				
			case OT:
				if (neuerAbschnitt)
				{
					tSpielzeit = T_OVERTIME;
					tSpielLaeuft = 0;
				}
				//RGB
				if (tSpielLaeuft)
				{
					rgb_gruen = RGB_HELL;
					rgb_blau = rgb_rot = RGB_DUKEL;
				}
				else
				{
					rgb_rot = RGB_HELL;
					rgb_blau = rgb_gruen = RGB_DUKEL;
				}
				//Transition
				if (endeAbschnitt || (toreG != toreH))
				{
					status_old = OT;
					neuerAbschnitt = 1;
					endeAbschnitt = 0;
				
					if(toreH != toreG)
						status = ENDE;
					else
						status = SO;
					audSirene = T_SIRENE_LANG;
				}
				break;
				
			case PAUSEOT:
				if (neuerAbschnitt)
				{
					tSpielzeit = T_PAUSE_OVERTIME;
					tSpielLaeuft = 1;
					neuerAbschnitt = 0;
				}
				//RGB
				rgb_blau = RGB_HELL;
				rgb_rot = rgb_gruen = RGB_DUKEL;
				//Transition
				if (endeAbschnitt)
				{
					neuerAbschnitt = 1;
					endeAbschnitt = 0;
					tAbschnitt;
					status = OT;
					audSirene = T_SIRENE_KURZ;
				}
				break;
				
			case SO:
				if (neuerAbschnitt)
				{
					shootout(&toreH, &toreG,1);
					neuerAbschnitt = 0;
					elobLcd_clearDisplay();
					printf("H:%i", toreH);
					elobLcd_cursor_toXY(1,7);
					printf("x");
					elobLcd_cursor_toXY(1,12);
					printf("G:%i", toreG);
				}
				rgb_rot = RGB_HELL;
				rgb_gruen = RGB_HELL;
				rgb_blau = RGB_DUKEL;
				shootout(&toreH, &toreG, 0);
				if (toreH != toreG)
				{
					status = ENDE;
					neuerAbschnitt = 1;
					status_old = SO;
				}
				break;
				
			case TIMEOUT:
				rgb_gruen = RGB_HELL;
				rgb_blau = RGB_MITTEL;
				rgb_rot = RGB_DUKEL;
				//Transition
				if (endeAbschnitt)
				{
					endeAbschnitt = 0;
					status = status_old;
					tSpielzeit = tSpielzeit_old;
					tSpielLaeuft = 0;
					audSirene = T_SIRENE_KURZ;
				}
				break;
				
			case ENDE:
				if (neuerAbschnitt)
				{
					neuerAbschnitt = 0;
					tSpielzeit = 0;
					tSpielLaeuft = 1;
					elobLcd_clearDisplay();
					printf("H:%i", toreH);
					elobLcd_cursor_toXY(1,7);
					if(status_old == OT)
						printf("nV");
					if(status_old == SO)
						printf("nP");
					elobLcd_cursor_toXY(1,12);
					printf("G:%i", toreG);
					elobLcd_cursor_toXY(2,0);
					printf("End of the Game");
					_delay_us(1);
				}
				rgb_rot = rgb_blau = RGB_HELL;
				rgb_gruen = RGB_DUKEL;
				break;
				
			default:
				elobLcd_clearDisplay();
				printf("ERROR\nSTATE-MACHINE");
				rgb_rot = RGB_HELL;
				rgb_gruen = RGB_MITTEL;
				rgb_blau = RGB_DUKEL;
				break;
			
		}
		//**Output RGB**
		//Helligkeit via Poti
		rgb_rot *= poti.result;
		rgb_gruen *= poti.result;
		rgb_blau *= poti.result;
		elobRGB_setColor(rgb_rot, rgb_gruen, rgb_blau);
	}
	
}

//=================================================
//=================================================

//***Funktionen***
/**
 * Initialisiert das Ganze
 */
void init_system(void)
{
	sei();
	timer2_init();
	tastenmatix_init();
	elobDigiIO_init();
	elobRGB_init();
	elobRGB_setColor(RGB_SCHWACH, RGB_SCHWACH, RGB_SCHWACH);
	elobLcd_init();
	adc_init(0, &poti, 0b111);
	adc_start();
	DDRD |= (1<<7);	//Buzzer Output
	lcdBacklight_on;
	
	init_zeiten();
	
	elobLcd_cursor_off();
	elob7seg_init();
}

/**
 * Initialisiert alle Zeiten (Spielzeit, anzAbschnitte, ...)
 */
void init_zeiten(void)
{
	unsigned int time_old;
	
	printf("Matchuhr\nUnihockey");
	_delay_ms(2000);
	elobLcd_clearDisplay();
	
	//***Anzahl Abschnitte***
	printf("max. Abschnitte:\n9");
	_delay_ms(1500);
	elobLcd_clearDisplay();
	elobLcd_cursor_toXY(1,0);
	printf("Anz. Abschnitte");
	elobLcd_cursor_toXY(2,0);
	do 
	{
		time_old = T_ABSCHNITTE;
		flankenerkennung_matrix();
		T_ABSCHNITTE = tastenmatrix_matrixToInt(T_ABSCHNITTE, MATRIX_SEL_POS);
		if(time_old != T_ABSCHNITTE)
		{
			elobLcd_cursor_toXY(2,0);
			printf("%i", T_ABSCHNITTE);
		}
		_delay_ms(10);
	} while (!matrix_enter_pos);
	
	//***Abschnittszeit***
	elobLcd_clearDisplay();
	printf("maxZeit: 99min\nZeitformat: MMSS");
	_delay_ms(2500);
	elobLcd_clearDisplay();
	elobLcd_cursor_toXY(1,0);
	printf("Dauer pro Absch.");
	elobLcd_cursor_toXY(2,0);
	do
	{
		time_old = T_ABSCHNITTSZEIT;
		flankenerkennung_matrix();
		T_ABSCHNITTSZEIT = tastenmatrix_matrixToInt(T_ABSCHNITTSZEIT, MATRIX_SEL_POS);
		if(time_old != T_ABSCHNITTSZEIT)
		{
			elobLcd_cursor_toXY(2,0);
			printf("%i", T_ABSCHNITTSZEIT);
		}
		_delay_ms(10);
	} while (!matrix_enter_pos);
	
	//***Pausenzeit***
	elobLcd_clearDisplay();
	elobLcd_cursor_toXY(1,0);
	printf("Dauer pro Pause");
	elobLcd_cursor_toXY(2,0);
	do
	{
		time_old = T_PAUSE;
		flankenerkennung_matrix();
		T_PAUSE = tastenmatrix_matrixToInt(T_PAUSE, MATRIX_SEL_POS);
		if(time_old != T_PAUSE)
		{
			elobLcd_cursor_toXY(2,0);
			printf("%i", T_PAUSE);
		}
		_delay_ms(10);
	} while (!matrix_enter_pos);
	
	//***OT?***
	elobLcd_clearDisplay();
	elobLcd_cursor_toXY(1,0);
	printf("mit OT (und SO)?");
	elobLcd_cursor_toXY(2,0);
	printf("N");
	elobLcd_cursor_toXY(2,2);
	printf("J");
	elobLcd_cursor_toXY(2,0);
	do 
	{
		flankenerkennung_matrix();
		if(matrix_4_pos)
		{
			OVERTIME_REQUESTET = 0;
			elobLcd_cursor_toXY(2,0);
		}
		if(matrix_6_pos)
		{
			OVERTIME_REQUESTET = 1;
			elobLcd_cursor_toXY(2,2);
		}
		_delay_ms(10);
	} while (!matrix_enter_pos);
	
	if (OVERTIME_REQUESTET)
	{	
		//**PauseOT zeit**
		elobLcd_clearDisplay();
		elobLcd_cursor_toXY(1,0);
		printf("Dauer Pause OT");
		elobLcd_cursor_toXY(2,0);
		do
		{
			time_old = T_PAUSE_OVERTIME;
			flankenerkennung_matrix();
			T_PAUSE_OVERTIME = tastenmatrix_matrixToInt(T_PAUSE_OVERTIME, MATRIX_SEL_POS);
			if(time_old != T_PAUSE_OVERTIME)
			{
				elobLcd_cursor_toXY(2,0);
				printf("%i", T_PAUSE_OVERTIME);
			}
			_delay_ms(10);
		} while (!matrix_enter_pos);
		
		//**Zeit OT**
		elobLcd_clearDisplay();
		elobLcd_cursor_toXY(1,0);
		printf("Dauer OT");
		elobLcd_cursor_toXY(2,0);
		do
		{
			time_old = T_OVERTIME;
			flankenerkennung_matrix();
			T_OVERTIME = tastenmatrix_matrixToInt(T_OVERTIME, MATRIX_SEL_POS);
			if(time_old != T_OVERTIME)
			{
				elobLcd_cursor_toXY(2,0);
				printf("%i", T_OVERTIME);
			}
			_delay_ms(10);
		} while (!matrix_enter_pos);
	}
	elobLcd_clearDisplay();
}

/**
 * Initialisierung timer 2
 */
void timer2_init(void)
{
	TCCR2A |= (2);			//Waveform Generation Mode, Mode 2 = CTC
	TCCR2B |= (0<<2);		//" , nur Vollständigkeitshalber
	TCCR2B |= (1<<7);		//force Output Compare A, FOC2A
	TCCR2B |= (0b100);		//Clock Select, CS22:0, 100 = clk/64
	
	OCR2A |= 249;		//Output Compare Register A,  (ausgerechnet, 1ms)
	
	TIMSK2 |= (1<<1);	//Compare Match A Interrupt Enable, OCIE2A
}

/**
 * Berechnet die Zeiten neu
 * @param: uInt bisherige Zeit, MMSS
 * @return:	uInt neue Zeit, MMSS
 */
unsigned int zeitBerechnung(unsigned int zeit_old)
{
	unsigned int zeit_neu = 0;
	unsigned int sekunden;
	unsigned int zSekunden;
	unsigned int minuten;
	unsigned int zMinuten;
	
	sekunden = zeit_old % 10;
	zeit_old /= 10;
	zSekunden = zeit_old % 10;
	zeit_old /= 10;
	minuten = zeit_old % 10;
	zeit_old /= 10;
	zMinuten = zeit_old % 10;
	
	if (sekunden || zSekunden || minuten || zMinuten)
	{
		if (sekunden)
			sekunden--;
		else
		{
			sekunden = 9;
			if (zSekunden)
				zSekunden--;
			else
			{
				zSekunden = 5;
				if (minuten)
				minuten--;
				else
				{
					minuten = 9;
					if (zMinuten)
					{
						zMinuten--;
					}
				}
			}
		}
	}
	else
	endeAbschnitt = 1;
	
	zeit_neu = zMinuten * 1000;
	zeit_neu += minuten * 100;
	zeit_neu += zSekunden * 10;
	zeit_neu += sekunden;
	
	return zeit_neu;
}

/**
 * setzt/löscht Werte von, Toren und Strafen
 * @param: pointer Tore H
 * @param: pointer Tore G
 * @param: pointer Strafzeit H
 * @param: pointer Strafzeit G
 */
void tasterauswertung(int* toreH, int* toreG, int* zeitStrafeH, int* zeitStrafeG)
{
	//*** Variablen Status taster***
	static unsigned int counter_tst_T_H;
	static unsigned int counter_tst_T_G;
	static unsigned int counter_tst_2_H;
	static unsigned int counter_tst_2_G;
	static unsigned int counter_tst_5_H;
	static unsigned int counter_tst_5_G;
	
	//***Counters***
	if(counter_tst_T_H && counter_tst_T_H < T_DELAY_TASTERUMKEHRUNG)
		counter_tst_T_H++;
	if(counter_tst_T_G && counter_tst_T_G < T_DELAY_TASTERUMKEHRUNG)
		counter_tst_T_G++;
	if(counter_tst_2_H && counter_tst_2_H < T_DELAY_TASTERUMKEHRUNG)
		counter_tst_2_H++;
	if(counter_tst_2_G && counter_tst_2_G < T_DELAY_TASTERUMKEHRUNG)
		counter_tst_2_G++;
	if(counter_tst_5_H && counter_tst_5_H < T_DELAY_TASTERUMKEHRUNG)
		counter_tst_5_H++;
	if(counter_tst_5_G && counter_tst_5_G < T_DELAY_TASTERUMKEHRUNG)
		counter_tst_5_G++;
	
	//***Tore H***
	if(tst2_pos)
		counter_tst_T_H = 1;
	if(tst2_neg)
	{
		if (counter_tst_T_H >= (T_DELAY_TASTERUMKEHRUNG - 1))
			(*toreH)--;
		else
			(*toreH)++;
		counter_tst_T_H = 0;
	}
		
	//***Tore G***
	if(tst1_pos)
		counter_tst_T_G = 1;
	if(tst1_neg)
	{
		if (counter_tst_T_G >= (T_DELAY_TASTERUMKEHRUNG - 1))
			(*toreG)--;
		else
			(*toreG)++;
		counter_tst_T_G = 0;
	}
	
	//***Strafen***
	//**Strafe 2 H**
	if(tst4_pos)
		counter_tst_2_H = 1;
	if(tst4_neg)
	{
		if (counter_tst_2_H >= (T_DELAY_TASTERUMKEHRUNG - 1))
			*zeitStrafeH = 0;
		else
			*zeitStrafeH = T_STRAFE2;
		counter_tst_2_H = 0;
	}
	//**Strafe 2 G**
	if(tst3_pos)
		counter_tst_2_G = 1;
	if(tst3_neg)
	{
		if (counter_tst_2_G >= (T_DELAY_TASTERUMKEHRUNG - 1))
			*zeitStrafeG = 0;
		else
			*zeitStrafeG = T_STRAFE2;
		counter_tst_2_G = 0;
	}
	//**Strafe 5 H**
	if(tst6_pos)
		counter_tst_5_H = 1;
	if(tst6_neg)
	{
		if (counter_tst_5_H >= (T_DELAY_TASTERUMKEHRUNG - 1))
			*zeitStrafeH = 0;
		else
			*zeitStrafeH = T_STRAFE5;
		counter_tst_5_H = 0;
	}
	//**Strafe 5 G**
	if(tst5_pos)
		counter_tst_5_G = 1;
	if(tst5_neg)
	{
		if (counter_tst_5_G >= (T_DELAY_TASTERUMKEHRUNG - 1))
			*zeitStrafeG = 0;
		else
			*zeitStrafeG = T_STRAFE5;
		counter_tst_5_G = 0;
	}
}

/**
* Organisiert das Shootout
* selbständige Tasterauswertung
* selbständige Ausgabe
* @param:	pointer auf anz. Tore H
* @param:	pointer auf anz. Tore G
* @param:	erster Aufruf, 1 = erster aufruf
*/
void shootout(int* toreH, int*toreG, unsigned char ersterAufruf)
{
	static unsigned char anzPenaltysH;
	static unsigned char anzPenaltysG;
	static unsigned char anzPenaltysTotal;
	static const unsigned char lcd_offset = 7;
	
	if(ersterAufruf)
	{
		anzPenaltysTotal = anzPenaltysG = anzPenaltysH = 0;
	}
	if(tst1_pos)
		(*toreG)++;
	if(tst2_pos)
		(*toreH)++;
	if(tst3_pos||tst4_pos||tst5_pos||tst6_pos)
		anzPenaltysTotal++;
		
	if ((anzPenaltysTotal > 10) && (anzPenaltysTotal % 10))
	{
		anzPenaltysTotal = 0;
		anzPenaltysH = 0;
		anzPenaltysG = 0;
		elobLcd_clearDisplay();
		printf("H:%i", *toreH);
		elobLcd_cursor_toXY(1,7);
		printf("x");
		elobLcd_cursor_toXY(1,12);
		printf("G:%i", *toreG);
	}
	if(tst3_pos)	//gast hat verschossen
	{
		elobLcd_cursor_toXY(2, lcd_offset + anzPenaltysG);
		printf("X");
		anzPenaltysG++;
	}
	if(tst4_pos)	//gast hat getroffen
	{
		elobLcd_cursor_toXY(2, lcd_offset + anzPenaltysG);
		printf("O");
		anzPenaltysG++;
	}
	if(tst5_pos)	//heim hat verschossen
	{
		elobLcd_cursor_toXY(2, anzPenaltysH);
		printf("X");
		anzPenaltysH++;
	}
	if(tst6_pos)	//heim hat getroffen
	{
		elobLcd_cursor_toXY(2, anzPenaltysH);
		printf("O");
		anzPenaltysH++;
		//for Test
		_delay_ms(10);
	}
}