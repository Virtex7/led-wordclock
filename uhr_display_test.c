/*
 *    Filename: uhr.c
 *     Version: 0.2.4
 * Description: Ansteuerung für eine umgangssprachliche Uhr
 *     License: GPLv3 or later
 *     Depends:     global.h, io.h, stdio.h, pgmspace.h, interrupt.h
 *    	lcd.c, i2c.h, ht1632.h, uart.h
 *
 *     Authors: Copyright (C) Philipp Hörauf, Alexander Schuhmann
 *        Date: 2012-06-06
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define WORDCLOCK_MIRROR
#define POWER_LED PC0

/*
 * Sprechende Bezeichner für die Variable fixed
 */
#define RTC_FIX 0
#define RTC_FIRST_SYNC 1
#define RTC_OFF_PRESYNC 2


#include "../atmel/lib/0.1.3/global.h"
#include "../atmel/lib/0.1.3/io/io.h"
// #include <math.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

// USART - die serielle Schnittstelle
#define mega8
#include "../atmel/lib/0.1.3/io/serial/uart.h"

// LCD

#define LCD_SHIFT_PORT PORTC
#define LCD_SHIFT_DDR DDRC
#define LCD_SHIFT_DATA_PIN PC5
#define LCD_SHIFT_CLOCK_PIN PC3
#define LCD_SHIFT_LATCH_PIN PC4
#define LCD_EN_PIN 5
#define LCD_RS_PIN 4
#define LCD_D4_PIN 0
#define LCD_LINES 2
#define LCD_COLS 20
#define LCD_MODE_SHIFT_4BIT
#include "../atmel/lib/0.1.3/io/display/lcd/lcd.c"

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

// Grenzen, wann DCF-Signale als 0 oder 1 aufgefasst werden. (in ms*(TimerTicks/ms))
#define NULL_LOW  (uint16_t)156*45
#define NULL_HIGH (uint16_t)156*130
#define EINS_LOW  (uint16_t)156*140
#define EINS_HIGH (uint16_t)156*230

const uint16_t esIst [11] PROGMEM = {1024,1024,0,1024,1024,1024,0,0,0,0,0};
const uint16_t kurz [11] PROGMEM = {0,0,0,0,64,64,64,64,0,0,0};
const uint16_t minVor [11] PROGMEM = {0,0,0,0,0,0,0,512,512,512,0};
const uint16_t minNach [11] PROGMEM = {256,256,256,256,0,0,0,0,0,0,0};
const uint16_t minFuenf [11] PROGMEM = {0,0,0,0,0,0,0,1024,1024,1024,1024};
const uint16_t minZehn [11] PROGMEM = {512,512,512,512,0,0,0,0,0,0,0};
const uint16_t minViertel [11] PROGMEM = {0,0,0,0,256,256,256,256,256,256,256};
const uint16_t minHalb [11] PROGMEM = {128,128,128,128,0,0,0,0,0,0,0};

const uint16_t stdEin [11] PROGMEM = {64,64,64,0,0,0,0,0,0,0,0};
const uint16_t stdEins [11] PROGMEM = {64,64,64,64,0,0,0,0,0,0,0};
const uint16_t stdZwei [11] PROGMEM = {0,0,0,0,0,0,0,64,64,64,64};
const uint16_t stdDrei [11] PROGMEM = {32,32,32,32,0,0,0,0,0,0,0};
const uint16_t stdVier [11] PROGMEM = {0,0,0,0,0,0,0,32,32,32,32};
const uint16_t stdFuenf [11] PROGMEM = {16,16,16,16,0,0,0,0,0,0,0};
const uint16_t stdSechs [11] PROGMEM = {0,0,0,0,0,0,16,16,16,16,16};
const uint16_t stdSieben [11] PROGMEM = {8,8,8,8,8,8,0,0,0,0,0};
const uint16_t stdAcht [11] PROGMEM = {0,0,0,0,0,0,0,8,8,8,8};
const uint16_t stdNeun [11] PROGMEM = {0,4,4,4,4,0,0,0,0,0,0};
const uint16_t stdZehn [11] PROGMEM = {0,0,0,0,0,0,0,4,4,4,4};
const uint16_t stdElf [11] PROGMEM = {0,0,0,0,0,0,0,0,2,2,2};
const uint16_t stdZwoelf [11] PROGMEM = {2,2,2,2,2,0,0,0,0,0,0};

const uint16_t stdVor [11] PROGMEM = {0,0,0,0,128,128,128,0,0,0,0};
const uint16_t stdNach [11] PROGMEM = {0,0,0,0,0,0,0,128,128,128,128};
const uint16_t uhr [11] PROGMEM = {0,0,0,0,0,0,0,0,1,1,1};

uint16_t Allon [11] = {2047,2047,2047,2047,2047,2047,2047,2047,2047,2047,2047};
uint16_t funkuhr [11] = {0,0,0,0,577,577,833,321,257,1,1};

volatile uint8_t stundenValid = 0, minutenValid = 0, sekundenValid=0;;
uint16_t temp[11];
uint8_t DisplayOffTimer = 0;

uint8_t nachtmodus = 0;
uint16_t Syncnacht = 0;
uint8_t SyncNotNacht = 0;
uint8_t set_dimmen = 0;
uint8_t RTC_read_error = 0;
uint8_t last_sync_min = 0;
uint8_t last_sync_std = 0;

/*
 * 
 *  LED_ROT = Anzeige
 *  LED_GELB = Sync in der Nacht nicht erfolgreich
 *  LED_GRÜN = DCF-SIGNAL
 *
 *  UART-Einstellung: 57600 BAUD, 1 Stoppbits, 8 Bit Frame, no Parity
 *
 *
 *  DEBUG_TIMER:
 *  Ausgabe des erfassten Timerwertes in ms für den High-Pegel des DCF-Signals (roh) aus.
 *
 *  DEBUG_ERFASSUNG:
 *  gibt die erfassten DCF-Bits aus (60Bit Wort, LSB first)
 *
 *  DEBUG_DATENWERT:
 *  Ausgabe des Teils des erfassten Wertes, in dem die Uhrzeit steht.
 *  (binär, Reihenfolge wie DCF)
 *
 *  DEBUG_ZEIT
 *  Ausgabe der berechneten Uhrzeit
 *
 *  DEBUG_RTC
 *  gibt den Zeitwert der RTC und den aktuellen DCF-Wert in BCD-Code aus
 *  beide Werte stehen zum besseren Vergleich untereinander.
 *
 *  DEBUG_RTC_TEST
 *  RTC wird nicht geschrieben sondern nur gelesen, ideal zum Test der RTC-Genauigkeit.
 *  OBSOLET geworden durch erwieiterte RTC-Funktionalitäten
 *
 *  DEBUG_RTC_READ
 *  UART-Ausgabe beim halbsekündlichen Auslesen der RTC (viel Output!)
 *
 *  DEBUG_DISPLAY
 *
 *  DEBUG_LCD
 *  Ausgabe der Uhrzeit in Stunden, Minuten und Sekunden auf einem 2x20 Zeichen LCD
 *
 *  DISPLAY_SCROLL
 *  Matrix-ähnliche Lautschrift wenn die Uhr die Anzeige wechselt.
 *  gehört zum normalen Programm der Uhr.
 *
 *  RTC_NEU
 *  Ansteuerung der neuen RTC (DS3231)
 *  Enthält einen MEMS-Quarz und ist so einfacher als die DS1307 anzuschließen
 *
 *  RTC_3UHR_TEST
 *  Setzt die RTC am Anfang der Main auf kurz vor drei Uhr
 *
 *
 */

#define DEBUG_PIN PB2
#define DEBUG_PORT PINB

#define DEBUG_TIMER
#define DEBUG_ERFASSUNG
// #define DEBUG_DATENWERT
#define DEBUG_ZEIT
#define DEBUG_RTC
// #define DEBUG_RTC_TEST
// #define RTC_RESET
#define RTC_3UHR_TEST
#define DEBUG_RTC_READ
// #define DEBUG_DISPLAY
// #define DEBUG_LCD
#define DISPLAY_SCROLL

// #define dimmen





void dcfInit(void) {
	DDRD | (1<<PD3); // PON
	TCCR1B = (1<<CS12) | (1<<CS10); // Prescaler = 1024
	GICR = (1<<INT0);   // INT0 ist ab hier ein Interrupt-Pin
	MCUCR = (1<<ISC00); // INT0 Logic Change Interrupt
}

inline void dcfOn(void) { //Aktivieren des DCF77 Moduls (PON)
sbi(PORTD, PD3);
}

inline void dcfOff(void) { //Deaktivieren des DCF77 Moduls (PON)
cbi(PORTD, PD3);
}

void rtcInit(void) {
	i2c_tx(0b00100100,0xE,0b11010000); // Aktiviere Oszillator
	delayms(10);
	i2c_tx(0x00,0x02,0b11010000); // Stelle 24-Stunden-Modus ein
	delayms(10);
}

void uart_init(void) {
	UBRRL = 10;		// 57,6k Baud, 2 Stoppbits (Achtung: -1,4% Fehler....)
	UCSRB = (1<<RXEN)  | (1<<TXEN);
	UCSRC = (1<<URSEL) | (1<<USBS) | (1<<UCSZ1) | (1<<UCSZ0);
}

volatile uint8_t empfangFehler = 0, fixed = RTC_OFF_PRESYNC;


void addArray(const uint16_t *Array) {
	
	for(uint8_t i=0; i<11; i++) {
		temp[i] += pgm_read_word(&(Array[i]));
	}
}

void clearTemp(void) {
	for(uint8_t i=0; i<11; i++) {
		temp[i] = 0;
	}
}

void timeToArray(void) {
	if(minutenValid%5 == 0) { // minuten muss durch 5 Teilbar sein.
		clearTemp();
		addArray(esIst); // Grundsätzlich soll "Es ist" angezeigt werden
		switch (minutenValid) {
			case 5:
				addArray(minFuenf);
				addArray(minNach);
				break;
			case 10:
				addArray(minZehn);
				addArray(stdNach);
				break;
			case 15:
				addArray(minViertel);
				addArray(stdNach);
				break;
			case 20:
				addArray(minZehn);
				addArray(minVor);
				addArray(minHalb);
				stundenValid++;
				break;
			case 25:
				addArray(minFuenf);
				addArray(minVor);
				addArray(minHalb);
				stundenValid++;
				break;
			case 30:
				addArray(minHalb);
				stundenValid++;
				break;
			case 35:
				addArray(minFuenf);
				addArray(minNach);
				addArray(minHalb);
				stundenValid++;
				break;
			case 40:
				addArray(minZehn);
				addArray(minNach);
				addArray(minHalb);
				stundenValid++;
				break;
			case 45:
				addArray(minViertel);
				addArray(stdVor);
				stundenValid++;
				break;
			case 50:
				addArray(minZehn);
				addArray(minVor);
				stundenValid++;
				break;
			case 55:
				addArray(minFuenf);
				addArray(minVor);
				stundenValid++;
				break;
			case 0:
				addArray(uhr);
				break;
		}
		switch (stundenValid) {
			case 1:
			case 13:
				if (minutenValid != 0)
				{
					addArray(stdEins);
				}
				else
				{
					addArray(stdEin);
				}
				break;
			case 2:
			case 14:
				addArray(stdZwei);
				break;
			case 3:
			case 15:
				addArray(stdDrei);
				break;
			case 4:
			case 16:
				addArray(stdVier);
				break;
			case 5:
			case 17:
				addArray(stdFuenf);
				break;
			case 6:
			case 18:
				addArray(stdSechs);
				break;
			case 7:
			case 19:
				addArray(stdSieben);
				break;
			case 8:
			case 20:
				addArray(stdAcht);
				break;
			case 9:
			case 21:
				addArray(stdNeun);
				break;
			case 10:
			case 22:
				addArray(stdZehn);
				break;
			case 11:
			case 23:
				addArray(stdElf);
				break;
			case 24:
			case 12:
			case 0:
				addArray(stdZwoelf);
				break;
		}
		if ((minutenValid == 0) || (minutenValid == 5) || (minutenValid == 10) || (minutenValid == 15)) {
		} else {
			stundenValid--; // setze stundenValid wieder auf richtigen Wert
		}
		
		htWriteDisplay(temp);
	}
}


ISR(INT0_vect,ISR_BLOCK) { // Pinchange-Interrupt an INT0 (DCF-Signal IN)
// Dieser Interrupt verarbeitet das DCF-Signal, also High- und Lowphasen!!

static uint32_t datenwert=0;
static uint8_t i=0, sync=0, timeValid=0;
static uint8_t minutenDcfAktuell=0, stundenDcfAktuell=0;
static uint8_t minutenDcfLast=0, stundenDcfLast=0;

uint16_t  timerwert_alt = TCNT1;
TCNT1 = 0; // leere Timer-Speicherwert
delayms(1);

if (DEBUG_PORT &= (1 << DEBUG_PIN)) { //Test ob Debug Stecker gelöst
	PORTD ^= (1<<PD7); //LED an PD7 zeigt das DCF77 Signal an
}

if(PIND & (1<<PD2)) { // gerade ist HIGH
	TCCR1B = (1<<CS11) | (1<<CS10); // Prescaler = 64
	TCNT1 = 156;
	
	if(timerwert_alt > 14000) {
		sync = 1;
		i=0;
		datenwert = 0;
		timeValid = 0;
		#ifdef DEBUG_ERFASSUNG
		uart_tx_newline();
		#endif
		
		if((minutenDcfLast+1 == minutenDcfAktuell) && (stundenDcfLast == stundenDcfAktuell)) {
			// Vergleich und Setzen der Valid-Uhrzeitvariablen
			timeValid = 1;
		} else if ((minutenDcfLast == 59) && (minutenDcfAktuell == 0) && (stundenDcfLast +1 == stundenDcfAktuell)) {
			timeValid = 1;
		} else if((minutenDcfLast == 59) && (minutenDcfAktuell == 0) && ((stundenDcfLast == 23) && (stundenDcfAktuell == 0))) {
			timeValid = 1;
		} else { // ACHTUNG FEHLER!! (oder erster Empfang nach Inbetriebnahme)
			empfangFehler++;
			#ifdef DEBUG_ZEIT
			uart_tx_strln("Fehler bei Zeitübernahme!");
			#endif
		}
		if(timeValid) { // Zeit ist valide: RTC wird gesetzt
			minutenValid = minutenDcfAktuell;
			stundenValid = stundenDcfAktuell;
			}
		minutenDcfLast = minutenDcfAktuell;
		stundenDcfLast = stundenDcfAktuell;
		#ifdef DEBUG_ZEIT
		uart_tx_str("aktuelle DCF-Zeit: ");
		uart_tx_dec(stundenDcfAktuell);
		uart_tx(':');
		uart_tx_dec(minutenDcfAktuell);
		uart_tx_newline();
		#endif
	}
	
} else { // gerade ist LOW
	TCCR1B = (1<<CS12) | (1<<CS10); // Prescaler = 1024
	TCNT1 = 10;
	#ifdef DEBUG_TIMER
	uart_tx_newline();
	uart_tx_str("Zeit: ");
	uart_tx_dec(timerwert_alt/156);
	uart_tx_str(" ms");
	uart_tx_newline();
	#endif
	if(sync) {
		if((NULL_LOW < timerwert_alt) && (timerwert_alt < NULL_HIGH)) { // und wir haben eine NULL
			i++;
			; // nullen stehen schon da.
			#ifdef DEBUG_ERFASSUNG
			uart_tx('0');
			cbi(PORTB,PB1);
			#endif
		} else if((EINS_LOW < timerwert_alt) && (timerwert_alt < EINS_HIGH)) { // und wir haben eine EINS
			#ifdef DEBUG_ERFASSUNG
			uart_tx('1');
			sbi(PORTB,PB1);
			#endif
			if((i>=15) && (i<47)) {
				datenwert |= ((uint32_t)1<<(31-(i-15)));
			}
			i++;
		}
		if(i == 50) { // Zeit zum Auswerten des Datenwortes
			#ifdef DEBUG_DATENWERT
			uart_tx_newline();
			uart_tx_strln("Datenwert: ");
			uart_tx_bin(datenwert);
			uart_tx_newline();
			#endif
			
			/*
			 *		Die Uhrzeit wird aus den verschiedenen Speicherorten im DCF-Signal
			 *		herausgelesen und in einen einzelnen Wert für Minuten und
			 *		Sekunden verwandelt, die in je einer Variablen gesichert werden.
			 *		Nähere Doku zum DCF-Signal: wikipedia lesen, wie ich auch.
			 */
			
			// Minuten Einerstelle
			minutenDcfAktuell =  (1 & (datenwert>>25)) + 2*(1 & (datenwert>>24)) + 4*(1 &(datenwert>>23)) + 8*(1 & (datenwert>>22));
			// Minuten Zehnerstelle
			minutenDcfAktuell += 10*((1 & datenwert>>21)) + 20*((1 & datenwert>>20)) + 40*((1 & datenwert>>19));
			
			// Stunden Einerstelle
			stundenDcfAktuell =  (1 & (datenwert>>17)) + 2*(1 & (datenwert>>16)) + 4*(1 &(datenwert>>15)) + 8*(1 & (datenwert>>14));
			// Stunden Zehnerstelle
			stundenDcfAktuell += 10*((1 & datenwert>>13)) + 20*((1 & datenwert>>12));
		}
	}
	if(i == 58) {
		i=0; // eine Minute ist vergangen. SYNC muss neu aufgezogen werden. (und die Minutenlücke kommt auch)
	}
}
}

void time2LCD(void) {
	char temp[21] = "                    "; // leerer string
	uint8_t nullzeichen = sprintf(temp, "Zeit: %u:%u:%u",stundenValid,minutenValid,sekundenValid);
	temp[nullzeichen] = ' ';
	lcd_set_position(20);
	lcd_putstr(temp);
}

void delays(uint8_t delay){
	for (;delay>0;delay--){
		delayms(1000);
	}
}




int main (void) {
	DDRB  |= (1<<PB1);
	DDRC  |= (1<<PC0) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);
	DDRD  |= (1<<PD1) | (1<<PD5) | (1<<PD6) | (1<<PD7); //Definieren der drei LEDs und des UART-TX als Output
	PORTB |= (1<<PB2); // Pullup DEBUG-Jumper
	PORTC |= (1<<PC0) | (1<<PC2) | (1<<PC3) | (1<<PC5); // Power LED, leuchtet; CS, RD, WR sind idle HIGH
	PORTD |= (1<<PD3);
	
	
	delayms(100);
	uart_init();		// Akivierung Kommuniaktion UART 56,8kBaud 8n1
	dcfInit();		// Pon als Output, Setzen der Timereigenschaften
	i2c_init();		// Pullup SDA, Initzialisierung I2C
// 	rtcInit();		// Starte Oszillator der RTC
	
	uart_tx_strln("DCF-Wordclock!"); //
	htInit();
	rtcInit();
	htWriteDisplay(funkuhr);
	delayms(10000);
	htDisplOff();
	
	dcfOn();
	delayms(100);
	sei();
	
	
	while(1) {
		
		#ifdef dimmen // Hier wird der Code für das Dimmen des LCDs definiert
		if (((stundenValid > 22) | (stundenValid < 7)) & (set_dimmen == 0))	{
			htCommand(0b10100101);
			set_dimmen = 1;
		}
		if ((stundenValid < 22) & (stundenValid > 7) & (set_dimmen == 1)){
			htCommand(0b10111101);
			set_dimmen = 0;
		}
		#endif
		
		//Es ist drei Uhr Nachts, Uhr läuft auf RTC (fixed == RTC_FIX ) und der Nachtmodus ist noch nicht aktiv
		if ((stundenValid == 3) && (fixed == RTC_FIX) && (nachtmodus == 0)) {
			//Hier werden die Lesefehler in die RTC geschrieben 	
			htDisplOff(); // Display wird für Sync ausgeschaltet
			fixed = RTC_OFF_PRESYNC; 
			nachtmodus = 1;
			dcfOn();
			sei(); // Und es seien wieder Interrupts!
		} 
		
		//Es ist sechs Uhr Nachts, das DCF Modul wird abgeschaltet, KEIN SYNC erfolgt!
		if ((stundenValid == 6) &&  (nachtmodus == 1)){
			dcfOff();
			fixed = RTC_FIX; // Uhr wieder aktiv - RTC-Modus
			cli(); // Interrupts wieder aus - nach 6 Uhr früh kein DCF Empfang mehr.
			//Hier wird der Fehlerwert von Sync_Fehler Nacht  erhöht und geschrieben
			nachtmodus = 0; // zurück zu normalem Betrieb
		}
		
		// Bedinung zum Testen der Nachabschaltung
		// Bedingt die 500 Sekündige Aktualisierung der angezeigten Uhrzeit
		if ((nachtmodus == 1) && (fixed == RTC_OFF_PRESYNC)) {
			if (DisplayOffTimer < 18)	{
				DisplayOffTimer++;
			}
			if (DisplayOffTimer == 18) { 
				htDisplOn(); //Display wird nach zirka 1200 Sekunden wieder angeschaltet
			}
			cli();
			timeToArray();
			time2LCD();
			sei();
			delays(200); //TODO: 200 Sekunden Wartezeit = 3,3 Minuten * 180 = 600 Minuten = 10 Stunden !! Eindeutig zu lange !
		}
		
		if (fixed == RTC_OFF_PRESYNC) {
			PORTC ^= (1<<PC0);
			// Solange kein valides DCF77 Signal blinkt PowerLED (Extern per Draht angeschlossen)
		}
		
		if (fixed == RTC_FIRST_SYNC) { // Darf nur nach einem erfolgreichen DCF-Sync durchlaufen
			// maximal ein mal am Tag, Nach dem Nachtmodus oder beim ersten anschalten.
			
			cli(); //Deaktivieren der Interrupts
			
			cbi(PORTC,PC0); // Signal empfangen, PowerLED aus
			
			dcfOff(); //Deaktivieren des DCF Moduls nach erfolgreichem Sync
			
			if (nachtmodus == 1) {
				Syncnacht++;
				nachtmodus = 0;
			}
			
			
			cbi(PORTD, PD6); //LED an PD6 wird ausgeschaltet (Erfolgreicher Sync in der Nacht)
			sbi(PORTC, PC0); //PowerLED an - RTC Modus erreicht
			
			fixed = RTC_FIX; // beim wiederanschalten der DCF-Uhr wird MinutenValid und StundenValid durch die RTC aktualisiert.
			
			minutenValid = (minutenValid - (minutenValid%5));
			// Damit die Uhr direkt nach dem Sync eine Uhrzeit anzeigt und nicht erst
			// bei den nächsten vollen 5 Minuten.
			// Änderung wird gleich danach wieder durch rtcRead() zurückgesetzt.
			
			timeToArray(); // momentane Zeit muss direkt in das HT-Display geschrieben werden!
			htDisplOn();   // Aktivieren der Clockgeneratoren für die LEDs des HT1632 Controllers
		}
		
		if (fixed == RTC_FIX) { // Vermeidung der Überschneidung mit dem Interrupt

			
			#ifdef DISPLAY_SCROLL
			if ((sekundenValid == 59) && ((minutenValid) % 5 == 4)) {
				for (uint8_t shift = 0;shift<11;shift++) { // Matrix-like scrollen

					for (uint8_t x = 0;x<11;x++) {
						temp[x]= temp[x] >> 1;
					}
					
					htWriteDisplay(temp);
					delayms(100);
				}
			}
			#endif
			
			
			//Simulation einer Uhr, die RTC ist ohne Nutzen, es werden intern alle Uhrenwerte durchlaufen.

			if (minutenValid == 59)
			{
			minutenValid = 0;
			  if (stundenValid == 23)
			  {
			    stundenValid = 0;
			  }
			  else{
			    stundenValid++;
			  }
			  
			}
			minutenValid++;
			
			timeToArray(); // wird nur ausgeführt, wenn die Uhr einen sicheren, kalibrierten Zeitgeber hat. (RTC fixed)
			

		}
		delayms(100);
	}
	return 0;
}

