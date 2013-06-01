/*
 *    Filename: dcf.h
 *     Version: 0.3.0
 * Description: Funktionen für ein DCF-Modul
 *     License: GPLv3 or later
 *     Depends:     global.h, io.h,
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

// Benötigte Definitionen in der Quelldatei:

// #define DCF_POWER_DDR
// #define DCF_POWER_PORT
// #define DCF_POWER_PIN
// #define DCF_PON_DDR
// #define DCF_PON_PORT
// #define DCF_PON_PIN


// Diese INIT-Funktion nimmt einen Takt von 10MHz an. ACHTUNG!
void dcfInit(void) {
	DCF_PON_DDR |= (1<<DCF_PON_PIN);
	TCCR1B = (1<<CS12) | (1<<CS10); // Prescaler = 1024
	GICR = (1<<INT0);   // INT0 ist ab hier ein Interrupt-Pin
	MCUCR = (1<<ISC00); // INT0 Logic Change Interrupt
}

inline void dcfOn(void) { //Aktivieren des DCF77 Moduls (PON)
cbi(DCF_POWER_PORT, DCF_POWER_PIN); // Aktiviere DCF-Strom
delayms(2000);
sbi(DCF_PON_PORT, DCF_PON_PIN);
}

inline void dcfOff(void) { //Deaktivieren des DCF77 Moduls (PON)
#ifdef HW_0_4
cbi(PORTC, PC1);
cbi(PORTD, PD4);
#else
cbi(PORTD, PD3);
#endif
}