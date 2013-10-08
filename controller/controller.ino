#include "Wire.h"

const byte rOffset = 0;
const byte gOffset = 1;
const byte bOffset = 2;
const byte sinCurve255[256] = {128, 131, 134, 137, 140, 143, 146, 149, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 182, 185, 188, 191, 194, 196, 199, 201, 204, 207, 209, 211, 214, 216, 218, 220, 223, 225, 227, 229, 231, 232, 234, 236, 238, 239, 241, 242, 243, 245, 246, 247, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 245, 244, 243, 241, 240, 238, 237, 235, 233, 232, 230, 228, 226, 224, 222, 219, 217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 192, 189, 187, 184, 181, 178, 175, 172, 169, 166, 163, 160, 157, 154, 151, 148, 145, 142, 139, 135, 132, 129, 127, 124, 121, 117, 114, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 78, 75, 72, 69, 67, 64, 61, 58, 56, 53, 51, 48, 46, 43, 41, 39, 37, 34, 32, 30, 28, 26, 24, 23, 21, 19, 18, 16, 15, 13, 12, 11, 9, 8, 7, 6, 5, 5, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 17, 18, 20, 22, 24, 25, 27, 29, 31, 33, 36, 38, 40, 42, 45, 47, 49, 52, 55, 57, 60, 62, 65, 68, 71, 74, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 107, 110, 113, 116, 119, 122, 125};
const uint16_t sinCurve360[255] = {180, 184, 188, 193, 197, 202, 206, 210, 215, 219, 223, 228, 232, 236, 240, 245, 249, 253, 257, 261, 265, 269, 272, 276, 280, 283, 287, 291, 294, 297, 301, 304, 307, 310, 313, 316, 319, 322, 324, 327, 330, 332, 334, 336, 339, 341, 343, 344, 346, 348, 349, 351, 352, 353, 354, 355, 356, 357, 358, 358, 359, 359, 359, 359, 359, 359, 359, 359, 359, 358, 357, 357, 356, 355, 354, 353, 351, 350, 349, 347, 345, 343, 342, 340, 338, 335, 333, 331, 328, 326, 323, 320, 318, 315, 312, 309, 306, 302, 299, 296, 292, 289, 285, 282, 278, 274, 270, 267, 263, 259, 255, 251, 247, 242, 238, 234, 230, 226, 221, 217, 213, 208, 204, 199, 195, 191, 186, 182, 178, 174, 169, 165, 161, 156, 152, 147, 143, 139, 134, 130, 126, 122, 118, 113, 109, 105, 101, 97, 93, 90, 86, 82, 78, 75, 71, 68, 64, 61, 58, 54, 51, 48, 45, 42, 40, 37, 34, 32, 29, 27, 25, 22, 20, 18, 17, 15, 13, 11, 10, 9, 7, 6, 5, 4, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 14, 16, 17, 19, 21, 24, 26, 28, 30, 33, 36, 38, 41, 44, 47, 50, 53, 56, 59, 63, 66, 69, 73, 77, 80, 84, 88, 91, 95, 99, 103, 107, 111, 115, 120, 124, 128, 132, 137, 141, 145, 150, 154, 158, 163, 167, 172, 176};
byte rPin = 14;
byte gPin = 15;
byte bPin = 16;
int val = 0;
int i;
int loopVal = 0;
void setup(){
	pinMode(rPin, OUTPUT);
	pinMode(gPin, OUTPUT);  
	pinMode(bPin, OUTPUT);  
	digitalWrite(rPin, HIGH);
	Wire.begin();
	Serial.begin(9600);
	setParams(0);
	setParams(1);
	Serial.begin(9600);
}
void loop(){
	for (i=0; i<10; i++){
		int hIndex = loopVal + i * 25;
		hIndex = hIndex > 255 ? hIndex-255 : hIndex;
		int hsv[3] = {sinCurve360[hIndex], 100, 255};
		writeLedColor(0, i, hsv);
	}
	loopVal += 2;
	loopVal = loopVal > 255 ? 0 : loopVal;
	delay(50);
}

void sendPwmCmd (uint8_t chipAddr, uint8_t ledAddr, uint16_t val){
	#define LED0_ON_L 0x6
	#define LED0_ON_L 0x6
	#define I2C_START_ADDR 0x40
	Wire.beginTransmission(chipAddr + I2C_START_ADDR);
	Wire.write(LED0_ON_L+4*ledAddr);
	Wire.write(0); //on bit 0
	Wire.write(0); //on bit 1
	Wire.write(val); //off bit 0
	Wire.write(val>>8); //off bit 1
	Wire.endTransmission();
}


void setParams(uint8_t chipAddr) {
	uint8_t config = 0b00100001;
	sendByte(chipAddr, 0, config & 0x10); // sleep
	sendByte(chipAddr, 0xFE, 14); // set the prescaler
	sendByte(chipAddr, 0, config);
	sendByte(chipAddr, 1, 0b00010001);
}
void sendByte(uint8_t chipAddr, uint8_t regAddr, uint8_t data) {
	Wire.beginTransmission(chipAddr + I2C_START_ADDR);
	Wire.write(regAddr);
	Wire.write(data);
	Wire.endTransmission();
}





void writeLedColor(byte arm, byte ledIndex, int hsv[3])  { //hsv[0] hue, 0-360; hsv[1], 0-255; hsv[2], 0-255
	uint8_t rgb[3];
	uint8_t chipAddr = arm * 2;
	uint8_t regaddr;
	if (ledIndex > 4){
		chipAddr++;
		ledIndex -= 5;
	}
	ledIndex *= 3;
	getRGB(hsv[0], hsv[1], hsv[2], rgb);
	sendPwmCmd(chipAddr, ledIndex+rOffset, rgb[0]*16);
	sendPwmCmd(chipAddr, ledIndex+gOffset, rgb[1]*16);
	sendPwmCmd(chipAddr, ledIndex+bOffset, rgb[2]*16);
}

void getRGB(int hue, int sat, int val, uint8_t colors[3]) { 
const byte dim_curve[] = {
		0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
		3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
		4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
		6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
		8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
		11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
		15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
		20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
		27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
		36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
		48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
		63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
		83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
		110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
		146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
		193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};


	/* From http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
		convert hue, saturation and brightness ( HSB/HSV ) to RGB
		 The dim_curve is used only on brightness/value and on saturation (inverted).
		 This looks the most natural.      
	*/
	
	val = dim_curve[val];
	sat = 255-dim_curve[255-sat];
	
	int r;
	int g;
	int b;
	int base;
	
	if (sat == 0) { // Acromatic color (gray). Hue doesn't mind.
		colors[0]=val;
		colors[1]=val;
		colors[2]=val;  
	} else  { 
		
		base = ((255 - sat) * val)>>8;
	
		switch(hue/60) {
	case 0:
		r = val;
		g = (((val-base)*hue)/60)+base;
		b = base;
	break;
	
	case 1:
		r = (((val-base)*(60-(hue%60)))/60)+base;
		g = val;
		b = base;
	break;
	
	case 2:
		r = base;
		g = val;
		b = (((val-base)*(hue%60))/60)+base;
	break;
	
	case 3:
		r = base;
		g = (((val-base)*(60-(hue%60)))/60)+base;
		b = val;
	break;
	
	case 4:
		r = (((val-base)*(hue%60))/60)+base;
		g = base;
		b = val;
	break;
	
	case 5:
		r = val;
		g = base;
		b = (((val-base)*(60-(hue%60)))/60)+base;
	break;
		}
			
		colors[0]=r;
		colors[1]=g;
		colors[2]=b; 
	}   

}