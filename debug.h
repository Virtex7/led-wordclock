// debug.h

/*  DEBUG SECTION 
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
 *  Ausgabe der berechneten Uhrzeit, wenn die DCF-Uhr synct.
 * 
 *  DEBUG_ZEITAUSGABE
 *  Gibt die jetzt-Zeit aus, wenn die Uhr auf RTC läuft.
 *  ACHTUNG: Ausgabe jede Sekunde!
 *
 *  DEBUG_RTC
 *  gibt den Zeitwert der RTC und den aktuellen DCF-Wert in BCD-Code aus
 *  beide Werte stehen zum besseren Vergleich untereinander.
 * 
 *  DEBUG_RTC_RESET
 *  Setzt die RTC auf jeden Fall zurück, auch wenn schon eine valide
 *  Uhrzeit drinsteht. Für erzwungenen DCF-Empfang verwenden.
 *
 *  RTC_3UHR_TEST
 *  Setzt die RTC am Anfang der Main auf kurz vor drei Uhr
 * 
 *  DEBUG_RTC_READ
 *  UART-Ausgabe beim dauerhaften Auslesen der RTC (viel Output!)
 *  genaue Anzeige der aktuellen Uhrzeit ist gewährleistet
 *
 *  DEBUG_DISPLAY
 *  Alle LEDs des Displays sollten jetzt leuchten, danach Uhr-Funktionen
 *  Wichtig, um Lötfehler ausschließen zu können
 *
 */

#define DEBUG_PIN PB2
#define DEBUG_PORT PINB

// #define DEBUG_TIMER
#define DEBUG_ERFASSUNG
// #define DEBUG_DATENWERT
#define DEBUG_ZEIT
#define DEBUG_ZEITAUSGABE
#define DEBUG_RTC
// #define RTC_RESET
// #define RTC_SET_CLOCK
// #define RTC_NO_DCF
#define DEBUG_RTC_READ
#define DEBUG_DISPLAY
