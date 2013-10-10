#include "Wire.h"
#include <avr/pgmspace.h>
#define PCA9685_LED0_ON_L 0x6
#define PCA9685_I2C_START_ADDR 0x40
#define PCA9685_CONFIG_BYTE1 0b00100001
#define PCA9685_CONFIG_BYTE2 0b00011001


#define NUM_ARMS 1
#define NUM_LEDS_PER_ARM 10
#define KEYFRAMESPACING 1000
#define NUM_PATTERN_LAYERS 2
const uint8_t dimCurve[256] PROGMEM = {
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

const uint8_t sinCurve256[256] PROGMEM = {128, 131, 134, 137, 140, 143, 146, 149, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 182, 185, 188, 191, 194, 196, 199, 201, 204, 207, 209, 211, 214, 216, 218, 220, 223, 225, 227, 229, 231, 232, 234, 236, 238, 239, 241, 242, 243, 245, 246, 247, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 245, 244, 243, 241, 240, 238, 237, 235, 233, 232, 230, 228, 226, 224, 222, 219, 217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 192, 189, 187, 184, 181, 178, 175, 172, 169, 166, 163, 160, 157, 154, 151, 148, 145, 142, 139, 135, 132, 129, 127, 124, 121, 117, 114, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 78, 75, 72, 69, 67, 64, 61, 58, 56, 53, 51, 48, 46, 43, 41, 39, 37, 34, 32, 30, 28, 26, 24, 23, 21, 19, 18, 16, 15, 13, 12, 11, 9, 8, 7, 6, 5, 5, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 17, 18, 20, 22, 24, 25, 27, 29, 31, 33, 36, 38, 40, 42, 45, 47, 49, 52, 55, 57, 60, 62, 65, 68, 71, 74, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 107, 110, 113, 116, 119, 122, 125};

const uint8_t ledOrdering[NUM_LEDS_PER_ARM] = {0, 4, 1, 2, 3, 5, 9, 6, 8, 7};
typedef struct {
	uint8_t strengthFactor; //relative intensity, 0-90
	uint8_t timeStep; //how many steps of a cycle length of 255 to take per frame
	uint8_t phaseOffset;
	uint8_t positionVariance;
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




void writeRGBColor(uint8_t arm, uint8_t ledIndex, uint16_t rgb[3])  { //hsv[0] hue, 0-360; hsv[1], 0-255; hsv[2], 0-255
	uint8_t chipAddr = arm * 2;
	if (ledIndex > 4){
		chipAddr++;
		ledIndex -= 5;
	}
	chipAddr += PCA9685_I2C_START_ADDR;
	ledIndex *= 3;
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
	for (uint8_t ledIndex=0; ledIndex < NUM_LEDS_PER_ARM; ledIndex++){
		uint16_t hsv[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
		for (uint8_t patternIndex=0; patternIndex < NUM_PATTERN_LAYERS; patternIndex++){
			if (sinePatternLayers[patternIndex].strengthFactor > 0 && (sinePatternLayers[patternIndex].ledMask & (1 << ledIndex))){
				uint16_t interval = (sinePatternLayers[patternIndex].currentTime + ledIndex * sinePatternLayers[patternIndex].positionVariance + sinePatternLayers[patternIndex].phaseOffset) % 256;
				uint16_t thisHsv[3] = {
					calculateSinVariation(sinePatternLayers[patternIndex].hues[0], sinePatternLayers[patternIndex].hues[1], interval),
					calculateSinVariation(sinePatternLayers[patternIndex].saturations[0], sinePatternLayers[patternIndex].saturations[1], interval),
					calculateSinVariation(sinePatternLayers[patternIndex].brightnesses[0], sinePatternLayers[patternIndex].brightnesses[1], interval)
				};
				for (uint8_t armIndex = 0; armIndex < NUM_ARMS; armIndex++){
					if (sinePatternLayers[patternIndex].armMask & (1<<armIndex)){
						if (patternIndex == 0){
							for (uint8_t colorIndex = 0; colorIndex < 3; colorIndex++){
								hsv[armIndex][colorIndex] = thisHsv[colorIndex];
							}
						}
						else{
							int diff = (int) thisHsv[0] - hsv[armIndex][0];
							uint16_t oldHue;
							uint16_t newHue;
							if (diff < - 180){
								oldHue = hsv[armIndex][0];
								newHue = thisHsv[0] + 360;
							}
							else if (diff > 180){
								oldHue = hsv[armIndex][0] + 360;
								newHue = thisHsv[0];
							}
							else {
								oldHue = hsv[armIndex][0];
								newHue = thisHsv[0];
							}
							uint16_t oldFactor = 100 * hsv[armIndex][2];
							uint16_t newFactor = sinePatternLayers[patternIndex].strengthFactor * thisHsv[2];
							long dividingFactor = (oldFactor + newFactor);
							long val = (long) oldHue * oldFactor + (long) newHue * newFactor;
							val /= dividingFactor;
							hsv[armIndex][0] = val % 360;
							val = (long) hsv[armIndex][1] * oldFactor + (long) thisHsv[1] * newFactor;
							val /= dividingFactor;
							hsv[armIndex][1] = val;
							val = (long) hsv[armIndex][2] * oldFactor + (long) thisHsv[2] * newFactor;
							val /= dividingFactor;
							hsv[armIndex][2] = val;
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
	for (uint8_t patternIndex=0; patternIndex < NUM_PATTERN_LAYERS; patternIndex++){
		uint16_t val = (sinePatternLayers[patternIndex].currentTime + sinePatternLayers[patternIndex].timeStep) % 256;
		sinePatternLayers[patternIndex].currentTime = val;
	}
	keyFrameExpiration = millis() + KEYFRAMESPACING;
}
void shiftKeyFrames(){
	Serial.println("begin shift frames");
	for (uint8_t armIndex = 0; armIndex < NUM_ARMS; armIndex++){
		for (uint8_t ledIndex=0; ledIndex < NUM_LEDS_PER_ARM; ledIndex++){
			for (uint8_t colorIndex = 0; colorIndex < 3; colorIndex++){
				frames[0][armIndex][ledIndex][colorIndex] = frames[1][armIndex][ledIndex][colorIndex];
				frames[1][armIndex][ledIndex][colorIndex] = frames[2][armIndex][ledIndex][colorIndex];
				Serial.print(frames[0][armIndex][ledIndex][colorIndex]);
				Serial.print(", ");
			}
			Serial.print(";");
			
		}
		Serial.println ("arm");
	}
}
uint16_t calculateSinVariation(uint16_t bottomValue, uint16_t topValue, uint8_t interval){
	//Serial.println("calculateSinVariation");
	long val = bottomValue + (((long) topValue-bottomValue) * pgm_read_byte(&sinCurve256[interval]))/256;
	//Serial.println(val);
	return val;
}


void getRGB(uint16_t hue, uint8_t sat, uint8_t val, uint16_t colors[3]) { 



	/* From http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
		convert hue, saturation and brightness ( HSB/HSV ) to RGB
		 The dim_curve is used only on brightness/value and on saturation (inverted).
		 This looks the most natural.      
	*/
	//	Serial.println(val);
	val = pgm_read_byte(&dimCurve[val]);
	sat = 255 - pgm_read_byte(&dimCurve[255-sat]);
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
		base = ((255 - sat) * (val))/255;
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
		colors[0]=r*16;
		colors[1]=g*16;
		colors[2]=b*16; 
		/*Serial.println(hue/60);
		Serial.println(r);
		Serial.println(g);
		Serial.println(b);
		Serial.println("===");*/
	}   

}

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
	sinePatternLayers[0].timeStep = 20;
	sinePatternLayers[0].phaseOffset = 0;
	sinePatternLayers[0].positionVariance = 10;
	sinePatternLayers[0].ledMask = 0b0000001111111111;
	sinePatternLayers[0].armMask = 0b00000111;
	sinePatternLayers[0].hues[0] = 260;
	sinePatternLayers[0].hues[1] = 320;
	sinePatternLayers[0].saturations[0] = 200;
	sinePatternLayers[0].saturations[1] = 200;
	sinePatternLayers[0].brightnesses[0] = 255;
	sinePatternLayers[0].brightnesses[1] = 10;
	
	sinePatternLayers[1].strengthFactor = 90;
	sinePatternLayers[1].timeStep = 35;
	sinePatternLayers[1].phaseOffset = 0;
	sinePatternLayers[1].positionVariance = 20;
	sinePatternLayers[1].ledMask = 0b0000001111000000;
	sinePatternLayers[1].armMask = 0b00000111;
	sinePatternLayers[1].hues[0] = 0;
	sinePatternLayers[1].hues[1] = 20;
	sinePatternLayers[1].saturations[0] = 200;
	sinePatternLayers[1].saturations[1] = 200;
	sinePatternLayers[1].brightnesses[0] = 255;
	sinePatternLayers[1].brightnesses[1] = 10;
	for (i=0; i<3; i++){
		calculateNextKeyFrame();
	}
}
void loop(){
	if (millis() > keyFrameExpiration){
		calculateNextKeyFrame();
	}
	unsigned long frameAge = KEYFRAMESPACING - (keyFrameExpiration - millis());
	for (uint8_t armIndex = 0; armIndex < NUM_ARMS; armIndex++){
		for (uint8_t ledIndex = 0; ledIndex < NUM_LEDS_PER_ARM; ledIndex++){
			uint16_t rgb[3];
			for (uint8_t colorIndex = 0; colorIndex < 3; colorIndex++){
				long val = ((long)frames[1][armIndex][ledIndex][colorIndex] - frames[0][armIndex][ledIndex][colorIndex]);
				val *= frameAge;
				val /= KEYFRAMESPACING;
				val += frames[0][armIndex][ledIndex][colorIndex];
				/*if (ledIndex == 0){
					Serial.print(val);
					Serial.print(", ");
				}*/
				rgb[colorIndex] = val;
			}
			/*if (ledIndex == 0){
				Serial.println("-");
			}*/
			writeRGBColor(armIndex, ledOrdering[ledIndex], rgb);
			//Serial.println("writing LED");
		}
	}
}


