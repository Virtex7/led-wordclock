// Prototypen:

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
#include "./dcf.h"

//Einbinden der I2C Library
#define I2C_DDR  DDRB
#define I2C_PORT PORTB
#define I2C_PIN  PINB
#define SCL PB5
#define SDA PB3
uint8_t ERR = 0; // Variable (global)
#include "../atmel/lib/0.1.3/io/serial/i2c.h"

//Pindefinitionen für den HT1632 Chip
#define CS(x) out(PORTC,PC2,0,x)	// idle ON
#define RD(x) out(PORTC,PC3,0,x)	// idle ON
#define WR(x) out(PORTC,PC5,0,x)	// idle ON
#define DATA(x) out(PORTC,PC4,0,x)
#include "./ht1632.h" //Einbinden der Displaycontroller-Ansteuerung

void rtcInit(void) {
	i2c_tx(0b00000000,0xE,0b11010000); // Aktiviere Oszillator
	delayms(10);
// 	i2c_tx(0b10000000,0xB,0b11010000); // Aktiviere den Alarm jede Minute
// 	delayms(10);
	i2c_tx(0b10000000,0xC,0b11010000);
	delayms(10);
	i2c_tx(0b10000000,0xD,0b11010000);
	delayms(10);
// 	i2c_tx(0b00000000,0xE,0b11010000); // Aktiviere Oszillator und den sekundlichen INT
// 	delayms(10);
	i2c_tx(0x00,0x02,0b11010000); // Stelle 24-Stunden-Modus ein
	delayms(10);
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
