#include <SoftwareSerial.h>

#define Mp3ModulePin		A1		// connect it over 1kOhm with the RX pin of the DFPlayerMini
#define OutputPin 			8		// this pin will be set to HIGH if the puzzle is solved
#define Volume				30		// max: 30

SoftwareSerial Mp3Module(255, Mp3ModulePin); 

const uint8_t PianoKeys[] = {2, 3, 4, 5, 6, 7};		// these pins have to be connected to the buttons in the piano keys
const uint8_t KeyOrder[] = {6, 2, 3, 1, 5, 4};		// this is the order the keys have to be played (1 means first key declard in PianoKeys[])
uint8_t PositionMelody = 0; 						// this variable counts, how many right keys have already been hit, don't change its initial value
bool lastState[sizeof(PianoKeys)];
bool lastOutputState = LOW;


void setup() {
	for (uint8_t i = 0; i < sizeof(PianoKeys); i++) {
		pinMode(PianoKeys[i], INPUT_PULLUP);
	}
	pinMode(OutputPin, OUTPUT);
	Mp3Module.begin(9600);
	SetVolume(30);
}


void loop() {
	for (uint8_t i = 0; i < sizeof(PianoKeys); i++) {
		if (digitalRead(PianoKeys[i]) == LOW && lastState[i] == HIGH) {
			lastState[i] = LOW;
			PlayFile[i];
			if (i+1 == KeyOrder[PositionMelody]) {
				PositionMelody++;
				if (PositionMelody == sizeof(KeyOrder)) {
					if (lastOutputState == HIGH) {
						digitalWrite(OutputPin, LOW);
						lastOutputState = LOW;
					}
					else {
						digitalWrite(OutputPin, HIGH);
						lastOutputState = HIGH;
					}
				}
			}
			else {
				PositionMelody = 0;
			}
		}
		else if (lastState[i] == LOW) {
			lastState[i] = HIGH;
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