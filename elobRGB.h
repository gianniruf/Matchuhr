/*
 * elobRGB.h
 *
 * Created: 18.03.2020 10:00:33
 *  Author: rufg
  * ___________________________
  * Benötigt:
  * init-Funktion muss aufgerufen werden
 */ 


#ifndef ELOBRGB_H_
#define ELOBRGB_H_

//***INCLUDES***

//***DEFINES***
//RGB Standartwerte
#define RGB_HELL	255
#define RGB_MITTEL	128
#define RGB_SCHWACH	20
#define RGB_DUKEL	0

//***STRUKTUREN***

//***VARIABLEN GLOBAL***

//***FUNKTIONSPROTOTYPEN***
void elobRGB_init(void);

/**
 * Setzt Farbwerte
 * @param:	rot		0..255, ^= PWM-Wert (oder vorgefertigtes DEFINE)
 * @param:	gruen	0..255, ^= PWM-Wert
 * @param:	blau	0..255, ^= PWM-Wert
 */
void elobRGB_setColor(unsigned char rot, unsigned char gruen, unsigned char blau);

/**
 * setzt Rot-Wert
 * @param:	RGB-Wert Rot 0..255
 */
void elobRGB_setRed(unsigned char rot);

/**
 * setzt Grün-Wert
 * @param:	RGB-Wert Grün 0..255
 */
void elobRGB_setGreen(unsigned char gruen);

/**
 * setzt blau-Wert
 * @param:	RGB-Wert blau 0..255
 */
void elobRGB_setBlue(unsigned char blau);
void elobRGB_clear(void);
//***INTERRUPTS***

#endif /* ELOBRGB_H_ */