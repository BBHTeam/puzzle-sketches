#include <SoftwareSerial.h>

#define Mp3ModulePin		A1		//connect over 1k Ohm to the RX pin of the first DFPlayerMini
#define Volume				15		//max: 30
#define RelayPin			13
#define ReceiverPin			10
#define DialPin				11
#define DebounceTime		30
#define MaxImpulseTime		300		//in ms
#define MaxWaitingTime		5000	//in ms

SoftwareSerial Mp3Module(Mp3ModulePin, 255);

const uint8_t telephoneNumber[] = {1, 2, 3, 4};


void setup() {
	pinMode(RelayPin, OUTPUT);
	pinMode(ReceiverPin, INPUT_PULLUP);
	pinMode(DialPin, INPUT);
	Mp3Module.begin(9600);
	SetVolume(Volume);
}


void loop() {
	if (digitalRead(ReceiverPin) == HIGH) {
		PlayFile(1);
		delay(DebounceTime);
		uint32_t previousMillis = millis();
		bool lastState = LOW;
		uint8_t counter = 0;
		uint8_t currentNumberIndex = 0;
		do {
			if (digitalRead(DialPin) == LOW) {
				do {
					if (digitalRead(DialPin) == LOW) {
						if (lastState == HIGH) {
							lastState = LOW;
							counter++;
							previousMillis = millis();
							delay(DebounceTime);
						}
					}
					else if (lastState == LOW) {
						lastState = HIGH;
						delay(DebounceTime);
					}
				} while (millis() - previousMillis <= MaxImpulseTime);
				if (counter == telephoneNumber[currentNumberIndex] || (counter == 10 && telephoneNumber[currentNumberIndex] == 0)) {
					currentNumberIndex++;
					if (currentNumberIndex == sizeof(telephoneNumber)) {
						PlayFile(3);
						// digitalWrite(RelayPin, HIGH);
						currentNumberIndex = 0;
					}
				}
				else {
					currentNumberIndex = 0;
				}
				counter = 1;
			}
			
			if (millis() - previousMillis >= 50000) {
				PlayFile(2);
				currentNumberIndex = 0;
			}
		} while (digitalRead(ReceiverPin) == HIGH);
	} 
}


void SendCommand(uint8_t cmd, uint8_t paraml=0x00, uint8_t paramh=0x00) {
	uint8_t buf[] = {0x7E, 0xFF, 0x06, cmd, 0x00, paramh, paraml, 0xEF};
	for (uint8_t i = 0; i < 8; i++) {
		Mp3Module.write(buf[i]);
	}
}


void SetVolume(byte volume) {
	SendCommand(0x10, volume, 0x01);
}


void LoopFile(uint8_t file) {
	SendCommand(0x08, file, 0);
}


void PlayFile(uint8_t file){
	SendCommand(0x03, file, 0);
}

