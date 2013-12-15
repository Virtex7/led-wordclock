const uint16_t esIst [11] PROGMEM = {1024,1024,0,1024,1024,1024,0,0,0,0,0};
const uint16_t kurz [11] PROGMEM = {0,0,0,0,0,0,0,512,512,512,512};
const uint16_t minVor [11] PROGMEM = {0,0,0,0,0,64,64,64,0,0,0};
const uint16_t minNach [11] PROGMEM = {64,64,64,64,0,0,0,0,0,0,0};
const uint16_t minFuenf [11] PROGMEM = {0,0,0,0,0,0,0,1024,1024,1024,1024};
const uint16_t minZehn [11] PROGMEM = {256,256,256,256,0,0,0,0,0,0,0};
const uint16_t minViertel [11] PROGMEM = {0,0,0,0,128,128,128,128,128,128,128};
const uint16_t minHalb [11] PROGMEM = {32,32,32,32,0,0,0,0,0,0,0};
const uint16_t minDreiviertel [11] PROGMEM = {128,128,128,128,128,128,128,128,128,128,128};

const uint16_t stdEin [11] PROGMEM = {0,0,4,4,4,0,0,0,0,0,0};
const uint16_t stdEins [11] PROGMEM = {0,0,4,4,4,4,0,0,0,0,0};
const uint16_t stdZwei [11] PROGMEM = {4,4,4,4,0,0,0,0,0,0,0};
const uint16_t stdDrei [11] PROGMEM = {0,0,0,0,0,0,0,8,8,8,8};
const uint16_t stdVier [11] PROGMEM = {0,0,0,2,2,2,2,0,0,0,0};
const uint16_t stdFuenf [11] PROGMEM = {0,0,0,0,0,0,16,16,16,16,0};
const uint16_t stdSechs [11] PROGMEM = {1,1,1,1,1,0,0,0,0,0,0};
const uint16_t stdSieben [11] PROGMEM = {0,0,0,0,0,4,4,4,4,4,4};
const uint16_t stdAcht [11] PROGMEM = {0,0,0,0,0,0,0,2,2,2,2};
const uint16_t stdNeun [11] PROGMEM = {0,0,0,8,8,8,8,0,0,0,0};
const uint16_t stdZehn [11] PROGMEM = {8,8,8,8,0,0,0,0,0,0,0};
const uint16_t stdElf [11] PROGMEM = {2,2,2,0,0,0,0,0,0,0,0};
const uint16_t stdZwoelf [11] PROGMEM = {0,0,16,16,16,16,16,0,0,0,0};

const uint16_t stdVor [11] PROGMEM = {0,0,0,0,64,64,64,0,0,0,0};
const uint16_t stdNach [11] PROGMEM = {64,64,64,64,0,0,0,0,0,0,0};
const uint16_t uhr [11] PROGMEM = {0,0,0,0,0,0,1,1,1,0,0};

uint16_t Allon [11] = {2047,2047,2047,2047,2047,2047,2047,2047,2047,2047,2047};
uint16_t funkuhr [11] = {512,512,512,512,544,544,32,544,544,544,544};

void timeToArray(void) {
	if(minutenValid%5 == 0) { // minuten muss durch 5 Teilbar sein.
		clearTemp();
		addArray(esIst); // GrundsÃ€tzlich soll "Es ist" angezeigt werden
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
				addArray(minDreiviertel);
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
