// Definitionen für die Wordclock - von User zu setzen!

// RTC_SET_CLOCK
// Benötigt zwei defines (RTC_SET_MIN, RTC_SET_STD)
#define RTC_SET_MIN 59
#define RTC_SET_STD 2

//Allgemeiner Defines-Block
#include "../AtmelLib/global.h"
#include "../AtmelLib/io/io.h"
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define HFUSE C9
#define LFUSE 9F

// Einstellungen für Funktionalität und Verhalten:
#include "./config.h"

/* Bitte die Datei config.example.h nach config.h kopieren und dann dort die spezifischen Einstellungen setzen.
 *
 * #define HW_0_4
 * Für die aktuelle Hardwareversion. Zur Zeit werden keine anderen unterstützt.
 * Wird auch in der Debug-Anzeige angezeigt.
 * 
 * # define DISP_xxxx
 * um das Display-Layout auszuwählen. Es gibt:
 * Dieses Layout muss zum aufgebauten Display passen!
 *
 * DISP_NORM:
 * 
 * ESKISTAFÜNF
 * ZEHNDAUVORG
 * NACHVIERTEL
 * HALBVORNACH
 * EINSKURZWEI
 * DREIAUJVIER
 * FÜNFTOSECHS
 * SIEBENLACHT
 * ANEUNMHZEHN
 * ZWÖLFDTFELF
 * WASDFUNKUHR
 * 
 * 
 * DISP_FRANKEN:
 * 
 * ESKISTLFÜNF
 * DAUERTXKURZ
 * ZENHZWANZIG
 * DREIVIERTEL
 * NACHMVORJGZ
 * HALBFUNKUHR
 * DRZWÖLFÜNFX
 * ZEHNEUNDREI
 * ZWEINSIEBEN
 * ELFVIERACHT
 * SECHSIUHRYE
 * 
 * 
 * DISP_HORM:
 * 
 * Spezialversion für die Gemeinde "Hormersdorf" und deren sehr spezielle
 * Art, die Uhrzeit zu sagen. Mehr Info unter:
 * http://www.hormersdorf-erzgebirge.de/mundart.html
 *
 * xxx
 * 
 * 
 * Weitere Schalter nach Bedarf:
 * 
 * #define DISPLAY_SCROLL
 * Matrix-ähnliche Lautschrift wenn die Uhr die Anzeige wechselt.
 * Dies gehört zum normalen Programm der Uhr.
 * Zum Deaktivieren Zeile weglassen.
 *
 * #define DISPLAY_DIMMEN
 * #define DIMMEN_START 22
 * #define DIMMEN_END 3
 * Aktiviert das Dimmen des Displays zwischen den Uhrzeiten DIMMEN_START und DIMMEN_END
 * Zum Deaktivieren Zeilen weglassen.
 *
 * 
 * Display-Spiegelung: Wenn man die Verkabelung falschrum gemacht hat, folgende Parameter (oder beide) ausprobieren:
 * 
 * #define WORDCLOCK_MIRROR
 * Spiegelt die Matrixspalten (?) des Displays ( Je nachdem wie man die Verkabelung anfängt)
 * Zum Deaktivieren Zeile weglassen.
 * 
 * #define WORDCLOCK_MIRROR_ZEILEN
 * Spiegelt die Matrixzeilen des Displays ( Je nachdem wie man die Verkabelung anfängt)
 * Zum Deaktivieren Zeile weglassen.
 *
 * 
 * Einstellparameter:
 * 
 * #define nightSyncTime 6
 * Definiert in 10 Minuten Schritten wie lange das Display in der Nacht aus ist, z.B. 6 entspricht 60 Minuten.
 * Nur während dieser Zeit kann DCF-Empfang stattfinden, da das LED-PWM den Empfang stört. Also ist 0 nicht erlaubt!
 * 
 * 
 * DEBUG-Einstellungen (auch in der config.h zu setzen)
 * 
 * LED_ROT = Anzeige
 * LED_GELB = Sync in der Nacht nicht erfolgreich
 * LED_GRÜN = DCF-SIGNAL
 *
 * UART-Einstellung: 57600 BAUD, 1 Stoppbits, 8 Bit Frame, no Parity
 *
 *
 * DEBUG_TIMER:
 * Ausgabe des erfassten Timerwertes in ms für den High-Pegel des DCF-Signals (roh) aus.
 *
 * DEBUG_ERFASSUNG:
 * gibt die erfassten DCF-Bits aus (60Bit Wort, LSB first)
 *
 * DEBUG_DATENWERT:
 * Ausgabe des Teils des erfassten Wertes, in dem die Uhrzeit steht.
 * (binär, Reihenfolge wie DCF)
 *
 * DEBUG_ZEIT
 * Ausgabe der berechneten Uhrzeit, wenn die DCF-Uhr synct.
 * 
 * DEBUG_ZEITAUSGABE
 * Gibt die jetzt-Zeit aus, wenn die Uhr auf RTC läuft.
 * ACHTUNG: Ausgabe jede Sekunde!
 *
 * DEBUG_RTC
 * gibt den Zeitwert der RTC und den aktuellen DCF-Wert in BCD-Code aus
 * beide Werte stehen zum besseren Vergleich untereinander.
 * 
 * DEBUG_RTC_RESET
 * Setzt die RTC auf jeden Fall zurück, auch wenn schon eine valide
 * Uhrzeit drinsteht. Für erzwungenen DCF-Empfang verwenden.
 *
 * RTC_3UHR_TEST
 * Setzt die RTC am Anfang der Main auf kurz vor drei Uhr
 * 
 * DEBUG_RTC_READ
 * UART-Ausgabe beim dauerhaften Auslesen der RTC (viel Output!)
 * genaue Anzeige der aktuellen Uhrzeit ist gewährleistet
 *
 * DEBUG_DISPLAY
 * Alle LEDs des Displays sollten jetzt leuchten, danach Uhr-Funktionen
 * Wichtig, um Lötfehler ausschließen zu können
 *
 */


// Globale Variablen:
volatile uint8_t status = 0;
volatile uint8_t stundenValid = 0, minutenValid = 0, sekundenValid = 0;
uint16_t temp[11]; // TODO: eindeutigen Namen einführen
volatile uint16_t dcfSyncSuccess=0, dcfSyncErrors=0, rtcI2CErrors=0, rtcHighlevelErrors=0;
volatile uint8_t dcfLastSyncHour=99, dcfLastSyncMinute=99;

// Grenzen, wann DCF-Signale als 0 oder 1 aufgefasst werden. (in ms*(TimerTicks/ms))
#define NULL_LOW  (uint16_t)156*45
#define NULL_HIGH (uint16_t)156*130
#define EINS_LOW  (uint16_t)156*140
#define EINS_HIGH (uint16_t)156*230


// USART - die serielle Schnittstelle
#define mega8
#define UBRRH_VALUE 0
#define UBRRL_VALUE 11
#include "../AtmelLib/io/serial/uart.h"

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

// Definiere Hardwareversion und Displaylayout nach config.h
#ifdef HW_0_4
#include "./HW_0_4.h"
#define HW_VERSION "0_4"
#endif
#ifdef DISP_FRANKEN
#include "./disp_franken.h"
#define DISP "FRANKEN"
#endif
#ifdef DISP_HORM
#include "./disp_horm.h"
#define DISP "HORM"
#endif
#ifdef DISP_NORM
#include "./disp_norm.h"
#define DISP "NORM"
#endif

//Einbinden der RTC Funktionen
uint8_t RTC_read_error = 0;
#include "./rtc.h"

//Status der Wordclock (Wert der Variable "Status")
#define INIT 0
#define PRE_DCF_SYNC 2
#define DCF_SYNC 3
#define POST_DCF_SYNC 4
#define RTC_VALID 5
#define DREIUHR_PRE_DCF_SYNC 6
#define DREIUHR_DCF_SYNC 7
#define DREIUHR_POST_DCF_SYNC 8
#define PRE_RTC_VALID 9
#define WRITE_DISP 10
#define RTC_CHECK_VALID 11
