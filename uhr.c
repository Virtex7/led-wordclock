/*
 *    Filename: uhr.c
 *     Version: siehe DEFINE
 * Description: Ansteuerung für eine umgangssprachliche Uhr
 *     License: GPLv3 or later
 *     Depends: defines.h
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
 *
 * Hinweis zur aktuellen Codeversion > 0.3.1
 * 
 * Dieser Quellcode macht sich viele Funktionalität der RTC zu Nutze. Der
 * Atmel wird hierbei hauptsächlich schlafen, und nur jede Minute kurz geweckt.
 */

// Einstellungen und globale Variablen sowie weitere Includes
#include "./defines.h"

// builddate und Soft-Version hier lassen.
#define SOFT_VERSION "0.3.9"
#define BUILDDATE "21.06.2013"


int main(void){
	
	while(1) {
		
		if (status == INIT) {
			init(); //Je nach Hardware Rev. unterschiedliche Init --> siehe HW_x_x.h
			//Nächster Ablauf 
			status = RTC_CHECK_VALID;
			
			#ifdef RTC_SET_CLOCK
			uartTxStrln("setze RTC auf 2:59 Uhr");
			rtcWrite(RTC_SET_STD,RTC_SET_MIN);
			#endif
			
			#ifdef RTC_NO_DCF
			rtcWrite(3,59);
			uartTxStrln("setze RTC auf 3:59 Uhr");
			#endif
			
			#ifdef RTC_RESET
			uartTxStrln("setze RTC zurück...");
			i2c_tx(0b00000000,0x07,0b11010000);
			#endif
		}
		
		if (status == RTC_CHECK_VALID) {
			if ((i2c_rx_DS1307(0x7) == 0b01011101)) {
				//Register 0x7 wird von der RTC abgefragt. Hier wurde vorher bei valider Uhrzeit eine bestimmte Bitmusterfolge gesetzt (0101 1101).
				rtcRead();
				status = PRE_RTC_VALID;
				#ifdef DEBUG_RTC
				uart_tx_strln("RTC hat valide Daten - überspringe DCF-Empfang.");
				#endif
			} 
			else {
				status = PRE_DCF_SYNC;
			}
		}
		
		if (status == PRE_RTC_VALID) {
			//INT1 wird initialisiert
			GICR |= (1<<INT1);   // INT1 ist ab hier ein Interrupt-Pin
			MCUCR |= (1<<ISC11) | (1<<ISC10); // INT1 Logic Change Interrupt
			sei();
			
			// einmal ausführen, damit die Zeit möglichst schnell auf dem Display steht.
			minutenValid-=(minutenValid%5); // setze auf 5-Minuten-Step, damit Uhrzeit angezeigt wird.
			timeToArray(); // wird nur ausgeführt, wenn die Uhr einen sicheren, kalibrierten Zeitgeber hat. (RTC fixed)
			htDisplOn(); // Aktiviere Display
			rtcRead(); // korrigiere falsch gesetzten minutenValid-Wert durch RTC-Auslesung
			
			#ifdef DEBUG_RTC
			uartTxStrln("Uhr läuft, aktiviere Display!");
			#endif
			status = RTC_VALID;
		}
		
		if (status == PRE_DCF_SYNC) {
			#ifdef DEBUG_RTC
			uart_tx_strln("Aktiviere RTC und DCF-Uhr");
			#endif
			rtcInit();
			dcfInit();
			GICR |= (1<<INT0);   // INT0 ist ab hier ein Interrupt-Pin
			MCUCR |= (1<<ISC00); // INT0 Pinchange Interrupt aktiv
			
			// Für 10 Sekunden wird Funkuhr angezeigt
			#ifdef DEBUG_DISPLAY
			htWriteDisplay(Allon);
			delayms(10000);
			#endif
			htWriteDisplay(funkuhr);
			delayms(10000);
			htDisplOff();
			uart_tx_strln("DCF77 auf Empfang!");
			dcfOn();
			delayms(100);
			status = DCF_SYNC;
			sei(); // und es seien Interrupts!
		}
		
		if (status == DCF_SYNC) {
			PORTC ^= (1<<PC0);
			delayms(500);
			// Solange kein valides DCF77 Signal blinkt PowerLED (Extern per Draht angeschlossen)
		}
		
		if (status == POST_DCF_SYNC) {
			cbi(GICR, INT0); // INT0 Interrupt deaktivert
			dcfOff(); // schalte DCF-Modul aus
			rtcSetValid(); // Schreibe Validierungswert in die RTC
			status = PRE_RTC_VALID;
		}
		
		if (status == RTC_VALID) {
			#ifdef RTC_NO_DCF
			static uint8_t einmal = 0;
			if (einmal == 0) {
				uartTxStrln("setze Zeit auf 3:59 Uhr");
				status = DREIUHR_PRE_DCF_SYNC;
				einmal = 1;
			}
			#endif
		}
		
		if (status == WRITE_DISP) {
			#ifdef DISPLAY_DIMMEN // Hier wird der Code für das Dimmen der LEDs definiert
			if (((stundenValid > DIMMEN_START) | (stundenValid < DIMMEN_END)) & (set_dimmen == 0))	{
				htCommand(0b10100101);
				set_dimmen = 1;
			}
			if ((stundenValid < DIMMEN_START) & (DIMMEN_END > 7) & (set_dimmen == 1)){
				htCommand(0b10111101);
				set_dimmen = 0;
			}
			#endif		
			#ifdef DISPLAY_SCROLL
			if ((sekundenValid == 59) && ((minutenValid) % 5 == 4)) {
				for (uint8_t shift = 0;shift<11;shift++) { // Matrix-like scrollen
				for (uint8_t x = 0;x<11;x++) {
					temp[x]= temp[x] >> 1;
				}
				htWriteDisplay(temp);
				delayms(85);
				}
			}
			#endif
			
			#ifdef DEBUG_RTC
			if ((minutenValid %5 == 0) && (sekundenValid == 05)) {
				uart_tx_str("Software-Revision: ");
				uart_tx_strln(SOFT_VERSION);
				
				//TODO: Fehlervariablen ausgeben!
// 				uartTxStrln("letzte Fehler bei Synchro, usw.");
			}
			#endif
			
			if (sekundenValid <=2) {
				rtcRead();
				timeToArray(); // wird nur ausgeführt, wenn die Uhr einen sicheren, kalibrierten Zeitgeber hat. (RTC fixed)
			}
			
			#ifdef DEBUG_ZEITAUSGABE
			uartTxDec2(stundenValid);
			uartTxStr(":");
			uartTxDec2(minutenValid);
			uartTxStr(":");
			uartTxDec2(sekundenValid);
			uartTxStrln(" Uhr");
			#endif
			
			// zurück zu RTC_VALID, damit der Interrupt1 wieder was bringt.
			status = RTC_VALID;
			
			// lege Atmel bis zum nächsten Interrupt schlafen
			set_sleep_mode(SLEEP_MODE_IDLE);
			sleep_enable();
			sei();
			sleep_cpu();
			sleep_disable();
		}
		
		if (status == DREIUHR_PRE_DCF_SYNC) {
			dcfOn();
			htDisplOff();
			delayms(100);
			sbi(GICR, INT0); // INT0 Interrupt aktivert (DCF Empfang)
			status = DREIUHR_DCF_SYNC;
		}
		
		if (status == DREIUHR_DCF_SYNC) {
			if (sekundenValid == 0) {
				rtcRead();
				#ifdef DEBUG_ZEITAUSGABE
				uartTxDec2(stundenValid);
				uartTxStr(":");
				uartTxDec2(minutenValid);
				uartTxStr(":");
				uartTxDec2(sekundenValid);
				uartTxStrln(" Uhr");
				delayms(1000);
				#endif
			}
// 			if (min_increase >40) {
// 				min_increase = 0;
// 				status = DREIUHR_POST_DCF_SYNC;
// 			}
		}
		
		if (status == DREIUHR_POST_DCF_SYNC) {
			// Wenn er hierhin springt hat er in der Nacht kein Zeitsignal erhalten 
			debug_sync_nacht_error++;
			cbi(GICR, INT0); // INT0 Interrupt deaktivert
			dcfOff(); // deaktivere DCF-Modul
			status = PRE_RTC_VALID;
		}
	}
	return 0;
}



ISR (INT0_vect, ISR_BLOCK) { // Pinchange-Interrupt an INT0 (DCF-Signal IN)
// Dieser Interrupt verarbeitet das DCF-Signal, also High- und Lowphasen!!

static uint32_t datenwert=0;
static uint8_t i=0, sync=0, timeValid=0;
static uint8_t minutenDcfAktuell=0, stundenDcfAktuell=0;
static uint8_t minutenDcfLast=0, stundenDcfLast=0;

uint16_t  timerwert_alt = TCNT1;
TCNT1 = 0; // leere Timer-Speicherwert
delayms(1);

PORTD ^= (1<<PD7); //LED an PD7 zeigt das DCF77 Signal an


if (PIND & (1<<PD2)) { // gerade ist HIGH
	TCCR1B = (1<<CS11) | (1<<CS10); // Prescaler = 64
	TCNT1 = 156;
	
	if (timerwert_alt > 14000) {
		sync = 1;
		i=0;
		datenwert = 0;
		timeValid = 0;
		#ifdef DEBUG_ERFASSUNG
		uart_tx_newline();
		#endif
		
		if ((minutenDcfLast+1 == minutenDcfAktuell) && (stundenDcfLast == stundenDcfAktuell)) {
			// Vergleich und Setzen der Valid-Uhrzeitvariablen
			timeValid = 1;
		} else if ((minutenDcfLast == 59) && (minutenDcfAktuell == 0) && (stundenDcfLast +1 == stundenDcfAktuell)) {
			timeValid = 1;
		} else if ((minutenDcfLast == 59) && (minutenDcfAktuell == 0) && ((stundenDcfLast == 23) && (stundenDcfAktuell == 0))) {
			timeValid = 1;
		} else { // ACHTUNG FEHLER!! (oder erster Empfang nach Inbetriebnahme)
			empfangFehler++;
			#ifdef DEBUG_ZEIT
			uart_tx_strln("Fehler bei Zeitübernahme!");
			#endif
		}
		if ((minutenDcfAktuell > 59) || (stundenDcfAktuell > 23)) { // offensichtlich Schmarrn.
			timeValid = 0;
			#ifdef DEBUG_ZEIT
			uart_tx_strln("Zeit über 23 Uhr und/oder 59 Minuten. WTF??");
			#endif
		}
		if (status > 5)//Hier wird überprüft ob die empfangene Uhrzeit nicht von der hinterlegten Uhrzeit abweicht. Die erste Zeile überprüft ob wir nicht im Anfangssync sind
		{
			//Achtung ! Dieser Code funktioniert nur im Zeitfenster um 3 Uhr herum
			if ((stundenDcfAktuell - stundenValid) > 2){
				timeValid = 0;
			}
		}
		if (timeValid) { // Zeit ist valide: RTC wird gesetzt
			minutenValid = minutenDcfAktuell;
			stundenValid = stundenDcfAktuell;
			rtcWrite(stundenValid,minutenValid);
			status = POST_DCF_SYNC;
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
	
} else { // gerade ist LOW ---> Auswertung!
	TCCR1B = (1<<CS12) | (1<<CS10); // Prescaler = 1024
	TCNT1 = 10;
	#ifdef DEBUG_TIMER
	uart_tx_newline();
	uart_tx_str("Zeit: ");
	uart_tx_dec(timerwert_alt/156);
	uart_tx_str(" ms");
	uart_tx_newline();
	#endif
	if (sync) {
		if ((NULL_LOW < timerwert_alt) && (timerwert_alt < NULL_HIGH)) { // und wir haben eine NULL
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

// Die RTC erzeugt jede Sekunde eine Aktualisierung der sekundenValid-Variable
// Muss alle Sekunden sein, damit die Scroll-Funktion geht!
ISR (INT1_vect, ISR_BLOCK) {
	sekundenValid++;
	
	if (sekundenValid == 60) {
		sekundenValid = 0;
		#ifdef MINUTEN_PHIL
		// Behandlung der Minuten_LEDs
		if (RTC_VALID) {
			if (minutenValid %5 == 0) { // alle LEDs aus
				cbi(PORTB, PB0);
				cbi(PORTB, PB1);
				cbi(PORTB, PB5);
				cbi(PORTB, PB6);
			} else if (minutenValid %5 == 1) { // LED 1 an
				sbi(PORTB, PB0);
			} else if (minutenValid %5 == 2) { // LED 2 an
				sbi(PORTB, PB1);
			} else if (minutenValid %5 == 3) { // LED 3 an
				sbi(PORTB, PB5);
			} else if (minutenValid %5 == 4) { // LED 4 an
				sbi(PORTB, PB6);
			}
		}
		#endif
	}
	
	// Syncnacht aktivieren!
	if ((stundenValid == 3) && (minutenValid == 0) && (status == RTC_VALID)) {
// 		if(status == DREIUHR_PRE_DCF_SYNC) {
// 			min_increase = minutenValid;
// 		} else {
			status = DREIUHR_PRE_DCF_SYNC;
// 		}
	}
	
	// Uhr hat es nicht geschafft zu syncen.
	if ((stundenValid == 4) && (minutenValid == 0) && (status == DREIUHR_DCF_SYNC)) {
		status = DREIUHR_POST_DCF_SYNC;
	}
	
	// Normalbetrieb...
	if (status == RTC_VALID) {
		// RTC-Auslesung und evtl. aktualisierung des Displays
		status = WRITE_DISP;
	}
}
