/*
 * elobGPIO.h
 *
 * Created: 18.03.2020 17:38:05
 *  Author: rufg
 */ 


#ifndef ELOBGPIO_H_
#define ELOBGPIO_H_

//***DEFINES***
//**Allgemein**
#define TASTER		PINJ
#define SCHALTER	PINC
#define LEDs		PORTA

//**Taster**
#define tst0	(TASTER & (1<<0))
#define tst1	(TASTER & (1<<1))
#define tst2	(TASTER & (1<<2))
#define tst3	(TASTER & (1<<3))
#define tst4	(TASTER & (1<<4))
#define tst5	(TASTER & (1<<5))
#define tst6	(TASTER & (1<<6))
#define tst7	(TASTER & (1<<7))

#define tst0_pos	(taster.flanke_pos & (1<<0))
#define tst1_pos	(taster.flanke_pos & (1<<1))
#define tst2_pos	(taster.flanke_pos & (1<<2))
#define tst3_pos	(taster.flanke_pos & (1<<3))
#define tst4_pos	(taster.flanke_pos & (1<<4))
#define tst5_pos	(taster.flanke_pos & (1<<5))
#define tst6_pos	(taster.flanke_pos & (1<<6))
#define tst7_pos	(taster.flanke_pos & (1<<7))

#define tst0_neg	(taster.flanke_neg & (1<<0))
#define tst1_neg	(taster.flanke_neg & (1<<1))
#define tst2_neg	(taster.flanke_neg & (1<<2))
#define tst3_neg	(taster.flanke_neg & (1<<3))
#define tst4_neg	(taster.flanke_neg & (1<<4))
#define tst5_neg	(taster.flanke_neg & (1<<5))
#define tst6_neg	(taster.flanke_neg & (1<<6))
#define tst7_neg	(taster.flanke_neg & (1<<7))

//**Schalter**
#define sw0		(SCHALTER & (1<<0))
#define sw1		(SCHALTER & (1<<1))
#define sw2		(SCHALTER & (1<<2))
#define sw3		(SCHALTER & (1<<3))
#define sw4		(SCHALTER & (1<<4))
#define sw5		(SCHALTER & (1<<5))
#define sw6		(SCHALTER & (1<<6))
#define sw7		(SCHALTER & (1<<7))

#define sw0_pos	(schalter.flanke_pos & (1<<0))
#define sw1_pos	(schalter.flanke_pos & (1<<1))
#define sw2_pos	(schalter.flanke_pos & (1<<2))
#define sw3_pos	(schalter.flanke_pos & (1<<3))
#define sw4_pos	(schalter.flanke_pos & (1<<4))
#define sw5_pos	(schalter.flanke_pos & (1<<5))
#define sw6_pos	(schalter.flanke_pos & (1<<6))
#define sw7_pos	(schalter.flanke_pos & (1<<7))

#define sw0_neg	(schalter.flanke_neg & (1<<0))
#define sw1_neg	(schalter.flanke_neg & (1<<1))
#define sw2_neg	(schalter.flanke_neg & (1<<2))
#define sw3_neg	(schalter.flanke_neg & (1<<3))
#define sw4_neg	(schalter.flanke_neg & (1<<4))
#define sw5_neg	(schalter.flanke_neg & (1<<5))
#define sw6_neg	(schalter.flanke_neg & (1<<6))
#define sw7_neg	(schalter.flanke_neg & (1<<7))

//**LED**
#define led0_on	(LEDs |= (1<<0))
#define led1_on	(LEDs |= (1<<1))
#define led2_on	(LEDs |= (1<<2))
#define led3_on	(LEDs |= (1<<3))
#define led4_on	(LEDs |= (1<<4))
#define led5_on	(LEDs |= (1<<5))
#define led6_on	(LEDs |= (1<<6))
#define led7_on	(LEDs |= (1<<7))

#define led0_off	(LEDs &= ~(1<<0))
#define led1_off	(LEDs &= ~(1<<1))
#define led2_off	(LEDs &= ~(1<<2))
#define led3_off	(LEDs &= ~(1<<3))
#define led4_off	(LEDs &= ~(1<<4))
#define led5_off	(LEDs &= ~(1<<5))
#define led6_off	(LEDs &= ~(1<<6))
#define led7_off	(LEDs &= ~(1<<7))

//***STRUKTUREN***
struct flanken_digi{
	unsigned char flanke_pos;
	unsigned char flanke_neg;
	unsigned char old;
	unsigned char* pin;
};

struct blinken{
	unsigned char led_position;		//0-255
	unsigned char dutycycle;	//teilwert
	unsigned int frequenz_sp;	//gem. aufrufinterval
	unsigned int frequenz_pv;	
};

//***VARIABLEN GLOBAL***
struct flanken_digi taster;
struct flanken_digi schalter;

//***FUNKTIONSPROTOTYPEN***

/**
 * Erkennt flanken
 * @param:	pointer auf Struktur flanke von gewünschtem PIN
 */
void flankenerkennung_digi(struct flanken_digi *);

/**
 * lässt LEDs blinken
 * @param:	pointer auf Struktur blinken mit gewünschtr/em LED
 */
void ledBlinken(struct blinken *);

void elobDigiIO_init(void);

//***INTERRUPTS***


#endif /* ELOBGPIO_H_ */