const uint16_t minEine [11] PROGMEM = {256,256,384,0,0,0,0,0,0,0,0};
const uint16_t minZwei [11] PROGMEM = {1024,1024,1024,1024,0,0,0,0,0,0,0};
const uint16_t minVier [11] PROGMEM = {0,0,0,0,512,512,512,512,0,0,0};
const uint16_t minFuenf [11] PROGMEM = {0,0,0,0,0,0,0,128,128,128,128};
const uint16_t minSieben [11] PROGMEM = {0,0,0,0,0,1024,1024,1024,1024,1024,1024};
const uint16_t minAcht [11] PROGMEM = {0,0,0,0,256,256,256,256,0,0,0};
const uint16_t minZehn [11] PROGMEM = {0,128,128,128,128,0,0,0,0,0,0};
const uint16_t minElf [11] PROGMEM = {0,0,0,0,0,0,0,0,256,256,256};

const uint16_t minViertel [11] PROGMEM = {0,0,0,0,512,512,512,512,512,512,512};
const uint16_t minHalb [11] PROGMEM = {64,64,64,64,0,0,0,0,0,0,0};
const uint16_t minDreiViertel [11] PROGMEM = {512,512,512,512,512,512,512,512,512,512,512};
const uint16_t minUm [11] PROGMEM = {0,0,0,0,0,128,128,0,0,0,0};

const uint16_t stdEins [11] PROGMEM = {0,16,16,16,16,0,0,0,0,0,0};
const uint16_t stdZwei [11] PROGMEM = {1,1,1,1,0,0,0,0,0,0,0};
const uint16_t stdDrei [11] PROGMEM = {0,0,0,0,0,0,0,2,2,2,2};
const uint16_t stdVier [11] PROGMEM = {2,2,2,2,0,0,0,0,0,0,0};
const uint16_t stdFuenf [11] PROGMEM = {0,0,0,0,8,8,8,8,0,0,0};
const uint16_t stdSechs [11] PROGMEM = {0,0,0,0,16,16,16,16,16,0,0};
const uint16_t stdSieben [11] PROGMEM = {0,32,32,32,32,32,32,0,0,0,0};
const uint16_t stdAcht [11] PROGMEM = {0,0,0,0,4,4,4,4,0,0,0};
const uint16_t stdNeun [11] PROGMEM = {0,0,0,0,0,0,32,32,32,32,0};
const uint16_t stdZehn [11] PROGMEM = {4,4,4,4,0,0,0,0,0,0,0};
const uint16_t stdElf [11] PROGMEM = {0,0,0,0,0,0,0,0,4,4,4};
const uint16_t stdZwoelf [11] PROGMEM = {8,8,8,8,8,0,0,0,0,0,0};

const uint16_t uhr [11] PROGMEM = {0,0,0,0,0,0,0,0,1,1,1};
const uint16_t ziffern [11] PROGMEM = {0,0,0,0,64,64,64,64,64,64,64};
const uint16_t ziffer [11] PROGMEM = {0,0,0,0,64,64,64,64,64,64,0};

uint16_t Allon [11] = {2047,2047,2047,2047,2047,2047,2047,2047,2047,2047,2047};
uint16_t funkuhr [11] = {0,0,0,0,0,0,0,0,0,0,0};

void timeToArray(void) {
	if(minutenValid%5 == 0) { // minuten muss durch 5 Teilbar sein.
		clearTemp();
		stundenValid++;
		switch (minutenValid) {
			case 5:
				addArray(minEine);
				addArray(ziffer);
				break;
			case 10:
				addArray(minZwei);
				addArray(ziffern);
				break;
			case 15:
				addArray(minViertel);
				break;
			case 20:
				addArray(minVier);
				addArray(ziffern);
				break;
			case 25:
				addArray(minFuenf);
				addArray(ziffern);
				break;
			case 30:
				addArray(minHalb);
				break;
			case 35:
				addArray(minSieben);
				addArray(ziffern);
				break;
			case 40:
				addArray(minAcht);
				addArray(ziffern);
				break;
			case 45:
				addArray(minDreiViertel);
				break;
			case 50:
				addArray(minZehn);
				addArray(ziffern);
				break;
			case 55:
				addArray(minElf);
				addArray(ziffern);
				break;
			case 0:
				addArray(minUm);
				stundenValid--;
				break;
		}
		switch (stundenValid) {
			case 1:
			case 13:
				addArray(stdEins);
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
		
		htWriteDisplay(temp);
	}
}