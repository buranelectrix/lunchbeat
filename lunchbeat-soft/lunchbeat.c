/*
 * =============================
 *   B U R A N E L E C T R I X 
 * =============================
 *
 *       L U N C H B E A T
 *
 *        1-bit groovebox 
 *
 * =============================
 *          version: 1
 *   target: atmega328p@16MHz
 *     (arduino compatible)
 * =============================
 *     (c) 2013 Jan Cumpelik
 * =============================
 *
 */

#include <avr/io.h>

#define F_CPU 16000000L
#include <util/delay.h>

#include <avr/interrupt.h>

#include "iolunch.h"


#define NOW 	0
#define PREV 	1
#define PLAY 	4
#define EDIT 	5
uint8_t rawbutton = 0; 
uint8_t button[2][6] = {{0,0,0,0,0,0},{0,0,0,0,0,0}} ; 	
uint8_t seq[8] = {	0b00000001,
							0b00000100,
							0b00001010,
							0b00000100,
							0b00000000,
							0b00000100,
							0b00001010,
							0b00001100 };
uint8_t playing = 0;
uint8_t editmode = 0;
volatile uint8_t playstep = 0;
volatile uint8_t newstep = 0;
volatile uint8_t editstep = 0;
volatile uint16_t tempo = 4000;
volatile uint16_t counter = 0;
uint8_t ledbar = 0;
uint8_t ledbarhlf = 0;
uint16_t pot[5];
#define KICK 	0
#define SNARE 	1
#define HAT 	2
#define WAVE	3
#define TEMPO	4
uint16_t gate[4];  





/////////////////////////////////////////////////
//
// controls()
//
// cteni tlacitek a potenciometru
// reading buttons and potentiometers
//
/////////////////////////////////////////////////

void controls() {

// cteni hodnot z potenciometru
// reading potentiometers
	static uint8_t p = 0;
	pot[p] = readpot10b((4-p));
	p ++;
	if (p > 4) p = 0;


	// snimani buttonu do registru "rawbutton"
	// capture buttons to "rawbutton" variable
	uint8_t rawbutton = ~(PIND) & 0b00011111;
	if (~(PINB) & (1<<PB0)) rawbutton |= 0b00100000; // pridame edit tlacitko z portu B
																	 // add EDIT button from B port

	// debouncing
	uint8_t but;
	for (but = 0; but < 6; but ++) {
			button[NOW][but] <<= 1;
		if (rawbutton & (1 << but)) {
			button[NOW][but] |= 0x01;
		} else {
			button[NOW][but] &= ~(0x01);
		}
	}

	// akce pro prave stisknuta tlacitka oneshot zvuku (tl. 0, 1, 2)
	// button with oneshot sound just pressed
	uint8_t tlac = 0;
	for (tlac = 0; tlac < 6; tlac++) {
		if (tlac < 4) {
			if ((button[NOW][tlac] == 0xff) && (button[PREV][tlac] != 0xff)) {
				if (editmode) {
					if (seq[editstep] & (1 << tlac)) {
						seq[editstep] &= ~(1 << tlac);
					}
					else {
						seq[editstep] |= (1 << tlac);
					}
				} 
				else {
					gate[tlac]	= 1;
				}
			}
				
			// pro button wave (tl. 3), kde hraje zvuk pokud je stisknuto 
			// BASS button - plays until realeased
			if (tlac == 3) {
				if (button[NOW][WAVE] == 0x00) {
					if (!(playing   &&    ((seq[playstep] & (1 << WAVE))))) {
						gate[WAVE] = 0;
					}
				}
			}
		}
		
		// tlacitko play (tl. 4)
		// play button
		if (tlac == 4) {
			if ((button[NOW][PLAY] == 0xff) && (button[PREV][PLAY] != 0xff)) { 
				if (playing) {
					playing = 0;
				} else {
					playing = 1;
					counter = 0;
					playstep = 0;
					newstep = 1;
					tempo = 5500 - (pot[TEMPO] << 2);
				}
			}
		}
		
		// tlacitko edit (tl.5)
		// edit button
		if (tlac == 5) {
			if ((button[NOW][EDIT] == 0xff) && (button[PREV][EDIT] != 0xff)) {
				if (editmode) {
					editstep ++;
					if (editstep > 7) {
						editmode = 0;
						editstep = 0;
						ledbarhlf = 0;
					}
				} 
				else {
					editmode = 1;
				}
			}
		}

		button[PREV][tlac] = button[NOW][tlac];
	}
}



/////////////////////////////////////////////////
//
// lights()
//
/////////////////////////////////////////////////

void lights() {

	if (editmode) {
		if (seq[editstep] & (1 << KICK))  {ledbar |= 0b11000000;} else {ledbar &= ~(0b11000000);}
		if (seq[editstep] & (1 << SNARE)) {ledbar |= 0b00110000;} else {ledbar &= ~(0b00110000);}
		if (seq[editstep] & (1 << HAT))   {ledbar |= 0b00001100;} else {ledbar &= ~(0b00001100);}
		if (seq[editstep] & (1 << WAVE))  {ledbar |= 0b00000011;} else {ledbar &= ~(0b00000011);}

		ledbar 		|=  (1<<(7-editstep));
		ledbarhlf  	 = ~(1<<(7-editstep));
	}
	else {
		if (gate[KICK] ) {ledbar |= 0b11000000;} else {ledbar &= ~(0b11000000);}
		if (gate[SNARE]) {ledbar |= 0b00110000;} else {ledbar &= ~(0b00110000);}
		if (gate[HAT]  ) {ledbar |= 0b00001100;} else {ledbar &= ~(0b00001100);}
		if (gate[WAVE] ) {ledbar |= 0b00000011;} else {ledbar &= ~(0b00000011);}
	}
}



/////////////////////////////////////////////////
/////////////////////////////////////////////////
//
//   M A I N 
//
/////////////////////////////////////////////////
/////////////////////////////////////////////////


int main() {

	setup();

	for (;;) {
		
		controls();		// read buttons and pots
		lights();		// compute which LED to light up

		if (newstep) {
			newstep = 0;
			uint8_t i;
			for (i = 0; i < 4; i++) {
				if (seq[playstep] & (1 << i)) gate[i] = 1;			
			}
		}
	}
}



/////////////////////////////////////////////////
/////////////////////////////////////////////////
//
//   I N T E R R U P T 
//
/////////////////////////////////////////////////
/////////////////////////////////////////////////


ISR(TIMER1_COMPA_vect) {

	// counter pocita tempo 
	// counter variable counts tempo
	if (playing) {
		counter ++;
		if (counter > tempo)   {
			tempo = 5500 - (pot[TEMPO] << 2);
			counter = 0;
			newstep = 1;
			playstep ++;
			if (playstep > 7) playstep = 0;
		}
	}
	
	///////////////////////
	// vypocet zvuku
	// sound modelling :)

	// kick
	static uint8_t kick = 0;
	static uint16_t kickfreq = 2;
	if (gate[KICK]) {
		if (gate[KICK] == 1) { // zvuk zrovna zacal - sound starts
			kickfreq = 2;
		}
		gate[KICK] ++;
		if (gate[KICK] > ((pot[KICK] << 3) + 0x01ff)) { // cas pro konec zvuku - sound ends
			gate[KICK] = 0;  // ticho
			kick = 0;
		} 
		else { // zvuk hraje - sound plays
			if (gate[KICK] == kickfreq) {
				if (kick) {
					kick = 0;
				} else {
					kick = 1;
				}
				kickfreq += (kickfreq>>4) + (64-(pot[KICK]>>4));
			}
		}
	}

	//snare 
	static uint8_t snare = 0;
	static uint16_t snarefreq = 2;
	static uint16_t snarelfsr = 0b0011010010100100; // seed pro linear feedback shift register
	if (gate[SNARE]) {
		if (gate[SNARE] == 1) {
			snarefreq = 2;
		}
		gate[SNARE] ++;
		if (gate[SNARE] > ((pot[SNARE]<<2) + 0x01ff)) { // cas pro konec zvuku - end of sound
			gate[SNARE] = 0;  // ticho - silence
			snare = 0;
		} else { 			// zvuk hraje - sound is playing
		// prvni cast zvuku - first part of sound
			if (gate[SNARE] < (pot[SNARE] + 0x0090)) {

				if ((gate[SNARE] & 128) ||	 (!(gate[SNARE] % ((pot[SNARE] >> 5) + 3)))) {
					if ( ((snarelfsr & 3) == 0) || ((snarelfsr & 3) == 3) ) {
						if (snare) {snare = 0;} else {snare = 1;}
						snarelfsr >>= 1;
						snarelfsr |= 0b1000000000000000;
					} else {
						snarelfsr >>= 1;
					}
				}
			}
			else 
			// druha cast zvuku - second part of sound
			{
				if ((gate[SNARE] & 32) ||	 (!(gate[SNARE] % ((pot[SNARE] >> 4) + 5)))) {
					if ( ((snarelfsr & 3) == 0) || ((snarelfsr & 3) == 3) ) {
						if (snare) {snare = 0;} else {snare = 1;}
						snarelfsr >>= 1;
						snarelfsr |= 0b1000000000000000;
					} else {
						snarelfsr >>= 1;
					}
				}
			}
		}
	}

	//hat 
	uint8_t hat = 0;
	static uint16_t hatlfsr = 0b0100101010001010; // seed for linear feedback shift register
	if (gate[HAT]) {
		gate[HAT] ++;
		if (gate[HAT] > (pot[HAT] + (pot[HAT] >> 2) + 0x01ff)) { // cas pro konec zvuku - sound ends here
			gate[HAT] = 0;  // ticho - silence
			hat = 0;
		} else { // zvuk hraje - sound is playing
			if (!(gate[HAT] % (((pot[HAT] + 190) >> 6) ))) {
				if ( ((hatlfsr & 3) == 0) || ((hatlfsr & 3) == 3) ) {
					hat = 1;
					hatlfsr >>= 1;
					hatlfsr |= 0b1000000000000000;
				} else {
					hatlfsr >>= 1;
				}
			}
		}
	}
	
	//wave 
	static uint8_t wave = 0;
	static uint16_t wavefreq = 1;
	if (gate[WAVE]) {
		gate[WAVE] ++;
		if (!(gate[WAVE] % wavefreq )) {
			wavefreq = 50 + (255 - (pot[WAVE] >> 2));
			gate[WAVE] = 1;
			if (wave) {wave = 0;} else {wave = 1;}
		}
	} 
	else {
		if ((!playing) || (!(seq[playstep] & (1 << WAVE)))) {
			wave = 0;
		}
	}



	// smichani vsech zvuku dohromady
	// add all sounds together
	uint8_t mix = kick + snare + hat + wave;	

	// zde uz primo vystup na prevodnik
	// prevodnik je na 5, 6 a 7 pinu portu D
	//
	// output to DAC
	// DAC is on PD5, PD6 and PD7 pins
	uint8_t portmp = (mix << 5);
	portmp |= 0b00011111;	// pullups on PD0 - PD4
	PORTD = portmp;


	// out lights
	uint8_t leds = ledbar;
	static uint8_t darkcycle = 0;
	darkcycle ++;
	if (darkcycle > 25) darkcycle = 0;
	if (darkcycle) leds &= ~(ledbarhlf);
	ledbarout(leds);

}

/*
 * konec programu
 * end of sranda
 */

