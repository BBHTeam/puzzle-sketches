#include <SoftwareSerial.h>

#define Mp3Module1Pin		A1		//connect it to 1k Ohm with the RX pin of the first DFPlayerMini
#define Mp3Module2Pin		A5		//connect it to 1k Ohm with the RX pin of the first DFPlayerMini
#define BusyPin				A2		//connect it to the BUSY pin of the second DFPlayerMini 
#define VolumeModule1		15		//max: 30
#define VolumeModule2		15		//max: 30

SoftwareSerial Mp3Module1(Mp3Module1Pin, 255); 
SoftwareSerial Mp3Module2(Mp3Module2Pin, 255); 

const uint8_t ButtonPins[] 	= {7, 8, 9, 10, 11, 12};


void setup() {
	pinMode(BusyPin, INPUT_PULLUP);
	for (uint8_t i = 0; i < sizeof(ButtonPins); i++) {
		pinMode(ButtonPins[i], INPUT_PULLUP);
	}
	Mp3Module1.begin(9600);
	Mp3Module2.begin(9600);
	SetVolume(Mp3Module1, VolumeModule1);
	SetVolume(Mp3Module1, VolumeModule1);
	LoopFile(Mp3Module2, 1);
}


void loop() {
	for (uint8_t i = 0; i < sizeof(ButtonPins); i++) {
		if (digitalRead(ButtonPins[i]) == LOW && digitalRead(BusyPin) == HIGH) {
			PlayFile(Mp3Module2, i);
			delay(20);
		}
	}
}


void SendCommand(Stream &_Serial, uint8_t cmd, uint8_t paraml=0x00, uint8_t paramh=0x00) {
	uint8_t buf[] = {0x7E, 0xFF, 0x06, cmd, 0x00, paramh, paraml, 0xEF};
	for (uint8_t i = 0; i < 8; i++) {
		_Serial.write(buf[i]);
	}
}


void SetVolume(Stream &_Serial, byte volume) {
	SendCommand(_Serial, 0x10, volume, 0x01);
}


void LoopFile(Stream &_Serial, uint8_t file) {
	SendCommand(_Serial, 0x08, file, 0);
}


void PlayFile(Stream &_Serial, uint8_t file){
	SendCommand(_Serial, 0x03, file, 0);
}
