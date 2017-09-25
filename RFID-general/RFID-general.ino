#include <MFRC522.h>
#include <EEPROM.h>

#define RFIDReaderPin			10		//connect it to the SDA pin of the RFID reader
#define OutputPin				11		//this is high as long the right RFID tag lays above
#define RFIDTagLength			4

MFRC522 RFIDReader(RFIDReaderPin, 255);

const uint32_t MasterTag = 0xB0E2A059;
bool lastState = LOW;


void setup() {
	pinMode(OutputPin, OUTPUT);
	Serial.begin(9600);
	SPI.begin();
	RFIDReader.PCD_Init();
}


void loop() {
	if (RFIDReader.PICC_IsNewCardPresent() && RFIDReader.PICC_ReadCardSerial()) {
		RFIDReader.PICC_IsNewCardPresent();
		RFIDReader.PICC_ReadCardSerial();
		Serial.println("found tag");
		bool validTag = true;
		for (uint8_t i = 0; i < RFIDTagLength && validTag; i++) {
			if ((MasterTag >> (8*(RFIDTagLength-i-1)) & 255) != RFIDReader.uid.uidByte[i]) validTag = false;
		}
		if (validTag) {
			Serial.println("found master tag");
			SetRFIDTags();
		}
		else {
			for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength && !validTag; i++) {
				validTag = true;
				for (uint8_t j = 0; j < RFIDTagLength && validTag; j++) {
					if (EEPROM.read(i*RFIDTagLength + j) != RFIDReader.uid.uidByte[j]) validTag = false;
				}
			}
			if (validTag && lastState == LOW) {
				digitalWrite(OutputPin, HIGH);
				lastState = HIGH;
			}
		}
	}
	else if (lastState == HIGH) {
		digitalWrite(OutputPin, LOW);
		lastState = LOW;
	}
	/*Serial.print('\t');
	for (uint8_t i; i < 4; i++) {
		Serial.print(RFIDReader.uid.uidByte[i], HEX);
		Serial.print('\t');
	}
	Serial.println("");*/
	//delay(100);
	
}


void SetRFIDTags() {
	while (1) {
		if (RFIDReader.PICC_IsNewCardPresent() && RFIDReader.PICC_ReadCardSerial()) {
			bool isMasterTag = true;
			for (uint8_t i = 0; i < RFIDTagLength && validTag; i++) {
				if ((MasterTag >> (8*(RFIDTagLength-i-1)) & 255) != RFIDReader.uid.uidByte[i]) validTag = false;
			}
			if (validTag) return;
			
		}
	}
}
