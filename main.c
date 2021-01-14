/*
 * Matchuhr_v2.c
 *
 * Created: 21.12.2019 12:24:53
 * Author : rufg
 */ 

//***INCLUDES***
#include <avr/io.h>
#include <avr/interrupt.h>
#include "elobLcd.h"
#include "tastenmatrix-7Seg.h"

//***DEFINES***
//***Zeit***
#define zeitAbschnitt	2000	//20min
#define zeitPause		1000	//10min
#define zeitOT			1000	//10min
#define zeitPauseOT		500		//5min
#define zeitTimeout		30		//30sek
#define zeitStrafe2		200		//2min
#define zeitStrafe5		500		//5min

#define delay 1001

//***Taster***
#define tstStartStopp	(pinj.flanke_pos & (1<<0))
#define tstTorH_pos			(pinj.flanke_pos & (1<<2))
#define tstTorG__pos		(pinj.flanke_pos & (1<<1))
#define tstStrafe2H_pos		(pinj.flanke_pos & (1<<4))
#define tstStrafe2G_pos		(pinj.flanke_pos & (1<<3))
#define tstStrafe5H_pos		(pinj.flanke_pos & (1<<6))
#define tstStrafe5G_pos		(pinj.flanke_pos & (1<<5))
#define tstTimeout_pos		(pinj.flanke_pos & (1<<7))
#define tstTorH_neg			(pinj.flanke_neg & (1<<2))
#define tstTorG_neg			(pinj.flanke_neg & (1<<1))
#define tstStrafe2H_neg		(pinj.flanke_neg & (1<<4))
#define tstStrafe2G_neg		(pinj.flanke_neg & (1<<3))
#define tstStrafe5H_neg		(pinj.flanke_neg & (1<<6))
#define tstStrafe5G_neg		(pinj.flanke_neg & (1<<5))
#define tstTimeout_neg		(pinj.flanke_neg & (1<<7))

//***rgb***
#define rgbAusgabe		(5)

//***Diverses***
#define audHorn_on		(PORTD |= (1<<7))
#define audHorn_off		(PORTD &= ~(1<<7))
#define AUD_HORN_LANG_SP	100	//1sek
#define AUD_HORN_KURZ_SP	50	//0.5sek


//***FUNKTIONSPROTOTYPEN***
void pin_init(void);
void timer_init(void);
void flankenerkennung(void);
int zeitberechnung(int zeit_old);
void tasterauswertung(int* toreH, int* toreG, int* zeitStrafeH, int* zeitStrafeG, unsigned char drittel, unsigned char* anzPen);

//***STRUKTUREN
struct flanken
{
	unsigned char state;
	unsigned char state_old;
	unsigned char flanke_pos;
	unsigned char flanke_neg;
};
struct flanken pinj;

//***ENUMS***
enum abschnitt {ABSCH1 = 1, ABSCH2, ABSCH3, PAUSE1, PAUSE2, PAUSEOT, OT, PENALTYS, TIMEOUT, ENDE};
enum farben {DUNKEL = 0, ROT, GRUEN, ORANGE, BLAU, VIOLET, TUERKIS, WEISS};

//***VARIABLEN GLOBAL***
unsigned int anzTimerInterrupts_pv;
unsigned char eineSekunde = 0;
unsigned char zehnmilisekunde = 0;
unsigned int horn_pv = 0;
unsigned int spielzeit;		//Aktuelle Zeit
unsigned char endeAbschnitt = 0;

//***INTERRUPTS***
ISR(TIMER2_COMPA_vect)
{
	anzTimerInterrupts_pv++;
	if (anzTimerInterrupts_pv > 1000)
	{
		anzTimerInterrupts_pv = 0;
		eineSekunde = 1;
	}
	if(!(anzTimerInterrupts_pv % 1))
		zehnmilisekunde = 1;
}

/**
 * MAIN-ROUTINE
 */
int main(void)
{
	//***Initialisierung***
	pin_init();
	elobLcd_init();
	elobLcd_cursor_off();
	timer_init();
	
	sei();	//Interrupts enable
	
	lcdBacklight_on;
	
	//***Variablen lokal***
	unsigned char state = ABSCH1;		//aktueller Abschnitt
	unsigned char state_old;			//für TimeOut return
	unsigned char neuerAbschnitt = 1;	//1 = neuer Abaschnitt
	unsigned char absch;				//Anzeige
	unsigned char rgbFarbe;
	unsigned char spielLaeuft = 0;
	unsigned int spielzeit_old;
	unsigned int toreH = 0;
	unsigned int toreG = 0;
	unsigned int zeitStrafeG = 0;
	unsigned int zeitStrafeH = 0;
	unsigned char anzPenaltys;
	
	//***SUPERLOOP***
    while (1) 
    {
		if (zehnmilisekunde)
		{
			flankenerkennung();
			tasterauswertung(&toreH, &toreG, &zeitStrafeH, &zeitStrafeG, absch, &anzPenaltys);
			if(tstStartStopp)
				spielLaeuft ^= 1;
			if (tstTimeout_pos)
			{
				state_old = state;
				state = TIMEOUT;
				neuerAbschnitt = 1;
			}
			if(tstStartStopp && neuerAbschnitt)
			{	
				neuerAbschnitt = 0;
				anzTimerInterrupts_pv = 999;
			}
			zehnmilisekunde = 0;
			ausgabe7seg(spielzeit, 0, 1, 0);
			if (horn_pv)
			{
				horn_pv--;
				audHorn_on;
			}
			else
				audHorn_off;
			PORTB = rgbFarbe << rgbAusgabe;
		}
		
		//***Zeitberechnungen***
		if (eineSekunde && (state != PENALTYS) && (state != ENDE))
		{
			eineSekunde = 0;
			if(spielLaeuft)
				spielzeit = zeitberechnung(spielzeit);
			elobLcd_clearDisplay();
			elobLcd_cursor_toXY(1, 0);
			printf("H:%i", toreH);
			elobLcd_cursor_toXY(1, 7);
			printf("%c", absch);
			elobLcd_cursor_toXY(1, 12);
			printf("G:%i", toreG);
			if (zeitStrafeH && (state != PENALTYS) && (state != ENDE))
			{
				elobLcd_cursor_toXY(2, 0);
				if(spielLaeuft && ((state == ABSCH1) || (state == ABSCH2) || (state == ABSCH3) || (state == OT)))
					zeitStrafeH = zeitberechnung(zeitStrafeH);
				printf("%i:%i%i", (zeitStrafeH / 100), ((zeitStrafeH / 10) % 10), (zeitStrafeH % 10));
			}
			if (zeitStrafeG && (absch != PENALTYS) && (state != ENDE))
			{
				elobLcd_cursor_toXY(2, 12);
				if(spielLaeuft && ((state == ABSCH1) || (state == ABSCH2) || (state == ABSCH3) || (state == OT)))
					zeitStrafeG = zeitberechnung(zeitStrafeG);
				printf("%i:%i%i", (zeitStrafeG / 100), ((zeitStrafeG / 10) % 10), (zeitStrafeG % 10));
			}
		}
		
		//***Statemachine***
		switch(state)
		{
			case ABSCH1:
				if (neuerAbschnitt)
				{
					spielzeit = zeitAbschnitt;
					spielLaeuft = 0;
				}
				absch = '1';
				if(spielLaeuft)
					rgbFarbe = GRUEN;
				else
					rgbFarbe = ROT;
				if (endeAbschnitt)
				{
					state = PAUSE1;
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_LANG_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case ABSCH2:
				if (neuerAbschnitt)
				{
					spielzeit = zeitAbschnitt;
					spielLaeuft = 0;
				}
				absch = '2';
				if(spielLaeuft)
					rgbFarbe = GRUEN;
				else
					rgbFarbe = ROT;
				if (endeAbschnitt)
				{
					state = PAUSE2;
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_LANG_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case ABSCH3:
				if (neuerAbschnitt)
				{
					spielzeit = zeitAbschnitt;
					spielLaeuft = 0;
				}
				absch = '3';
				if(spielLaeuft)
					rgbFarbe = GRUEN;
				else
					rgbFarbe = ROT;
				if (endeAbschnitt)
				{
					if (toreH == toreG)
						state = PAUSEOT;
					else
					{
						state = ENDE;
						state_old = ABSCH3;
					}
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_LANG_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case OT:
				if (neuerAbschnitt)
				{
					spielzeit = zeitOT;
					spielLaeuft = 0;
				}
				absch = '+';
				if(spielLaeuft)
				rgbFarbe = GRUEN;
				else
				rgbFarbe = ROT;
				if (endeAbschnitt || (toreH != toreG))
				{
					if (toreH == toreG)
						state = PENALTYS;
					else
					{
						state = ENDE;
						state_old = OT;
					}
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_LANG_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case PAUSE1:
				if(neuerAbschnitt)
				{
					spielzeit = zeitPause;
					spielLaeuft = 1;
					neuerAbschnitt = 0;
				}
				rgbFarbe = BLAU;
				if (endeAbschnitt)
				{
					state = ABSCH2;
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_KURZ_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case PAUSE2:
				if(neuerAbschnitt)
				{
					spielzeit = zeitPause;
					spielLaeuft = 1;
					neuerAbschnitt = 0;
				}
				rgbFarbe = BLAU;
				if (endeAbschnitt)
				{
					state = ABSCH3;
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_KURZ_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case PAUSEOT:
				if(neuerAbschnitt)
				{
					spielzeit = zeitPauseOT;
					spielLaeuft = 1;
					neuerAbschnitt = 0;
				}
				rgbFarbe = BLAU;
				if (endeAbschnitt)
				{
					state = OT;
					neuerAbschnitt = 1;
					horn_pv = AUD_HORN_KURZ_SP;
					endeAbschnitt = 0;
				}
				break;
				
			case PENALTYS:
				if (neuerAbschnitt)
				{
					absch = 'x';
					elobLcd_clearDisplay();
					elobLcd_cursor_toXY(1, 0);
					printf("H:%i", toreH);
					elobLcd_cursor_toXY(1, 7);
					printf("%c", absch);
					elobLcd_cursor_toXY(1, 12);
					printf("G:%i", toreG);
					neuerAbschnitt = 0;
					spielLaeuft = 0;
					anzPenaltys = 0;
					zeitStrafeH = 0;
					zeitStrafeG = 0;
				}
				rgbFarbe = WEISS;
				if (toreH != toreG)
				{
					state = ENDE;
					state_old = PENALTYS;
					neuerAbschnitt = 1;
				}
				break;
				
			case ENDE:
				if (neuerAbschnitt)
				{
					elobLcd_clearDisplay();
					elobLcd_cursor_toXY(1, 0);
					printf("H:%i", toreH);
					elobLcd_cursor_toXY(1, 12);
					printf("G:%i", toreG);
					neuerAbschnitt = 0;
					spielLaeuft = 0;
					spielzeit = 0;
					zeitStrafeG = 0;
					zeitStrafeH = 0;
					if (state_old == OT)
					{
						elobLcd_cursor_toXY(1, 7);
						printf("nV");
					}
					if (state_old == PENALTYS)
					{
						elobLcd_cursor_toXY(1, 7);
						printf("nP");
					}
				}
				rgbFarbe = VIOLET;
				break;
				
			case TIMEOUT:
				if (neuerAbschnitt)
				{
					spielzeit_old = spielzeit;
					spielzeit = zeitTimeout;
					spielLaeuft = 1;
					neuerAbschnitt = 0;
				}
				rgbFarbe = ORANGE;
				if (endeAbschnitt)
				{
					endeAbschnitt = 0;
					state = state_old;
					spielzeit = spielzeit_old;
					spielLaeuft = 0;
					horn_pv = AUD_HORN_KURZ_SP;
				}
				break;
				
			default:
				break;
		}
    }
}

/***********************************************************************
************************************************************************/
//***FUNKTIONEN***
/**
 * Initialisiert PINs
 */
void pin_init(void)
{
	//1=Output, 0=Input
	DDRB = 0xFF;	//RGB
	DDRJ = 0x00;	//Taster
	DDRG = 0xFF;	//7Seg
	DDRH = 0xFF;	//7Seg
	DDRL = 0xFF;	//Display
	DDRD = 0xFF;	//Buzzer
}

/**
 * Initialisierung timer 2
 */
void timer_init(void)
{
	TCCR2A |= (2);			//Waveform Generation Mode, Mode 2 = CTC
	TCCR2B |= (0<<2);		//" , nur Vollständigkeitshalber
	TCCR2B |= (1<<7);		//force Output Compare A, FOC2A
	TCCR2B |= (0b100);		//Clock Select, CS22:0, 100 = clk/64
	
	OCR2A |= 249;		//Output Compare Register A,  (ausgerechnet, 1ms)
	
	TIMSK2 |= (1<<1);	//Compare Match A Interrupt Enable, OCIE2A
}

void flankenerkennung(void)
{
	pinj.state = PINJ;
	pinj.flanke_pos = pinj.state & ~pinj.state_old;
	pinj.flanke_neg = ~pinj.state & pinj.state_old;
	pinj.state_old = pinj.state;
}

/**
 * @param:	zeit_old:		spielzeit bevor funktion
 * @return: neue Spielzeit:	zeit_old - 1
 */
int zeitberechnung(int zeit_old)
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
 * @param: pointer dritel
 * @param: pointer anzPenaltys
 */
void tasterauswertung(int* toreH, int* toreG, int* zeitStrafeH, int* zeitStrafeG, unsigned char drittel, unsigned char* anzPen)
{
	//*** Variablen Status taster***
	static unsigned int counter_tst_T_H;
	static unsigned int counter_tst_T_G;
	static unsigned int counter_tst_2_H;
	static unsigned int counter_tst_2_G;
	static unsigned int counter_tst_5_H;
	static unsigned int counter_tst_5_G;
	
	//***Counters***
	if(counter_tst_T_H && counter_tst_T_H < delay)
		counter_tst_T_H++;
	if(counter_tst_T_G && counter_tst_T_G < delay)
		counter_tst_T_G++;
	if(counter_tst_2_H && counter_tst_2_H < delay)
		counter_tst_2_H++;
	if(counter_tst_2_G && counter_tst_2_G < delay)
		counter_tst_2_G++;
	if(counter_tst_5_H && counter_tst_5_H < delay)
		counter_tst_5_H++;
	if(counter_tst_5_G && counter_tst_5_G < delay)
		counter_tst_5_G++;
	
	//***Tore H***
	if(tstTorH_pos)
		counter_tst_T_H = 1;
	if(tstTorH_neg)
	{
		if (counter_tst_T_H >= (delay - 1))
			(*toreH)--;
		else
			(*toreH)++;
		counter_tst_T_H = 0;
	}
		
	//***Tore G***
	if(tstTorG__pos)
		counter_tst_T_G = 1;
	if(tstTorG_neg)
	{
		if (counter_tst_T_G >= (delay - 1))
			(*toreG)--;
		else
			(*toreG)++;
		counter_tst_T_G = 0;
	}
	
	//***Strafen***
	if (drittel != PENALTYS)
	{
		//**Strafe 2 H**
		if(tstStrafe2H_pos)
			counter_tst_2_H = 1;
		if(tstStrafe2H_neg)
		{
			if (counter_tst_2_H >= (delay - 1))
				*zeitStrafeH = 0;
			else
				*zeitStrafeH = zeitStrafe2;
			counter_tst_2_H = 0;
		}
		//**Strafe 2 G**
		if(tstStrafe2G_pos)
			counter_tst_2_G = 1;
		if(tstStrafe2G_neg)
		{
			if (counter_tst_2_G >= (delay - 1))
				*zeitStrafeG = 0;
			else
				*zeitStrafeG = zeitStrafe2;
			counter_tst_2_G = 0;
		}
		//**Strafe 5 H**
		if(tstStrafe5H_pos)
			counter_tst_5_H = 1;
		if(tstStrafe5H_neg)
		{
			if (counter_tst_5_H >= (delay - 1))
				*zeitStrafeH = 0;
			else
				*zeitStrafeH = zeitStrafe5;
			counter_tst_5_H = 0;
		}
		//**Strafe 5 G**
		if(tstStrafe5G_pos)
			counter_tst_5_G = 1;
		if(tstStrafe5G_neg)
		{
			if (counter_tst_5_G >= (delay - 1))
				*zeitStrafeG = 0;
			else
				*zeitStrafeG = zeitStrafe5;
			counter_tst_5_G = 0;
		}
	}
}