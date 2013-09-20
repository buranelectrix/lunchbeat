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
 *           iolunch.h
 * setup routine
 * ADC (reading pot) routines
 * SPI out (for LEDs) routine
 * =============================
 *     (c) 2013 Jan Cumpelik
 * =============================
 *
 */


void setup() {
	
	// tlacitko EDIT - EDIT button
	DDRB &= ~(1 << PB0); // input
	PORTB |= (1 << PB0); // pullup on

	// port D - D0-D4 vstup, D5-D7 vystup
	// port D - D0-D4 input, D5-D7 output
	DDRD  = 0b11100000;
	PORTD = 0b00011111; 
	
	// interni ADC nastaveni
	// internal ADC setup
	//ADCSRA |= (0 << ADPS2) | (0 << ADPS1) | (0 << ADPS0); //   5   us
	//ADCSRA |= (0 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); //   6.5 us
	ADCSRA |= (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //  10   us
	//ADCSRA |= (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0); //  17   us
	//ADCSRA |= (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0); //  29.6 us
	//ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); //  56   us
	//ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 112   us
	ADMUX |= (1 << REFS0);
	//ADMUX |= (1 << ADLAR); // left adjust - pro 8-bit cteni pouze ADCH
	//ADMUX &= ~(1 << ADLAR);
	ADCSRA |= (1 << ADEN); // enable ADC

	//SPI pins
	DDRB |= (1 << PB3); // mosi - vystup - output
	DDRB &= ~(1 << PB4); // miso - vstup - input
	DDRB |= (1 << PB5); // sck - vystup  - output

	DDRB |= (1 << PB2); // led latch pin
	PORTB &= ~(1 << PB2); // set low
	
	// SPI nastaveni
	SPCR &= ~(1<<SPR0) & ~(1<<SPR1); // rychlost - speed
	SPCR &= ~(1<<SPI2X);             // rychlost - speed
	SPCR |= (1<<DORD);               // data order
	SPCR |=  (1<<SPE) | (1<<MSTR);	// spi enable, spi master
	
	// interrupt nastaveni
	cli();
	//TCCR1A  = (1 << COM1A0);		// toggle OC1A pin (PB1) on compare match
	TCCR1B  = (1 << WGM12); 		// CTC mode
	TCCR1B |= (1 << CS11);			// prescaler 8
	TIMSK1 |= (1 << OCIE1A);		// ctc interrupt enable
	OCR1A = 128;						// 15625Hz

	sei();
}

void ledbarout(uint8_t value) {

	SPDR = value;
	while(!(SPSR & (1<<SPIF)));
	PORTB |= (1 << PB2); // latch it
	PORTB &= ~(1 << PB2);
}
	
uint8_t readpot8b(uint8_t p) 
{
	// set multiplexer
	p &= 7;
	ADMUX &= ~(7);
	ADMUX |= p;
	ADMUX |= (1<<ADLAR); // left adjust 
	// start ADC conversion
	ADCSRA |= (1 << ADSC); // start single conversion
	while (ADCSRA & (1 << ADSC)) {} // wait for result
	// use left adjusted ADCH
	uint8_t pot = ADCH;
	return pot;
}

uint16_t readpot10b(uint8_t p) 
{
	// set multiplexer
	p &= 7;
	ADMUX &= ~(7);
	ADMUX |= p;
	ADMUX &= ~(1<<ADLAR); // no left adjust 
	// start ADC conversion
	ADCSRA |= (1 << ADSC); // start single conversion
	while (ADCSRA & (1 << ADSC)) {} // wait for result
	// convert to 16 bit variable
	uint8_t potL = ADCL;
	uint8_t potH = ADCH;
	uint16_t pot = potH;
	pot <<= 8;
	pot |= potL;
	return pot;
}


