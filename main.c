/*
 * Matchuhr.c
 *
 * Created: 06.10.2019 11:36:14
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

//***Taster***
#define tstStartStopp	(pinj.flanke_pos & (1<<0))
#define tstTorH			(pinj.flanke_pos & (1<<2))
#define tstTorG			(pinj.flanke_pos & (1<<1))
#define tstStrafe2H		(pinj.flanke_pos & (1<<4))
#define tstStrafe2G		(pinj.flanke_pos & (1<<3))
#define tstStrafe5H		(pinj.flanke_pos & (1<<6))
#define tstStrafe5G		(pinj.flanke_pos & (1<<5))
#define tstTimeout		(pinj.flanke_pos & (1<<7))

//***rgb***
#define rgbDunkel		(PORTB = 0)
#define rgbRot			(PORTB = (1<<5))
#define rgbOrange		(PORTB = (0b11<<5))
#define rgbGruen		(PORTB = (1<<6))
#define rgbTuerkis		(PORTB = (0b111<<5))
#define rgbBlau			(PORTB = (1<<7))
#define rgbViolett		(PORTB = (0b101<<5))

//***Diverses***
#define audHorn_on		(PORTD |= (1<<7))
#define audHorn_off		(PORTD &= ~(1<<7))
#define AUD_HORN_LANG_SP	1000	//1sek
#define AUD_HORN_KURZ_SP	500		//0.5sek


//***ENUMS***
enum abschnitt {ABSCH1 = 1, ABSCH2, ABSCH3, ABSCHOT, PAUSE1, PAUSE2, PAUSEOT, OT, PENALTYS, TIMEOUT, ENDE};
enum farben {DUNKEL = 0, ROT, ORANGE, GRUEN, TUERKIS, BLAU, VIOLETT};

//***STRUCKTUREN***
struct flanken
{
	unsigned char state;
	unsigned char state_old;
	unsigned char flanke_pos;
	unsigned char flanke_neg;
};
struct flanken pinj;

//***VARIABLEN GLOBAL***
unsigned int zeitSpielzeit = zeitAbschnitt;	//wird ausgegeben
unsigned int zeitSpielzeit_old;	//zwischenspeicher (z.B. für Timeout)
unsigned char state = ABSCH1;	//Spielabschnitt
unsigned int anzTimerInterrupts_pv;	//=anz. ms
unsigned char eineSekunde;	//1 = eine Sekunde
unsigned char startStopp = 0;	//1 = zeit läuft
unsigned int toreH = 0;
unsigned int toreG = 0;
unsigned char neuerAbschnitt = 1;	//1 = neuer Abschnitt
unsigned char tasterblock = 0;	//>1 = blockiert
unsigned int zeitStrafeH = 0;
unsigned int zeitStrafeG = 0;
unsigned char strafe2H_aktiv = 0;	//1 = Strafe läuft
unsigned char strafe2G_aktiv = 0;
unsigned char strafe5H_aktiv = 0;
unsigned char strafe5G_aktiv = 0;
unsigned char penaltys[15];
unsigned char absch;
unsigned int audHorn_pv;	//0 = aus


//***FUNKTIONSPROTOTYPEN***
void pin_init(void);
void timer_init(void);
void rgb(unsigned char Farbe);
int zeitberechnung(unsigned int zeit);
void penaltyschiessen(void);
void flankenerkennung(void);

//***INTERRUPTS***
ISR(TIMER2_COMPA_vect)
{
	anzTimerInterrupts_pv++;
	if (anzTimerInterrupts_pv > 1000)	//1s
	{
		anzTimerInterrupts_pv = 0;
		eineSekunde = 1;
	}
	if (tasterblock)
		tasterblock--;
	if (audHorn_pv)
		audHorn_pv--;
	ausgabe7seg(zeitSpielzeit, 0, 1, 0);
}

/**
 * MAIN-ROUTINE
 */
int main(void)
{
    //***Variablen Lokal***
	unsigned char rgbFarbe;
	unsigned char torstand_old;
	unsigned char state_old;
	
	//***Init***
	pin_init();
	elobLcd_init();
	elobLcd_cursor_off();
	timer_init();
	
	sei();	//Interrupts enable
	
	lcdBacklight_on;
	
    while (1) 
    {
		flankenerkennung();
		if (tstStartStopp)
			startStopp ^= 1;
		if (tstTorH)
		{
			if (strafe2G_aktiv && !strafe5G_aktiv && !(strafe2H_aktiv || strafe5H_aktiv))
				zeitStrafeG = 0;
			toreH++;
		}
		if (tstTorG)
		{
			if (strafe2H_aktiv && !strafe5H_aktiv && !(strafe2G_aktiv || strafe5G_aktiv))
				zeitStrafeH = 0;
			toreG++;
		}
		if (state == PENALTYS)
			penaltyschiessen();
		else
		{
			if(tstStrafe2H)
			{
				zeitStrafeH = zeitStrafe2;
				strafe2H_aktiv = 1;
			}
			if(tstStrafe2G)
			{
				zeitStrafeG = zeitStrafe2;
				strafe2G_aktiv = 1;
			}
			if(tstStrafe5H)
			{
				zeitStrafeH = zeitStrafe5;
				strafe5H_aktiv = 1;
			}
			if(tstStrafe5G)
			{
				zeitStrafeG = zeitStrafe5;
				strafe5G_aktiv = 1;
			}
		
		}
		if(tstTimeout)
		{
			state_old = state;
			zeitSpielzeit_old = zeitSpielzeit;
			state = TIMEOUT;
			neuerAbschnitt = 1;
			startStopp = 1;
		}
		
		switch(state)
		{
			case ABSCH1:
				if(neuerAbschnitt)
				{
					zeitSpielzeit = zeitAbschnitt;
					neuerAbschnitt = 0;
				}
				absch = '1';
				if (startStopp)
					rgbFarbe = GRUEN;
				else
					rgbFarbe = ROT;
				//Transition
				if (zeitSpielzeit <= 0)
				{
					state = PAUSE1;
					neuerAbschnitt = 1;
					audHorn_pv = AUD_HORN_LANG_SP;
				}
				break;
				
			case ABSCH2:
				if(neuerAbschnitt)
				{
					zeitSpielzeit = zeitAbschnitt;
					neuerAbschnitt = 0;
					startStopp = 0;
				}
				absch = '2';
				if (startStopp)
				rgbFarbe = GRUEN;
				else
				rgbFarbe = ROT;
				//Transition
				if (zeitSpielzeit <= 0)
				{
					state = PAUSE2;
					neuerAbschnitt = 1;
					audHorn_pv = AUD_HORN_LANG_SP;
				}
				break;
				
			case ABSCH3:
				if(neuerAbschnitt)
				{
					zeitSpielzeit = zeitAbschnitt;
					neuerAbschnitt = 0;
					startStopp = 0;
				}
				absch = '3';
				if (startStopp)
				rgbFarbe = GRUEN;
				else
				rgbFarbe = ROT;
				//Transition
				if (zeitSpielzeit <= 0)	
				{
					audHorn_pv = AUD_HORN_LANG_SP;
					if (toreH == toreG)	//unentschieden
						state = PAUSEOT;
					else //Spielende
						state = ENDE;
					neuerAbschnitt = 1;
				}
				break;
				
			case PAUSE1:
				if (neuerAbschnitt)
				{
					zeitSpielzeit = zeitPause;
					neuerAbschnitt = 0;
				}
				absch = '1';
				rgbFarbe = BLAU;
				//Transition
				if (zeitSpielzeit <= 0)
				{
					state = ABSCH2;
					neuerAbschnitt = 1;
					audHorn_pv = AUD_HORN_KURZ_SP;
				}
				break;
				
			case PAUSE2:
				if (neuerAbschnitt)
				{
					zeitSpielzeit = zeitPause;
					neuerAbschnitt = 0;
				}
				absch = '2';
				rgbFarbe = BLAU;
				//Transition
				if (zeitSpielzeit <= 0)
				{
					state = ABSCH3;
					neuerAbschnitt = 1;
					audHorn_pv = AUD_HORN_KURZ_SP;
				}
				break;
				
			case PAUSEOT:
				if (neuerAbschnitt)
				{
					zeitSpielzeit = zeitPauseOT;
					neuerAbschnitt = 0;
				}
				absch = '3';
				rgbFarbe = BLAU;
				//Transition
				if (zeitSpielzeit <= 0)
				{
					state = OT;
					neuerAbschnitt = 1;
					audHorn_pv = AUD_HORN_KURZ_SP;
				}
				break;
				
			case OT:
				if (neuerAbschnitt)
				{
					zeitSpielzeit = zeitOT;
					neuerAbschnitt = 0;
					startStopp = 0;
					torstand_old = toreH + toreG;
				}
				absch = '+';
				if (startStopp)
				rgbFarbe = GRUEN;
				else
				rgbFarbe = ROT;
				//Transition
				if(torstand_old < toreH + toreG)
					state = ENDE;
				if (zeitSpielzeit <= 0)
				{
					state = PENALTYS;
					neuerAbschnitt = 1;
					audHorn_pv = AUD_HORN_KURZ_SP;
				}
				break;
				
			case TIMEOUT:
				if(neuerAbschnitt)
				{
					zeitSpielzeit = zeitTimeout;
					neuerAbschnitt = 0;
					rgbFarbe = TUERKIS;
				}
				//Transition
				if (zeitSpielzeit <= 0)
				{
					startStopp = 0;
					audHorn_pv = AUD_HORN_KURZ_SP;
					zeitSpielzeit = zeitSpielzeit_old;
					state = state_old;
				}
				break;
				
			case PENALTYS:
				if (neuerAbschnitt)
				{
					absch = 'P';
					elobLcd_clearDisplay();
					printf("H:%i \t%c \tG:%i", toreH, absch, toreG);	//obere Zeile
					neuerAbschnitt = 0;
					torstand_old = toreH + toreG;
				}
				rgbFarbe = VIOLETT;
				//Transition
				if (torstand_old < toreH + toreG)
				{
					state = ENDE;
					neuerAbschnitt = 1;
				}
				break;
				
			case ENDE:
				rgbFarbe = ORANGE;
				absch = ' ';
				zeitSpielzeit = 0;
				startStopp = 0;
				if (neuerAbschnitt)
				{
					neuerAbschnitt = 0;
					elobLcd_clearDisplay();
					printf("H:%i \t%c \tG:%i", toreH, absch, toreG);	//obere Zeile
					zeitSpielzeit = 0;
					zeitStrafeG = 0;
					zeitStrafeH = 0;
				}
				break;					
		}
		
		//***Sirene***
		if (audHorn_pv)
			audHorn_on;
		else
			audHorn_off;
				
		if (eineSekunde)
		{
			//***Zeitberechnung***
			if (startStopp)
			{
				zeitSpielzeit = zeitberechnung(zeitSpielzeit);
				if (zeitStrafeH && ((state == ABSCH1) || (state == ABSCH2) || (state == ABSCH3) || (state == OT)))
					zeitStrafeH = zeitberechnung(zeitStrafeH);
				if (zeitStrafeG && ((state == ABSCH1) || (state == ABSCH2) || (state == ABSCH3) || (state == OT)))
					zeitStrafeG = zeitberechnung(zeitStrafeG);
			}
			//***Ausgabe Display***
			if (!(state == PENALTYS))
			{
				elobLcd_clearDisplay();
				printf("H:%i \t%c \tG:%i", toreH, absch, toreG);	//obere Zeile
				if (zeitStrafeG || zeitStrafeH)
				{
					printf("\n");	//untere Zeile
					if (zeitStrafeH)
					{
						unsigned int minuten = zeitStrafeH / 100;
						unsigned int sekunden = zeitStrafeH - (minuten * 100);
						printf("%i:%02i", minuten, sekunden);
					}
					else
					{
						printf("\t\t");
						strafe2H_aktiv = 0;
						strafe5H_aktiv = 0;
					}
						
					if (zeitStrafeG)
					{
						unsigned int minuten = zeitStrafeG / 100;
						unsigned int sekunden = zeitStrafeG - (minuten * 100);
						printf("\t\t\t%i:%02i", minuten, sekunden);
					}
					else
					{
						strafe2G_aktiv = 0;
						strafe5G_aktiv = 0;
					}
				}
			eineSekunde = 0;
			}
		}
	rgb(rgbFarbe);	
	}
}

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

/**
 * Inkremmentiert Zeiten alle eine Sekunde
 */
int zeitberechnung(unsigned int zeit)
{
	if (zeit > 0)
	{
		if (!(zeit % 100) && !(zeit % 10))	//X:00
		{
			zeit = (zeit - 100 + 60);
			zeit--;
		}
		else
			zeit--;
	}
	else
		zeit = 0;
	
	return zeit;
}


/**
 * RGB-Farbausgabe
 */
void rgb(unsigned char Farbe)
{
	switch(Farbe)
	{
		case ROT:
			rgbRot;
			break;
		case ORANGE:
			rgbOrange;
			break;
		case GRUEN:
			rgbGruen;
			break;
		case TUERKIS:
			rgbTuerkis;
			break;
		case BLAU:
			rgbBlau;
			break;
		case VIOLETT:
			rgbViolett;
			break;
		default:
			rgbDunkel;
			break;
	}
}

/**
 * Sonderanzeige Penaltyschiessen
 */
void penaltyschiessen(void)
{
	static unsigned int anzPenaltys;
	unsigned int anzPenRunden = anzPenaltys / 2;
	if(anzPenRunden <= 5)
	{
		if(tstStrafe5H)
		{
			elobLcd_cursor_toXY(2, anzPenRunden);
			elobLcd_zeichen('O');
		}
		if (tstStrafe5G)
		{
			elobLcd_cursor_toXY(2, anzPenRunden);
			elobLcd_zeichen('X');
		}
		if(tstStrafe2H)
		{
			elobLcd_cursor_toXY(2, (7+anzPenRunden));
			elobLcd_zeichen('O');
		}
		if(tstStrafe2G)
		{
			elobLcd_cursor_toXY(2, (7+anzPenRunden));
			elobLcd_zeichen('X');
		}
	}
	if(anzPenaltys > 8)
	{
		if (!(anzPenaltys % 2) && (tstStrafe2G||tstStrafe2H||tstStrafe5G||tstStrafe5H))
		{
			elobLcd_clearDisplay();
			printf("H:%i \t%c \tG:%i", toreH, absch, toreG);	//obere Zeile
			elobLcd_cursor_2Line();
		}
		
		if(tstStrafe5H)
		{
			elobLcd_cursor_toXY(2, 0);
			elobLcd_zeichen('O');
		}
		if (tstStrafe5G)
		{
			elobLcd_cursor_toXY(2, 0);
			elobLcd_zeichen('X');
		}
		if(tstStrafe2H)
		{
			elobLcd_cursor_toXY(2, 8);
			elobLcd_zeichen('O');
		}
		if(tstStrafe2G)
		{
			elobLcd_cursor_toXY(2, 8);
			elobLcd_zeichen('X');
		}
	}
	
	if(tstStrafe2G||tstStrafe2H||tstStrafe5G||tstStrafe5H)
		anzPenaltys++;
}

/**
 * Flankenerkennung
 */
void flankenerkennung(void)
{
	pinj.state = PINJ;
	pinj.flanke_pos = pinj.state & ~pinj.state_old;
	pinj.flanke_neg = ~pinj.state & pinj.state_old;
	pinj.state_old = pinj.state;
	
	if (pinj.flanke_pos)
		tasterblock = 30;	//30ms
}