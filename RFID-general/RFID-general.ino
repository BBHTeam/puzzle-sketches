#include <MFRC522.h>
#include <EEPROM.h>

#define Debug					0		// set this to 1 to enable debug over the Serial interface
#define RFIDReaderPin			10		// connect it to the SDA pin of the RFID reader
#define OutputPin				9		// this pin will be high as long the right RFID tag is hold above the reader
#define LEDGreenPin				6		// connect this pin to the green LED
#define LEDRedPin				7		// connect this pin to the red LED
#define RFIDTagLength			4


MFRC522 RFIDReader(RFIDReaderPin, 255);

const uint32_t MasterTag = 0x42424242;		// change it to the UID of your master tag
bool lastState = LOW;


void setup() {
	pinMode(OutputPin, OUTPUT);
	#if Debug
	Serial.begin(9600);
	#endif
	SPI.begin();
	RFIDReader.PCD_Init();
}


void loop() {
	if (RFIDReader.PICC_IsNewCardPresent() && RFIDReader.PICC_ReadCardSerial()) {
		bool validTag = true;
		for (uint8_t i = 0; i < RFIDTagLength && validTag; i++) {
			if ((MasterTag >> (8*(RFIDTagLength-i-1)) & 255) != RFIDReader.uid.uidByte[i]) validTag = false;
		}
		if (validTag) {
			#if Debug
			Serial.println("Found master tag");
			Serial.println("Begin editing tags");
			#endif
			RFIDReader.PICC_HaltA();
			digitalWrite(LEDGreenPin, LOW);		// switch off the green LED first to prevent over current when using a duo LED with just one resistor
			digitalWrite(LEDRedPin, HIGH);
			SetRFIDTags();
		}
		else {
			RFIDReader.PICC_IsNewCardPresent();			//necassary, only every second call returns true if a card is hold above the reader
			RFIDReader.PICC_ReadCardSerial();			//necassary, only every second call returns true if a card is hold above the reader
			for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength && !validTag; i++) {
				validTag = true;
				for (uint8_t j = 0; j < RFIDTagLength && validTag; j++) {
					if (EEPROM.read(i*RFIDTagLength + j) != RFIDReader.uid.uidByte[j]) validTag = false;
				}
			}
			if (validTag && lastState == LOW) {
				#if Debug
				Serial.println("Found valid tag");
				#endif
				digitalWrite(OutputPin, HIGH);
				lastState = HIGH;
			}
		}
	}
	else if (lastState == HIGH) {
		#if Debug
		Serial.println("Tag was removed");
		#endif
		digitalWrite(OutputPin, LOW);
		lastState = LOW;
	}
}


void SetRFIDTags() {
	while (1) {
		if (RFIDReader.PICC_IsNewCardPresent() && RFIDReader.PICC_ReadCardSerial()) {
			bool isMasterTag = true;
			for (uint8_t i = 0; i < RFIDTagLength && isMasterTag; i++) {
				if ((MasterTag >> (8*(RFIDTagLength-i-1)) & 255) != RFIDReader.uid.uidByte[i]) isMasterTag = false;
			}
			if (isMasterTag) {
				#if Debug
				Serial.println("End editing tags");
				#endif
				RFIDReader.PICC_HaltA();
				digitalWrite(LEDRedPin, LOW);		// switch off the red LED first to prevent over current when using a duo LED with just one resistor
				digitalWrite(LEDGreenPin, HIGH);
				return;
			}
			bool knownTag = false;
			for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength && !knownTag; i++) {
				knownTag = true;
				for (uint8_t j = 0; j < RFIDTagLength && knownTag; j++) {
					if (EEPROM.read(i*RFIDTagLength + j) != RFIDReader.uid.uidByte[j]) knownTag = false;
				}
				if (knownTag) {
					#if Debug
					Serial.print("Delete tag ");
					#endif
					for (uint8_t j = 0; j < RFIDTagLength; j++) {
						#if Debug
						Serial.print('\t');
						Serial.print(RFIDReader.uid.uidByte[j], HEX);
						#endif
						EEPROM.write(i*RFIDTagLength + j, 255);
					}
					#if Debug
					Serial.println("");
					#endif
				}
			}
			if (!knownTag) {
				bool tagSaved = false;
				for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength && !tagSaved; i++) {
					if (EEPROM.read(i*RFIDTagLength) == 255) {
						#if Debug
						Serial.print("Save tag ");
						#endif
						for (uint8_t j = 0; j < RFIDTagLength; j++) {
							#if Debug
							Serial.print('\t');
							Serial.print(RFIDReader.uid.uidByte[j], HEX);
							#endif
							EEPROM.write(i*RFIDTagLength + j, RFIDReader.uid.uidByte[j]);
						}
						#if Debug
						Serial.println("");
						#endif
						tagSaved = true;
					}
				}
			}
		}
		RFIDReader.PICC_HaltA();
	}
}
