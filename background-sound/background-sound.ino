#include <SoftwareSerial.h>

#define Mp3ModulePin		A5		// connect it over 1kOhm with the RX pin of the DFPlayerMini
#define Volume				30		// max: 30
#define DebounceTime		1000

SoftwareSerial Mp3Module(255, Mp3ModulePin);

const uint8_t ButtonPins[] = {2, 3, 4, 5, 6, 7};
bool lastState[sizeof(ButtonPins)];
uint32_t previousMillis[sizeof(ButtonPins)];


void setup() {
	for (uint8_t i = 0; i < sizeof(ButtonPins); i++) {
		pinMode(ButtonPins[i], INPUT_PULLUP);
	}
	Mp3Module.begin(9600);
	SetVolume(Volume);
	while (millis() < 2) {} // make sure, millis() returns at least 2 to prevent complications with buttons pressed while starting up
}


void loop() {
	for (uint8_t i = 0; i < sizeof(ButtonPins); i++) {
		if (digitalRead(ButtonPins[i]) == LOW) {
			if (previousMillis[i] == 0) {
				previousMillis[i] = millis();
			}
			else if (previousMillis[i] != 1 && millis() - previousMillis[i] >= DebounceTime) {
				PlayFile(i+1);
				previousMillis[i] = 1;
			}
		}
		else if (previousMillis[i] != 0) {
			previousMillis[i] = 0;
		}
	}
}


void SendCommand(uint8_t cmd, uint8_t paraml=0x00, uint8_t paramh=0x00) {
	uint8_t buf[] = {0x7E, 0xFF, 0x06, cmd, 0x00, paramh, paraml, 0xEF};
	for (uint8_t i = 0; i < 8; i++) {
		Mp3Module.write(buf[i]);
	}
}


void SetVolume(uint8_t volume) {
	SendCommand(0x10, volume, 0x01);
}


void PlayFile(uint8_t file){
	SendCommand(0x03, file, 0);
}
