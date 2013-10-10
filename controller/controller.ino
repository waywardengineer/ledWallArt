#include "Wire.h"
#include <avr/pgmspace.h>
#define PCA9685_LED0_ON_L 0x6
#define PCA9685_I2C_START_ADDR 0x40
#define PCA9685_CONFIG_BYTE1 0b00100001
#define PCA9685_CONFIG_BYTE2 0b00011001


#define NUM_ARMS 3
#define NUM_LEDS_PER_ARM 10
#define KEYFRAMESPACING 2000
#define NUM_PATTERN_LAYERS 1

const uint8_t ledOrdering[NUM_LEDS_PER_ARM] = {0, 4, 1, 2, 3, 5, 9, 6, 8, 7};
typedef struct {
	uint8_t strengthFactor; //relative intensity, 0-90
	uint8_t timebase; //length for cycle
	uint8_t phaseOffset;
	uint8_t positionVariance;
	//uint8_t blending; // 0=no blending over space, 1=blending over space
	uint16_t ledMask; //mask for which leds are affected by pattern
	uint8_t armMask;
	uint8_t currentTime;
	uint16_t hues[2];
	uint8_t saturations[2];
	uint8_t brightnesses[2];
} SinePatternLayer;
SinePatternLayer sinePatternLayers[NUM_PATTERN_LAYERS];
uint16_t frames[3][NUM_ARMS][NUM_LEDS_PER_ARM][3];
unsigned long keyFrameExpiration;
uint16_t rgb[3];

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
	sinePatternLayers[0].strengthFactor = 90;
	sinePatternLayers[0].timebase = 30;
	sinePatternLayers[0].phaseOffset = 0;
	sinePatternLayers[0].positionVariance = 10;
	sinePatternLayers[0].ledMask = 0b0000001111111111;
	sinePatternLayers[0].hues[0] = 290;
	sinePatternLayers[0].hues[1] = 320;
	sinePatternLayers[0].saturations[0] = 200;
	sinePatternLayers[0].saturations[1] = 200;
	sinePatternLayers[0].brightnesses[0] = 255;
	sinePatternLayers[0].brightnesses[1] = 200;
	calculateNextKeyFrame();
	calculateNextKeyFrame();
}
void loop(){
	
#ifdef TESTPATTERN
	for (i=0; i<10; i++){
		int hIndex = loopVal + i * 25;
		hIndex = hIndex > 255 ? hIndex-255 : hIndex;
		uint16_t hsv[3] = {sinCurve360[hIndex], 2000, 4095};
		writeLedColor(0, i, hsv);
	}
	loopVal += 2;
	loopVal = loopVal > 255 ? 0 : loopVal;
#else

#endif
}

void sendPwmCmd (uint8_t chipAddr, uint8_t ledAddr, uint16_t val){
	Wire.beginTransmission(chipAddr + PCA9685_I2C_START_ADDR);
	Wire.write(PCA9685_LED0_ON_L+4*ledAddr);
	Wire.write(0); //on bit 0
	Wire.write(0); //on bit 1
	Wire.write(val); //off bit 0
	Wire.write(val>>8); //off bit 1
	Wire.endTransmission();
}


void setParams(uint8_t chipAddr) {
	sendByte(chipAddr, 0, PCA9685_CONFIG_BYTE1 & 0x10); // sleep
	sendByte(chipAddr, 0xFE, 14); // set the prescaler
	sendByte(chipAddr, 0, PCA9685_CONFIG_BYTE1);
	sendByte(chipAddr, 1, PCA9685_CONFIG_BYTE2);
}
void sendByte(uint8_t chipAddr, uint8_t regAddr, uint8_t data) {
	Wire.beginTransmission(chipAddr + PCA9685_I2C_START_ADDR);
	Wire.write(regAddr);
	Wire.write(data);
	Wire.endTransmission();
}





void writeLedColor(uint8_t arm, uint8_t ledIndex, uint16_t hsv[3])  { //hsv[0] hue, 0-360; hsv[1], 0-255; hsv[2], 0-255
	uint8_t chipAddr = arm * 2;
	if (ledIndex > 4){
		chipAddr++;
		ledIndex -= 5;
	}
	chipAddr += PCA9685_I2C_START_ADDR;
	ledIndex *= 3;
	getRGB(hsv[0], hsv[1], hsv[2], rgb);
	Wire.beginTransmission(chipAddr);
	Wire.write(PCA9685_LED0_ON_L+4*ledIndex);
	Wire.write(0); //on bit 0
	Wire.write(0); //on bit 1
	Wire.write(rgb[1]); //off bit 0
	Wire.write(rgb[1]>>8); //off bit 1
	Wire.write(0); //on bit 0
	Wire.write(0); //on bit 1
	Wire.write(rgb[2]); //off bit 0
	Wire.write(rgb[2]>>8); //off bit 1
	Wire.write(0); //on bit 0
	Wire.write(0); //on bit 1
	Wire.write(rgb[0]); //off bit 0
	Wire.write(rgb[0]>>8); //off bit 1
	Wire.endTransmission();

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
void calculateNextKeyFrame(){
	shiftKeyFrames();
	for (ledIndex=0; ledIndex < NUM_LEDS_PER_ARM; ledIndex++){
		uint16_t hsv[NUM_ARMS][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
		for (patternIndex=0; patternIndex < NUM_PATTERN_LAYERS; patternIndex++){
			if (sinePatternLayers[patternIndex].strengthFactor > 0 && (sinePatternLayers[patternIndex].ledMask & (1 << ledIndex))){
				uint16_t interval = (sinePatternLayers[patternIndex].currentTime + ledIndex * sinePatternLayers[patternIndex].positionVariance + sinePatternLayers[patternIndex].phaseOffset) % 255;
				uint16_t thisHsv[3] = {
					calculateSin(sinePatternLayers[patternIndex].hues[0], sinePatternLayers[patternIndex].hues[1], interval),
					calculateSin(sinePatternLayers[patternIndex].saturations[0], sinePatternLayers[patternIndex].saturations[1], interval),
					calculateSin(sinePatternLayers[patternIndex].brightnesses[0], sinePatternLayers[patternIndex].brightnesses[1], interval)
				};
				for (uint8_t armIndex = 0; armIndex < NUM_ARMS; armIndex++){
					if (sinePatternLayers[patternIndex].armMask & (1<<armIndex)){
						for (uint8_t colorIndex = 0; colorIndex < 3; colorIndex++){
							if (patternIndex == 0){
								hsv[armIndex][colorIndex] = thisHsv[colorIndex];
							}
							else{
								uint16_t val = (hsv[armIndex][colorIndex]*90 + thisHsv[colorIndex]*sinePatternLayers[patternIndex].strengthFactor)/180;
								hsv[armIndex][colorIndex] = val;
							}
						}
					}
				}
			}
		}
		for (uint8_t armIndex = 0; armIndex < NUM_ARMS; armIndex++){
			getRGB(hsv[armIndex][0], hsv[armIndex][1], hsv[armIndex][2], rgb);
			for (uint8_t colorIndex = 0; colorIndex < 3; colorIndex++){
				frames[2][armIndex][ledIndex][colorIndex] = rgb[colorIndex];
			}
		}
	}
}

const uint16_t dim_curve[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 24, 26, 27, 28, 30, 31, 32, 34, 35, 37, 38, 40, 41, 43, 44, 46, 48, 49, 51, 53, 55, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 79, 81, 83, 85, 88, 90, 93, 95, 98, 100, 103, 106, 108, 111, 114, 117, 120, 122, 125, 129, 132, 135, 138, 141, 145, 148, 152, 155, 159, 162, 166, 170, 174, 177, 181, 185, 190, 194, 198, 202, 207, 211, 216, 220, 225, 230, 235, 240, 245, 250, 255, 261, 266, 272, 277, 283, 289, 295, 301, 307, 313, 319, 326, 333, 339, 346, 353, 360, 367, 375, 382, 390, 397, 405, 413, 421, 429, 438, 446, 455, 464, 473, 482, 491, 501, 510, 520, 530, 540, 551, 561, 572, 583, 594, 605, 617, 629, 640, 653, 665, 677, 690, 703, 716, 730, 743, 757, 772, 786, 801, 816, 831, 846, 862, 878, 894, 911, 928, 945, 962, 980, 998, 1016, 1035, 1054, 1074, 1093, 1113, 1134, 1154, 1176, 1197, 1219, 1241, 1264, 1287, 1310, 1334, 1359, 1383, 1409, 1434, 1460, 1487, 1514, 1541, 1569, 1598, 1626, 1656, 1686, 1716, 1747, 1779, 1811, 1844, 1877, 1911, 1945, 1980, 2016, 2052, 2089, 2127, 2165, 2204, 2243, 2284, 2325, 2366, 2409, 2452, 2496, 2540, 2586, 2632, 2679, 2727, 2776, 2826, 2876, 2928, 2980, 3033, 3087, 3142, 3198, 3255, 3314, 3373, 3433, 3494, 3556, 3619, 3684, 3750, 3816, 3884, 3953, 4024, 4095};
const uint8_t sinCurve255[256] = {128, 131, 134, 137, 140, 143, 146, 149, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 182, 185, 188, 191, 194, 196, 199, 201, 204, 207, 209, 211, 214, 216, 218, 220, 223, 225, 227, 229, 231, 232, 234, 236, 238, 239, 241, 242, 243, 245, 246, 247, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 245, 244, 243, 241, 240, 238, 237, 235, 233, 232, 230, 228, 226, 224, 222, 219, 217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 192, 189, 187, 184, 181, 178, 175, 172, 169, 166, 163, 160, 157, 154, 151, 148, 145, 142, 139, 135, 132, 129, 127, 124, 121, 117, 114, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 78, 75, 72, 69, 67, 64, 61, 58, 56, 53, 51, 48, 46, 43, 41, 39, 37, 34, 32, 30, 28, 26, 24, 23, 21, 19, 18, 16, 15, 13, 12, 11, 9, 8, 7, 6, 5, 5, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 17, 18, 20, 22, 24, 25, 27, 29, 31, 33, 36, 38, 40, 42, 45, 47, 49, 52, 55, 57, 60, 62, 65, 68, 71, 74, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 107, 110, 113, 116, 119, 122, 125};

uint16_t calculateSin(uint16_t bottomValue, uint16_t topValue, uint8_t interval){
	
}


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


