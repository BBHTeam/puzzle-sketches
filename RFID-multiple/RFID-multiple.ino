#include <MFRC522.h>
#include <EEPROM.h>

//#define Debug
#define RFIDTagLength			4
#define OutputPin				9		//this pin will be high as long the right RFID tag is hold above the reader


const uint8_t RFIDReaderPins[] = {2, 3, 4, 5, 6, 7}; //connect it to the SDA pins of the RFID readers
const uint32_t MasterTag = 0x42424242;		//change it to the UID of your master tag

MFRC522 RFIDReader[sizeof(RFIDReaderPins)];
bool RFIDReaderIsConnected[sizeof(RFIDReaderPins)];
bool previousState = LOW;


void setup() {
	pinMode(OutputPin, OUTPUT);
	#ifdef Debug
	Serial.begin(9600);
	#endif
	SPI.begin();
	for (uint8_t i = 0; i < sizeof(RFIDReaderPins); i++) {
		RFIDReader[i].PCD_Init(RFIDReaderPins[i], 255);
		RFIDReaderIsConnected[i] = RFIDReader[i].PCD_ReadRegister(0x22);
		#ifdef Debug
		if (RFIDReaderIsConnected[i]){
			Serial.print("Found RFID reader on pin ");
			Serial.println(RFIDReaderPins[i]);
		}
		#endif
	}
}


void loop() {
	bool validTag = true;
	for (uint8_t i = 0; i < sizeof(RFIDReaderPins) && validTag; i++) {
		if (RFIDReaderIsConnected[i]) {
			if (RFIDReader[i].PICC_IsNewCardPresent() && RFIDReader[i].PICC_ReadCardSerial()) {
				for (uint8_t j = 0; j < RFIDTagLength && validTag; j++) {
					if ((MasterTag >> (8*(RFIDTagLength-j-1)) & 255) != RFIDReader[i].uid.uidByte[j]) validTag = false;
				}
				if (validTag) {
					#ifdef Debug
					Serial.println("Found master tag");
					Serial.println("Begin editing tags");
					#endif
					RFIDReader[i].PICC_HaltA();
					SetRFIDTags(i);
				}
				else {
					RFIDReader[i].PICC_IsNewCardPresent();			//necassary, only every second call returns true if a card is hold above the reader
					RFIDReader[i].PICC_ReadCardSerial();			//necassary, only every second call returns true if a card is hold above the reader
					for (uint16_t j = 0; j < EEPROM.length()/RFIDTagLength/sizeof(RFIDReaderPins) && !validTag; j++) {
						validTag = true;
						for (uint8_t k = 0; k < RFIDTagLength && validTag; k++) {
							if (EEPROM.read(i*RFIDTagLength + j*RFIDTagLength*sizeof(RFIDReaderPins) + k) != RFIDReader[i].uid.uidByte[k]) validTag = false;
						}
					}
				}
			}
			else {
				validTag = false;
			}
		}
	}
	if (validTag) {
		if (previousState == LOW) {
			#ifdef Debug
			Serial.println("All tags were placed");
			#endif
			digitalWrite(OutputPin, HIGH);
			previousState = HIGH;
		}
	}
	else if (previousState == HIGH) {
		#ifdef Debug
		Serial.println("At least one tag was removed");
		#endif
		digitalWrite(OutputPin, LOW);
		previousState = LOW;
	}
}


void SetRFIDTags(uint8_t readerNum) {
	while (1) {
		if (RFIDReader[readerNum].PICC_IsNewCardPresent() && RFIDReader[readerNum].PICC_ReadCardSerial()) {
			bool isMasterTag = true;
			for (uint8_t i = 0; i < RFIDTagLength && isMasterTag; i++) {
				if ((MasterTag >> (8*(RFIDTagLength-i-1)) & 255) != RFIDReader[readerNum].uid.uidByte[i]) isMasterTag = false;
			}
			if (isMasterTag) {
				#ifdef Debug
				Serial.println("End editing tags");
				#endif
				RFIDReader[readerNum].PICC_HaltA();
				return;
			}
			bool knownTag = false;
			for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength/sizeof(RFIDReaderPins) && !knownTag; i++) {
				knownTag = true;
				for (uint8_t j = 0; j < RFIDTagLength && knownTag; j++) {
					if (EEPROM.read(i*RFIDTagLength*sizeof(RFIDReaderPins) + readerNum*RFIDTagLength + j) != RFIDReader[readerNum].uid.uidByte[j]) knownTag = false;
				}
				if (knownTag) {
					#ifdef Debug
					Serial.print("Delete tag ");
					#endif
					for (uint8_t j = 0; j < RFIDTagLength; j++) {
						#ifdef Debug
						Serial.print('\t');
						Serial.print(RFIDReader[readerNum].uid.uidByte[j], HEX);
						#endif
						EEPROM.write(i*RFIDTagLength*sizeof(RFIDReaderPins) + readerNum*RFIDTagLength + j, 255);
					}
					#ifdef Debug
					Serial.println("");
					#endif
				}
			}
			if (!knownTag) {
				bool tagSaved = false;
				for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength/sizeof(RFIDReaderPins) && !tagSaved; i++) {
					if (EEPROM.read(i*RFIDTagLength*sizeof(RFIDReaderPins) + readerNum*RFIDTagLength) == 255) {
						#ifdef Debug
						Serial.print("Save tag ");
						#endif
						for (uint8_t j = 0; j < RFIDTagLength; j++) {
							#ifdef Debug
							Serial.print('\t');
							Serial.print(RFIDReader[readerNum].uid.uidByte[j], HEX);
							#endif
							EEPROM.write(i*RFIDTagLength*sizeof(RFIDReaderPins) + j, RFIDReader[readerNum].uid.uidByte[j]);
						}
						#ifdef Debug
						Serial.println("");
						#endif
						tagSaved = true;
					}
				}
			}
		}
		RFIDReader[readerNum].PICC_HaltA();
	}
}
