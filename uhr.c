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
 * Hinweis zur aktuellen Codeversion > 0.4.0
 *
 * Dieser Quellcode macht sich viele Funktionalität der RTC zu Nutze. Der
 * Atmel wird hierbei hauptsächlich schlafen, und nur jede Minute kurz geweckt.
 */

// Einstellungen und globale Variablen sowie weitere Includes
#include "./defines.h"

int main(void){
	uint8_t dcfLastSyncSuccessful=2; // letzter DCF-Sync erfolgreich? 0 = Nein, 1 = ja, 2 = Stromausfall
// 	if (1) {
// 		
// 		init();
// 		ERR=0;
// 		PORTC|=1;
// 		delayms(100);
// 		rtcReadRegister(0x02);
// 		delayms(1);
// 		rtcReadRegister(0x03);
// 		delayms(1);
// 		PORTC=0;
// 	}
// 	
// 	while(1) {}
#ifdef DEBUG_STATUS
	uint8_t oldStatus=123;
#endif
	
// 	init();
// 	while(1)  {
// 	PORTD ^= (1<<PD7);
// 	delayms(255);
// 	}
	while(1) {
#ifdef DEBUG_STATUS
		if (status != oldStatus) {
			uart_tx_str("status ");
			uartTxDec2(status);
			uart_tx_newline();
			oldStatus=status;
		}
#endif
		
		if (status == INIT) {
			init(); //Je nach Hardware Rev. unterschiedliche Init --> siehe HW_x_x.h
			uartTxStr("Firmware kompiliert am ");
			uartTxStrln(__TIMESTAMP__);
			sei(); // Ab hier sind Interrupts aktiv
			//Nächster Ablauf 
			status = RTC_CHECK_VALID;
			
			#ifdef RTC_SET_CLOCK
			uartTxStrln("setze RTC auf 2:59 Uhr");
			rtcWrite(RTC_SET_STD,RTC_SET_MIN);
			#endif
			
			#ifdef RTC_RESET
			rtcReset();
			#endif
			
			#ifdef RTC_NO_DCF
			rtcWrite(3,59);
			uartTxStrln("setze RTC auf 3:59 Uhr");
			status=POST_DCF_SYNC;
			#endif
		} else if (status == RTC_CHECK_VALID) {
			if (rtcIsValid()) {
				rtcRead();
				status = PRE_RTC_VALID;
				#ifdef DEBUG_RTC
				uart_tx_strln("RTC hat valide Daten - überspringe DCF-Empfang.");
				#endif
			} 
			else {
				uart_tx_strln("RTC invalid, reset.");
				rtcReset(); // RTC zurücksetzen, damit alle Register auf Sollwerten stehen. Nach Stromausfall ist der Inhalt der Register undefiniert!
				status = PRE_DCF_SYNC;
			}
		} else if (status == PRE_RTC_VALID) {
			//INT1 wird initialisiert
			GICR |= (1<<INT1);   // INT1 ist ab hier ein Interrupt-Pin
			MCUCR |= (1<<ISC11) | (1<<ISC10); // INT1 Logic Change Interrupt
			
			// einmal ausführen, damit die Zeit möglichst schnell auf dem Display steht.
			minutenValid-=(minutenValid%5); // setze auf 5-Minuten-Step, damit Uhrzeit angezeigt wird.
			timeToArray(); // wird nur ausgeführt, wenn die Uhr einen sicheren, kalibrierten Zeitgeber hat. (RTC fixed)
			htDisplOn(); // Aktiviere Display
			rtcRead(); // korrigiere falsch gesetzten minutenValid-Wert durch RTC-Auslesung
			setMinutenLeds(); // Setze Minuten-LEDs korrekt
			
			
			
			#ifdef DEBUG_RTC
			uartTxStrln("Uhr läuft, aktiviere Display!");
			#endif
			status = RTC_VALID;
		} else if (status == PRE_DCF_SYNC) {
			#ifdef DEBUG_RTC
			uart_tx_strln("Aktiviere RTC und DCF-Uhr");
			#endif
			rtcInit();
			// INT1 für RTC wird bereits bei PRE_RTC_VALID initialisiert
			dcfInit(); // richtet INT0 ein
			
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
		} else if (status == DCF_SYNC) {
			POWER_LED(1);
			delayms(300);
			POWER_LED(0);
			// Solange kein valides DCF77 Signal blinkt PowerLED (Extern per Draht angeschlossen)
		} else if (status == POST_DCF_SYNC) {
			cbi(GICR, INT0); // INT0 Interrupt deaktivert
			dcfOff(); // schalte DCF-Modul aus
			dcfLastSyncSuccessful=1;
			rtcSetValid(); // Schreibe Validierungswert in die RTC
			status = PRE_RTC_VALID;
		} else if (status == RTC_VALID) {
			// Achtung Race-Condition: Bei diesem Status kann der ISR den Inhalt von status verändern!
			#ifdef RTC_NO_DCF
			static uint8_t einmal = 0;
			if (einmal == 0) {
				uartTxStrln("setze Zeit auf 3:59 Uhr");
				status = DREIUHR_PRE_DCF_SYNC;
				einmal = 1;
			}
			#endif
		} else if (status == WRITE_DISP) {
			// Teil 1: alte Buchstaben "fallen" herunter...
			
			#ifdef DISPLAY_DIMMEN // Hier wird der Code für das Dimmen der LEDs definiert
			if (((stundenValid > DIMMEN_START) || (stundenValid < DIMMEN_END)) && (set_dimmen == 0))	{
				htCommand(0b10100101);
				set_dimmen = 1;
			}
			if ((stundenValid < DIMMEN_START) && (DIMMEN_END > 7) && (set_dimmen == 1)){
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
				uart_tx_strln("Software-Datum:");
				uart_tx_strln(__TIMESTAMP__);
				uart_tx_str("Anzahl erfolgreicher Syncs: ");
				uart_tx_dec(dcfSyncSuccess);
				uart_tx_newline();
				uart_tx_str("Anzahl nicht erfolgreicher Syncs: ");
				uart_tx_dec(dcfSyncErrors);
				uart_tx_newline();
				uart_tx_str("Anzahl I2C Fehler RTC: ");
				uart_tx_dec(rtcI2CErrors);
				uart_tx_newline();
				uart_tx_str("Anzahl Highlevel Fehler RTC: ");
				uart_tx_dec(rtcHighlevelErrors);
				uart_tx_newline();
				uart_tx_str("Zeit des letzten Syncs: ");
				uart_tx_dec(dcfLastSyncHour);
				uart_tx_str(":");
				uart_tx_dec(dcfLastSyncMinute);
				uart_tx_newline();
				// uartTxStrln("letzte Fehler bei Synchro, usw.");
			}
			#endif
			
			// Teil 2: neue Zeit wird aus der RTC gelesen
			if (sekundenValid <=2) {
				rtcRead();
				timeToArray(); // wird nur ausgeführt, wenn die Uhr einen sicheren, kalibrierten Zeitgeber hat. (RTC fixed)
			}
			
			// Teil 3: Minuten-LEDs werden gesetzt, sofern vorhanden!
			setMinutenLeds();
			
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
			
			// wenn letzter DCF-Empfang erfolglos war, blinke alle 2s kurz
			DCF_LED(0);
			if ((dcfLastSyncSuccessful != 1) && (sekundenValid % 2 == 0)) {
				DCF_LED(1);
				delayms(100);
				DCF_LED(0);
				if (dcfLastSyncSuccessful==2) {
					delayms(100);
					DCF_LED(1);
					delayms(100);
					DCF_LED(0);
				}
			}
			
			// lege Atmel bis zum nächsten Interrupt schlafen
			#ifdef DEBUG_STATUS
			uartTxStrln("sleeping");
			#endif
			set_sleep_mode(SLEEP_MODE_IDLE);
			sleep_enable();
			sleep_cpu();
			sleep_disable();
		} else if (status == DREIUHR_PRE_DCF_SYNC) {
			dcfOn();
			htDisplOff();
			delayms(100);
			sbi(GICR, INT0); // INT0 Interrupt aktivert (DCF Empfang)
			status = DREIUHR_DCF_SYNC;
		} else if (status == DREIUHR_DCF_SYNC) {
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
		} else if (status == DREIUHR_POST_DCF_SYNC) {
			// Wenn er hierhin springt hat er in der Nacht kein Zeitsignal erhalten 
			cbi(GICR, INT0); // INT0 Interrupt deaktivert
			dcfOff(); // deaktivere DCF-Modul
			dcfLastSyncSuccessful=0;
			status = PRE_RTC_VALID;
		}
	}
	return 0;
}



ISR (INT0_vect, ISR_BLOCK) { // Pinchange-Interrupt an INT0 (DCF-Signal IN)
	// Dieser Interrupt verarbeitet das DCF-Signal, also High- und Lowphasen!!
	static uint32_t datenwert=0;
	static uint8_t i=0, timeValid=0;
	// static uint8_t sync=0;
	static uint8_t minutenDcfAktuell=0, stundenDcfAktuell=0;
	static uint8_t minutenDcfLast=0, stundenDcfLast=0;
	static uint8_t stundenParity=0, minutenParity=0;
	uint16_t  timerwert_alt = TCNT1;
	TCNT1 = 0; // leere Timer-Speicherwert
	delayms(1);
	
	uint8_t inValue=DCF_IN();
	DCF_LED(inValue); //LED an PD7 zeigt das DCF77 Signal an
	if (inValue) { // gerade ist HIGH
		TCCR1B = (1<<CS11) | (1<<CS10); // Prescaler = 64
		TCNT1 = 156;
		
		if (timerwert_alt > 14000) {
// 			sync = 1;
			uint8_t sekundenticks=i;
			i=0;
			datenwert = 0;
			timeValid = 0;
			#ifdef DEBUG_ERFASSUNG
			uart_tx_newline();
			#endif
			#ifdef DEBUG_ZEIT
			uart_tx_str("aktuelle DCF-Zeit: ");
			uart_tx_dec(stundenDcfAktuell);
			uart_tx(':');
			uart_tx_dec(minutenDcfAktuell);
			uart_tx_newline();
			uart_tx_str("Anzahl Bits: ");
			uart_tx_dec(sekundenticks);
			uart_tx_newline();
			#endif
			
			if (sekundenticks != 59) {
				timeValid=0;
				#ifdef DEBUG_ZEIT
				uart_tx_strln("zuwenig/zuviel DCF-Bits empfangen");
				#endif
			} else if ((minutenParity != 0) || (stundenParity != 0)) {
				timeValid = 0;
			} else if ((minutenDcfAktuell > 59) || (stundenDcfAktuell > 23)) { // offensichtlich Schmarrn.
				timeValid = 0;
				#ifdef DEBUG_ZEIT
				uart_tx_strln("Zeit über 23 Uhr und/oder 59 Minuten. WTF??");
				#endif
			} else if ((minutenDcfLast+1 == minutenDcfAktuell) && (stundenDcfLast == stundenDcfAktuell)) {
				// Vergleich und Setzen der Valid-Uhrzeitvariablen
				timeValid = 1;
			} else if ((minutenDcfLast == 59) && (minutenDcfAktuell == 0) && (stundenDcfLast +1 == stundenDcfAktuell)) {
				timeValid = 1;
			} else if ((minutenDcfLast == 59) && (minutenDcfAktuell == 0) && ((stundenDcfLast == 23) && (stundenDcfAktuell == 0))) {
				timeValid = 1;
			} else { // ACHTUNG FEHLER!! (oder erster Empfang nach Inbetriebnahme)
				timeValid=0;
				#ifdef DEBUG_ZEIT
				uart_tx_strln("Zeit eigentlich gültig, aber passt nicht zur vorher empfangenen");
				#endif
			}
			#ifdef DEBUG_ZEIT
			uart_tx_str("timeValid=");
			uart_tx_dec(timeValid);
			uart_tx_newline();
			#endif


			
			if (timeValid) { // Zeit ist valide: RTC wird gesetzt
				minutenValid = minutenDcfAktuell;
				stundenValid = stundenDcfAktuell;
				rtcWrite(stundenValid,minutenValid);
				status = POST_DCF_SYNC;
				dcfLastSyncHour = stundenValid;
				dcfLastSyncMinute = minutenValid;
				dcfSyncSuccess++;
			}
			minutenDcfLast = minutenDcfAktuell;
			stundenDcfLast = stundenDcfAktuell;
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
		if (1) {
			if ((NULL_LOW < timerwert_alt) && (timerwert_alt < NULL_HIGH)) { // und wir haben eine NULL
				i++;
				; // nullen stehen schon da.
				#ifdef DEBUG_ERFASSUNG
				uart_tx('0');
				//cbi(PORTB,PB1);
				#endif
			} else if((EINS_LOW < timerwert_alt) && (timerwert_alt < EINS_HIGH)) { // und wir haben eine EINS
				#ifdef DEBUG_ERFASSUNG
				uart_tx('1');
				//sbi(PORTB,PB1);
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
				 * 		Die Bit-Nummerierung ist hier teilweise andersrum.
				 */
				
				// Minuten Einerstelle
				minutenDcfAktuell =  (1 & (datenwert>>25)) + 2*(1 & (datenwert>>24)) + 4*(1 &(datenwert>>23)) + 8*(1 & (datenwert>>22));
				// Minuten Zehnerstelle
				minutenDcfAktuell += 10*((1 & datenwert>>21)) + 20*((1 & datenwert>>20)) + 40*((1 & datenwert>>19));
				
				minutenParity=0;
				for (uint8_t j=18; j<=25; j++) {
					if (datenwert & (1<<i)) {
						minutenParity ^= 1;
					}
				}
				if (minutenParity != 0) {
#ifdef DEBUG_ZEIT
					uart_tx_strln("DCF Minutenparität falsch");
#endif
				}
				
				
				// Stunden Einerstelle
				stundenDcfAktuell =  (1 & (datenwert>>17)) + 2*(1 & (datenwert>>16)) + 4*(1 &(datenwert>>15)) + 8*(1 & (datenwert>>14));
				// Stunden Zehnerstelle
				stundenDcfAktuell += 10*((1 & datenwert>>13)) + 20*((1 & datenwert>>12));
				
				stundenParity=0;
				for (uint8_t j=11; j<=14; j++) {
					if (datenwert & (1<<i)) {
						stundenParity ^= 1;
					}
				}
				if (stundenParity != 0) {
#ifdef DEBUG_ZEIT
					uart_tx_strln("DCF Stundenparität falsch");
#endif
				}
			}
		}
// 		if(i == 58) {
// 			i=0; // eine Minute ist vergangen. SYNC muss neu aufgezogen werden. (und die Minutenlücke kommt auch)
// 		}
	}
}

// Die RTC erzeugt jede Sekunde eine Aktualisierung der sekundenValid-Variable
// Muss alle Sekunden und nicht nur jede Minute sein, damit die Scroll-Funktion geht!
ISR (INT1_vect, ISR_BLOCK) {
	sekundenValid++;
	
	if (sekundenValid == 60) {
		sekundenValid = 0;
	}
	
	// Syncnacht aktivieren!
	if ((stundenValid == 03) && (minutenValid == 00) && (status == RTC_VALID)) {
		// 		if(status == DREIUHR_PRE_DCF_SYNC) {
		// 			min_increase = minutenValid;
		// 		} else {
		status = DREIUHR_PRE_DCF_SYNC;
		// 		}
	}
	
	// Uhr hat es nicht geschafft zu syncen.
	if ((stundenValid == 4) && (minutenValid == 0) && (status == DREIUHR_DCF_SYNC)) {
		dcfSyncErrors++;
		status = DREIUHR_POST_DCF_SYNC;
	}
	
	// Normalbetrieb...
	if (status == RTC_VALID) {
		// RTC-Auslesung und evtl. aktualisierung des Displays
		status = WRITE_DISP;
	}
}
