// Prototypen:

void setMinutenLeds(void);
void rtcInit(void);
void uart_init(void);
void init(void);


// Definiere Anschlüsse des DCF-Moduls
#define DCF_POWER_DDR	DDRC
#define DCF_POWER_PORT	PORTC
#define DCF_POWER_PIN	PC1
#define DCF_PON_DDR	DDRD
#define DCF_PON_PORT	PORTD
#define DCF_PON_PIN	PD4
#define DCF_IN() (PIND & (1<<PD2))
#define DCF_LED(x) out(PORTD,PD7,0,x)
#include "./dcf.h"

// Einbinden der I2C Library
#define I2C_DDR  DDRB
#define I2C_PORT PORTB
#define I2C_PIN  PINB
#define SCL PB5
#define SDA PB3
uint8_t ERR = 0; // Variable (global)
#define I2C_DELAY_FACTOR 1
#include "../AtmelLib/io/serial/i2c.h"

// Pindefinitionen für den HT1632 Chip
#define CS(x) out(PORTC,PC2,0,x)	// idle ON
#define RD(x) out(PORTC,PC3,0,x)	// idle ON
#define WR(x) out(PORTC,PC5,0,x)	// idle ON
#define DATA(x) out(PORTC,PC4,0,x)
#include "./ht1632.h" //Einbinden der Displaycontroller-Ansteuerung

// Minuten-LEDs - hier kann die Reihenfolge definiert werden

#define MIN1(x) out(PORTD,PD5,0,x)
#define MIN2(x) out(PORTD,PD6,0,x)
#define MIN3(x) out(PORTB,PB1,0,x)
#define MIN4(x) out(PORTB,PB0,0,x)

#define POWER_LED(x) out(PORTC,PC0,0,x)


void setMinutenLeds(void) {
#ifdef MINUTEN_PHIL
if (minutenValid %5 == 0) { // alle LEDs aus
	MIN1(0);
	MIN2(0);
	MIN3(0);
	MIN4(0);
}
if (minutenValid %5 >= 1) { // LED 1 an
	MIN1(1);
}
if (minutenValid %5 >= 2) { // LED 2 an
	MIN2(1);
}
if (minutenValid %5 >= 3) { // LED 3 an
	MIN3(1);
}
if (minutenValid %5 >= 4) { // LED 4 an
	MIN4(1);
}
#endif
}

void uart_init(void) {
	UBRRL = 10;		// 57,6k Baud, 2 Stoppbits (Achtung: -1,4% Fehler....)
	UCSRB = (1<<RXEN)  | (1<<TXEN);
	UCSRC = (1<<URSEL) | (1<<USBS) | (1<<UCSZ1) | (1<<UCSZ0);
}



void init(void) {
	
	// Minuten-LEDs als Output definieren 
	// Die LEDs gehen nacheinander an und bei minuten_valid%5 = 0 aus.
	DDRB  |= (1<<PB0) | (1<<PB1) | (1<<PB5) | (1<<PB6);
	// PowerLED, HTKommunikation (CS, RD, WR, Data) als Output definieren
	DDRC  |= (1<<PC0) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);
	// Definieren der drei LEDs als Output
	DDRD  |= (1<<PD5) | (1<<PD6) | (1<<PD7);
	// UART-TX als Output
	DDRD  |= (1<<PD1);
	// Output für das DCF Modul
	DDRC  |= (1<<PC1); 
	// Pullup DEBUG-Jumper
	PORTB |= (1<<PB2);
	// Power LED, leuchtet; CS, RD, WR sind idle HIGH
	PORTC |= (1<<PC0) | (1<<PC2) | (1<<PC3) | (1<<PC5); 
	// MOSI auf High Pegel
	PORTD |= (1<<PD3);
	// DCF Modul wird ausgeschalten (Invertierter PIN)
	PORTC |= (1<<PC1); 
	
	
	delayms(100);
	
	uart_init();		// Akivierung Kommuniaktion UART 57,6kBaud 8n1
	
	dcfInit();		// Pon als Output, Setzen der Timereigenschaften
	
	i2c_init();		// Pullup SDA, Initzialisierung I2C
		
	uart_tx_strln("DCF-Wordclock!");
	
	htInit();		// Aktiviere Displaycontroller
}

void minutentest(void) {
	for(uint8_t i=0; i<3; i++) {
		MIN1(1);
		delayms(500);
		MIN2(1);
		delayms(500);
		MIN3(1);
		delayms(500);
		MIN4(1);
		
		delayms(1000);
		MIN1(0);
		MIN2(0);
		MIN3(0);
		MIN4(0);
		delayms(500);
		
	}
}
