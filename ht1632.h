/*
*    Filename: ht1632.c
*     Version: 0.0.4
* Description: Ansteuerung für eine Wordclock(TM)
*     License: GPLv3 or later
*     Depends: global.h, io.h
*
*      Author: Copyright (C) Philipp Hörauf, Alexander Schuhmann
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
/*
Folgende Definitionen werden benötigt:

//Pindefinitionen für den HT1632 Chip
#define CS(x) out(PORTC,PC2,0,x)	// idle ON
#define RD(x) out(PORTC,PC3,0,x)	// idle ON
#define WR(x) out(PORTC,PC5,0,x)	// idle ON
#define DATA(x) out(PORTC,PC4,0,x)


*/
void htWriteClk(void) {
	WR(0);
	delayus(2);
	WR(1);
	delayus(2);
}
void htReadClk(void) {
	RD(0);
	delayus(4);
	RD(1);
	delayus(4);
}
void htAdress(uint8_t adress) {
	for(uint8_t i=0; i<7; i++) {
		if(adress & (1<<(6-i))) {
			DATA(1);
		} else {
			DATA(0);
		}
		htWriteClk();
	}
}
void htCommand(uint8_t befehl) {
	CS(0);
	DATA(1);
	htWriteClk();
	DATA(0);
	htWriteClk();
	htWriteClk();
	// enter Command mode
	for(uint8_t i=0; i<8; i++) {
		if(befehl & (1<<(7-i))) {
			DATA(1);
		} else {
			DATA(0);
		}
		htWriteClk();
	}
	DATA(0);
	htWriteClk();
	CS(1);
}
inline void htDisplOn(void) {
	htCommand(0b00000001);	// SYS EN
	htCommand(0b00000011);	// LED on
}

inline void htDisplOff(void) {
	htCommand(0b00000010);	// LED off
	htCommand(0b00000000);	// SYS off


}
void htInit(void) {
	delayms(500);
	htCommand(0);		// SYS DIS
	htCommand(0b00100100);	// COM Option: pMOS ROW-Treiber und 16-COM-Option
	htCommand(0b00011000);	// RC Master Mode
	htCommand(0b00000001);	// SYS EN
	htCommand(0b00000011);	// LED on
}
void htWriteDisplay(uint16_t *Array) {
	#ifdef WORDCLOCK_MIRROR_ZEILEN
	uint16_t temparray[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	for (uint8_t i=0; i<11; i++) { // Spalten (Elemente im Array)
		for (uint8_t k=0; k<11; k++) { // Bits in Array-Element
			if (Array[i] & (1<<k)) { 
				temparray[i] |= (1<<(10-k));
			}
		}
	}
	Array = temparray;
	#endif
	CS(0);
	DATA(1);
	htWriteClk();
	DATA(0);
	htWriteClk();
	DATA(1);
	htWriteClk();
	// enter Write mode
	htAdress(0);
	// Adresse gesendet
	#ifdef WORDCLOCK_MIRROR
	for(uint8_t i=0; i<11; i++) { // sendet einzelne Zeilen des großen Arrays
		for(uint8_t k=0; k<16; k++) {
			if(Array[10-i] & (1<<k)) {
				DATA(1);
			} else {
				DATA(0);
			}
			htWriteClk();
		}
	}
	
	#endif
	#ifndef WORDCLOCK_MIRROR
	for(uint8_t i=0; i<11; i++) { // sendet einzelne Zeilen des großen Arrays
		for(uint8_t k=0; k<16; k++) {
			if(Array[i] & (1<<k)) {
				DATA(1);
			} else {
				DATA(0);
			}
			htWriteClk();
		}
	}
	#endif
	CS(1);
}
