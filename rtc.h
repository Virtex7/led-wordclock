// Prototypen:

void rtcWrite(uint8_t hr, uint8_t min);
void rtcDebug(uint8_t hr_tx, uint8_t min_tx);
void rtcRead(void);
void rtcSetValid(void);


// Funkitonen

void rtcWrite(uint8_t hr, uint8_t min) {
	uint8_t i=3;
	uint8_t min_tx = ((min/10)<<4) | (min%10); // Erzeugung von BCD-Code-Notation h/min
	uint8_t hr_tx =  ((hr/10)<<4) | (hr%10);
	// while Schleife, damit die RTC bei schlechter Verbindung bist zu 3 mal geschrieben wird.
	while(i) {
		
		i2c_tx(0x00,0x00,0b11010000); // setze Sekunden auf Eins (gegen falschgehen der RTC)
		i2c_tx(min_tx,0x01,0b11010000); // setze Minuten in die RTC
		i2c_tx(hr_tx,0x02, 0b11010000); // ----- Stunden ----------
		
		#ifdef DEBUG_RTC
		rtcDebug(hr_tx, min_tx);
		#endif
		// Überprüfung der RTC-Zeit mit Soll Wert
		if((min_tx == i2c_rx_DS1307(0x01)) && (hr_tx == i2c_rx_DS1307(0x02))) {
			i=0;
		} else {
			i--;
			PORTD |= (1<<PD5);
		}
	}
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
	uart_tx_bin8(i2c_rx_DS1307(0x02));
	uart_tx_str(":");
	uart_tx_bin8(i2c_rx_DS1307(0x01));
	uart_tx_str(":");
	uart_tx_bin8(i2c_rx_DS1307(0x00));
	uart_tx_newline();
	
}

void rtcRead(void) {
	uint8_t sek_tmp=0, min_tmp=0, std_tmp=0, i=5;
	
	while(i) {
		sek_tmp = i2c_rx_DS1307(0x00);
		min_tmp = i2c_rx_DS1307(0x01);
		std_tmp = i2c_rx_DS1307(0x02);
		delayms(10);
		if((min_tmp == i2c_rx_DS1307(0x01)) && (std_tmp == i2c_rx_DS1307(0x02))) {
			i = 0;
		} else {
			i--;
			// TODO: Fehlerauswertung schreiben für den Fall:
			RTC_read_error++;
		}
	}
	sekundenValid = ((sek_tmp & 0x0F) + (sek_tmp >> 4)*10);
	minutenValid  = ((min_tmp & 0x0F) + (min_tmp >> 4)*10);
	stundenValid  = ((std_tmp & 0x0F) + (std_tmp >> 4)*10);
}

void rtcSetValid(void) {
	#ifdef DEBUG_RTC
	uartTxStrln("Validiere RTC");
	#endif
	i2c_tx(0b01011101,0x07, 0b11010000);
}
