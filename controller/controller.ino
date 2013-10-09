#include "Wire.h"
#include <avr/pgmspace.h>
const byte rOffset = 2;
const byte gOffset = 0;
const byte bOffset = 1;
const byte sinCurve255[256] = {128, 131, 134, 137, 140, 143, 146, 149, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 182, 185, 188, 191, 194, 196, 199, 201, 204, 207, 209, 211, 214, 216, 218, 220, 223, 225, 227, 229, 231, 232, 234, 236, 238, 239, 241, 242, 243, 245, 246, 247, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 245, 244, 243, 241, 240, 238, 237, 235, 233, 232, 230, 228, 226, 224, 222, 219, 217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 192, 189, 187, 184, 181, 178, 175, 172, 169, 166, 163, 160, 157, 154, 151, 148, 145, 142, 139, 135, 132, 129, 127, 124, 121, 117, 114, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 78, 75, 72, 69, 67, 64, 61, 58, 56, 53, 51, 48, 46, 43, 41, 39, 37, 34, 32, 30, 28, 26, 24, 23, 21, 19, 18, 16, 15, 13, 12, 11, 9, 8, 7, 6, 5, 5, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 17, 18, 20, 22, 24, 25, 27, 29, 31, 33, 36, 38, 40, 42, 45, 47, 49, 52, 55, 57, 60, 62, 65, 68, 71, 74, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 107, 110, 113, 116, 119, 122, 125};
const uint16_t sinCurve360[256] = {180, 184, 188, 193, 197, 202, 206, 210, 215, 219, 223, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 283, 287, 290, 294, 297, 300, 304, 307, 310, 313, 316, 319, 321, 324, 327, 329, 332, 334, 336, 338, 340, 342, 344, 346, 347, 349, 350, 352, 353, 354, 355, 356, 357, 358, 358, 359, 359, 359, 359, 360, 359, 359, 359, 359, 358, 358, 357, 356, 355, 354, 353, 352, 350, 349, 347, 346, 344, 342, 340, 338, 336, 334, 332, 329, 327, 324, 321, 319, 316, 313, 310, 307, 304, 300, 297, 294, 290, 287, 283, 280, 276, 272, 268, 264, 260, 256, 252, 248, 244, 240, 236, 232, 228, 223, 219, 215, 210, 206, 202, 197, 193, 188, 184, 180, 176, 172, 167, 163, 158, 154, 150, 145, 141, 137, 132, 128, 124, 120, 116, 112, 108, 104, 100, 96, 92, 88, 84, 80, 77, 73, 70, 66, 63, 60, 56, 53, 50, 47, 44, 41, 39, 36, 33, 31, 28, 26, 24, 22, 20, 18, 16, 14, 13, 11, 10, 8, 7, 6, 5, 4, 3, 2, 2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 10, 11, 13, 14, 16, 18, 20, 22, 24, 26, 28, 31, 33, 36, 39, 41, 44, 47, 50, 53, 56, 60, 63, 66, 70, 73, 77, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124, 128, 132, 137, 141, 145, 150, 154, 158, 163, 167, 172, 176};
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
	setParams(0);
	setParams(1);
	Serial.begin(9600);
}
void loop(){
	for (i=0; i<10; i++){
		int hIndex = loopVal + i * 25;
		hIndex = hIndex > 255 ? hIndex-255 : hIndex;
		uint16_t hsv[3] = {sinCurve360[hIndex], 2000, 4095};
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
	sendByte(chipAddr, 1, 0b00011001);
}
void sendByte(uint8_t chipAddr, uint8_t regAddr, uint8_t data) {
	Wire.beginTransmission(chipAddr + I2C_START_ADDR);
	Wire.write(regAddr);
	Wire.write(data);
	Wire.endTransmission();
}





void writeLedColor(uint8_t arm, uint8_t ledIndex, uint16_t hsv[3])  { //hsv[0] hue, 0-360; hsv[1], 0-255; hsv[2], 0-255
	uint16_t rgb[3];
	uint16_t chipAddr = arm * 2;
	uint16_t regaddr;
	if (ledIndex > 4){
		chipAddr++;
		ledIndex -= 5;
	}
	ledIndex *= 3;
	getRGB(hsv[0], hsv[1], hsv[2], rgb);
	sendPwmCmd(chipAddr, ledIndex+rOffset, rgb[0]);
	sendPwmCmd(chipAddr, ledIndex+gOffset, rgb[1]);
	sendPwmCmd(chipAddr, ledIndex+bOffset, rgb[2]);
	/*if (ledIndex  == 0 && chipAddr == 1){
		Serial.println(hsv[0]);
		Serial.println(hsv[1]);
		Serial.println(hsv[2]);
		Serial.println(" ");
		Serial.println(rgb[0]);
		Serial.println(rgb[1]);
		Serial.println(rgb[2]);
		Serial.println("======");
	}*/
}
const uint16_t dim_curve[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 24, 26, 27, 28, 30, 31, 32, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48, 49, 51, 53, 55, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 79, 81, 83, 85, 88, 90, 93, 95, 98, 100, 103, 106, 108, 111, 114, 117, 120, 122, 125, 129, 132, 135, 138, 141, 145, 148, 152, 155, 159, 162, 166, 170, 174, 177, 181, 185, 190, 194, 198, 202, 207, 211, 216, 220, 225, 230, 235, 240, 245, 250, 255, 261, 266, 272, 277, 283, 289, 295, 301, 307, 313, 319, 326, 333, 339, 346, 353, 360, 367, 375, 382, 390, 397, 405, 413, 421, 429, 438, 446, 455, 464, 473, 482, 491, 501, 510, 520, 530, 540, 551, 561, 572, 583, 594, 605, 617, 629, 640, 653, 665, 677, 690, 703, 716, 730, 743, 757, 772, 786, 801, 816, 831, 846, 862, 878, 894, 911, 928, 945, 962, 980, 998, 1016, 1035, 1054, 1074, 1093, 1113, 1134, 1154, 1176, 1197, 1219, 1241, 1264, 1287, 1310, 1334, 1359, 1383, 1409, 1434, 1460, 1487, 1514, 1541, 1569, 1598, 1626, 1656, 1686, 1716, 1747, 1779, 1811, 1844, 1877, 1911, 1945, 1980, 2016, 2052, 2089, 2127, 2165, 2204, 2243, 2284, 2325, 2366, 2409, 2452, 2496, 2540, 2586, 2632, 2679, 2727, 2776, 2826, 2876, 2928, 2980, 3033, 3087, 3142, 3198, 3255, 3314, 3373, 3433, 3494, 3556, 3619, 3684, 3750, 3816, 3884, 3953, 4024, 4095};

void getRGB(uint16_t hue, uint16_t sat, uint16_t val, uint16_t colors[3]) { 



	/* From http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
		convert hue, saturation and brightness ( HSB/HSV ) to RGB
		 The dim_curve is used only on brightness/value and on saturation (inverted).
		 This looks the most natural.      
	*/
	//	Serial.println(val);
	val = interpolateDimTable(val);
	sat = 4095 - interpolateDimTable(4095-sat);
	
	unsigned long r = 0;
	unsigned long g = 0;
	unsigned long b = 0;
	unsigned long base;
	if (sat == 0) { // Acromatic color (gray). Hue doesn't mind.
		colors[0]=val;
		colors[1]=val;
		colors[2]=val;  
	} 
	else  { 
		base = (63 - sat/64) * (val/64);
		/*Serial.println(sat);
		Serial.println(val);
		Serial.println(base);*/
		if (hue > 359){
			hue = hue % 360;
		}
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
		/*Serial.println(hue/60);
		Serial.println(r);
		Serial.println(g);
		Serial.println(b);
		Serial.println("===");*/
	}   

}
uint16_t interpolateDimTable(uint16_t value){
	uint8_t index = value >> 4;
	uint16_t addOn = 0;
	if (index < 255){
		uint16_t interval = dim_curve[index + 1] - dim_curve[index];
		addOn = (value % 16) * interval / 16;
	}
	else {
		addOn = 0;
	}
	uint16_t result = dim_curve[index] + addOn;
	return result;
}


