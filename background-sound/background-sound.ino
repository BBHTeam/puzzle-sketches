#include <SoftwareSerial.h>

#define Mp3ModulePin		A5		// connect it over 1kOhm with the RX pin of the DFPlayerMini
#define Volume				30		// max: 30
#define DebounceTime		20

SoftwareSerial Mp3Module(255, Mp3ModulePin);

const uint8_t ButtonPins[] = {2, 3, 4, 5, 6, 7};
bool lastState[sizeof(ButtonPins)];


void setup() {
	for (uint8_t i = 0; i < sizeof(ButtonPins); i++) {
		pinMode(ButtonPins[i], INPUT_PULLUP);
	}
	Mp3Module.begin(9600);
	SetVolume(Volume);
}


void loop() {
	for (uint8_t i = 0; i < sizeof(ButtonPins); i++) {
		if (digitalRead(ButtonPins[i]) == LOW) {
			if (lastState[i] == HIGH) {
				lastState[i] = LOW;
				PlayFile(i + 1);
				delay(DebounceTime);
			}
		}
		else if (lastState[i] == LOW) {
			lastState[i] = HIGH;
			delay(DebounceTime);
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
