// Prototypen:


void rtcWrite(uint8_t hr, uint8_t min);
void rtcDebug(uint8_t hr_tx, uint8_t min_tx);
void rtcRead(void);
void rtcSetValid(void);
void rtcWriteRegister(uint8_t reg, uint8_t data);
void rtcHandleI2cError(void);
void rtcPanic(void) __attribute__ ((noreturn));

void rtcInit(void) {
	rtcWriteRegister(0xE, 0); // Aktiviere Oszillator und den anfangs nicht aktiven 1Hz Rechteckausgang an INT
	delayms(10);
	rtcWriteRegister(0x02, 0); // Stelle 24-Stunden-Modus ein, setze Stunden auf 0?? TODO wieso zeigt die RTC dann nicht nach jedem Atmel-Reset "0 Uhr" an?
	delayms(10);
}

void rtcReset(void) {
	for (uint8_t i=0; i<=12; i++) {
		rtcWriteRegister(i, 0);
		delayms(10);
	}
	rtcInit();
}

// Funkitonen

uint8_t rtcReadRegister(uint8_t reg) {
	for (uint8_t i=0; i<=5; i++) {
		ERR=0;
		uint8_t r=i2c_rx_DS1307(reg);
	#ifdef DEBUG_RTC
		uart_tx_str("I2C rx");
		uartTxHex8(reg);
		uart_tx_str("=");
		uartTxHex8(r);
		uart_tx_newline();
	#endif
		if (ERR) {
			rtcHandleI2cError();
		} else {
			return r;
		}
	}
	rtcPanic();
};

void rtcWriteRegister(uint8_t reg, uint8_t data) {
	for (uint8_t i=0; i<=5; i++) {
		ERR=0;
	#ifdef DEBUG_RTC
		uart_tx_str("I2C tx");
		uartTxHex8(reg);
		uartTxHex8(data);
		uart_tx_newline();
		i2c_tx(data,reg,0b11010000);
	#endif
		if (ERR) {
			rtcHandleI2cError();
		} else {
			return;
		}
	}
	rtcPanic();
}

void rtcHandleI2cError(void) {
	if (ERR==1) {
		if (rtcI2CErrors < 65535) {
			rtcI2CErrors++;
		}
		ERR=0;
		#ifdef DEBUG_RTC
		uart_tx_strln("I2C err");
		#endif
	}
	//while(1) {}
}

void rtcHandleHighlevelError(void) {
	if (rtcHighlevelErrors < 65535) {
		rtcHighlevelErrors++;
	}
}

void rtcWrite(uint8_t hr, uint8_t min) {
	uint8_t min_tx = ((min/10)<<4) | (min%10); // Erzeugung von BCD-Code-Notation h/min
	uint8_t hr_tx =  ((hr/10)<<4) | (hr%10);
	// Schleife, damit die RTC bei schlechter Verbindung bist zu 3 mal geschrieben wird.
	for (uint8_t i=0; i<3; i++) {
		rtcWriteRegister(0, 0x00); // setze Sekunden auf Eins (gegen falschgehen der RTC)
		rtcWriteRegister(1, min_tx); // setze Minuten in die RTC
		rtcWriteRegister(2, hr_tx); // ----- Stunden ----------
		
		#ifdef DEBUG_RTC
		rtcDebug(hr_tx, min_tx);
		#endif
		// Überprüfung der RTC-Zeit mit Soll Wert
		if((min_tx == rtcReadRegister(0x01)) && (hr_tx == rtcReadRegister(0x02))) {
			return;
		} else {
			// Fehler!
			#ifdef DEBUG_RTC
			uartTxStrln("RTC schreiben fehlgeschlagen");
			#endif
			rtcHandleHighlevelError();
			continue;
		}
	}
	rtcPanic();
}

void rtcDebug(uint8_t hr_tx, uint8_t min_tx) {
	uart_tx_str("Soll-Zeit: ");
	uart_tx_bin8(hr_tx);
	uart_tx_str(":");
	uart_tx_bin8(min_tx);
	uart_tx_str(":");
	uart_tx_bin8(0);
	uart_tx_newline();
	uart_tx_str("RTC-Zeit:  ");
	uart_tx_bin8(rtcReadRegister(0x02));
	uart_tx_str(":");
	uart_tx_bin8(rtcReadRegister(0x01));
	uart_tx_str(":");
	uart_tx_bin8(rtcReadRegister(0x00));
	uart_tx_newline();
	
}

void rtcRead(void) {
	uint8_t sek_tmp=123, min_tmp=124, std_tmp=125;
	for (uint8_t i=0; i<3; i++) {
		sek_tmp = rtcReadRegister(0x00);
		min_tmp = rtcReadRegister(0x01);
		std_tmp = rtcReadRegister(0x02);
		delayms(10);
		if((min_tmp == rtcReadRegister(0x01)) && (std_tmp == rtcReadRegister(0x02))) {
			// erfolgreich zweimal hintereinander eingelesen
			sekundenValid = ((sek_tmp & 0x0F) + (sek_tmp >> 4)*10);
			minutenValid  = ((min_tmp & 0x0F) + (min_tmp >> 4)*10);
			stundenValid  = ((std_tmp & 0x0F) + (std_tmp >> 4)*10);
			return;
		} else {
			rtcHandleHighlevelError();
			#ifdef DEBUG_RTC
			uartTxStrln("RTC lesen fehlgeschlagen");
			#endif
		}
	}
	rtcPanic();
	
}


// Gültigkeitsprüfung für RTC:
// Register 0x7 (Alarm1 Sekunden, von uns ungenutzt) wird mit bestimmtem Wert beschrieben
// Flag "OSF" (Bit 7 in Register 0xF) ist 0, sonst gab es vorher einen Stromausfall
uint8_t rtcIsValid(void) {
	return ((rtcReadRegister(0x7) == 0b01011101) && !(rtcReadRegister(0xF) & (1<<7)));
}

void rtcSetValid(void) {
	#ifdef DEBUG_RTC
	uartTxStrln("Validiere RTC");
	#endif
	// Registerbedeutung siehe rtcIsValid
	rtcWriteRegister(0x7,0b01011101); // fester Wert
	rtcWriteRegister(0xF, 0); // OSF flag reset, siehe rtcIsValid
	#ifdef DEBUG_RTC
	if (!rtcIsValid()) {
		rtcHandleHighlevelError();
		uartTxStrln("RTC ungültig obwohl gerade validiert!");
		rtcPanic();
	}
	#endif
}

void rtcPanic(void) {
	#ifdef DEBUG_RTC
	uartTxStrln("PANIC: RTC");
	#endif
	while (1) {
		htDisplOff();
		
		DCF_LED(1);
		delayms(100);
		DCF_LED(0);
		delayms(100);
		
		DCF_LED(1);
		delayms(100);
		DCF_LED(0);
		delayms(100);
		
		DCF_LED(1);
		delayms(100);
		DCF_LED(0);
		
		delayms(1700);
	}
}