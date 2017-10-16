#include <MFRC522.h>
#include <EEPROM.h>

#define Debug					0		// set this to 1 to enable debug over the Serial interface
#define OutputPin				9		// this pin will be high as long the right RFID tag is hold above the reader
#define LEDGreenPin				6		// connect this pin to the green LED
#define LEDRedPin				7		// connect this pin to the red LED
#define RFIDTagLength			4


const uint8_t RFIDReaderPins[] = {2, 3, 4, 5, 6, 7};	// connect it to the SDA pins of the RFID readers
const uint32_t MasterTag = 0x42424242;					// change it to the UID of your master tag

MFRC522 RFIDReader[sizeof(RFIDReaderPins)];
bool RFIDReaderIsConnected[sizeof(RFIDReaderPins)];
bool previousState = LOW;


void setup() {
	pinMode(OutputPin, OUTPUT);
	pinMode(LEDGreenPin, OUTPUT);
	pinMode(LEDRedPin, OUTPUT);
	digitalWrite(LEDGreenPin, HIGH);
	for (uint8_t i = 0; i < sizeof(RFIDReaderPins); i++) {
		RFIDReader[i].PCD_Init(RFIDReaderPins[i], 255);
	}
	#if Debug
	Serial.begin(9600);
	#endif
	SPI.begin();
	for (uint8_t i = 0; i < sizeof(RFIDReaderPins); i++) {
		RFIDReaderIsConnected[i] = RFIDReader[i].PCD_ReadRegister(0x22);
		#if Debug
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
					#if Debug
					Serial.println("Found master tag");
					Serial.println("Begin editing tags");
					#endif
					RFIDReader[i].PICC_HaltA();
					digitalWrite(LEDGreenPin, LOW);		// switch off the green LED first to prevent over current when using a duo LED with just one resistor
					digitalWrite(LEDRedPin, HIGH);
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
			#if Debug
			Serial.println("All tags were placed");
			#endif
			digitalWrite(OutputPin, HIGH);
			previousState = HIGH;
		}
	}
	else if (previousState == HIGH) {
		#if Debug
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
				#if Debug
				Serial.println("End editing tags");
				#endif
				RFIDReader[readerNum].PICC_HaltA();
				digitalWrite(LEDRedPin, LOW);		// switch off the red LED first to prevent over current when using a duo LED with just one resistor
				digitalWrite(LEDGreenPin, HIGH);
				return;
			}
			bool knownTag = false;
			for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength/sizeof(RFIDReaderPins) && !knownTag; i++) {
				knownTag = true;
				for (uint8_t j = 0; j < RFIDTagLength && knownTag; j++) {
					if (EEPROM.read(i*RFIDTagLength*sizeof(RFIDReaderPins) + readerNum*RFIDTagLength + j) != RFIDReader[readerNum].uid.uidByte[j]) knownTag = false;
				}
				if (knownTag) {
					#if Debug
					Serial.print("Delete tag ");
					#endif
					for (uint8_t j = 0; j < RFIDTagLength; j++) {
						#if Debug
						Serial.print('\t');
						Serial.print(RFIDReader[readerNum].uid.uidByte[j], HEX);
						#endif
						EEPROM.write(i*RFIDTagLength*sizeof(RFIDReaderPins) + readerNum*RFIDTagLength + j, 255);
					}
					#if Debug
					Serial.println("");
					#endif
				}
			}
			if (!knownTag) {
				bool tagSaved = false;
				for (uint16_t i = 0; i < EEPROM.length()/RFIDTagLength/sizeof(RFIDReaderPins) && !tagSaved; i++) {
					if (EEPROM.read(i*RFIDTagLength*sizeof(RFIDReaderPins) + readerNum*RFIDTagLength) == 255) {
						#if Debug
						Serial.print("Save tag ");
						#endif
						for (uint8_t j = 0; j < RFIDTagLength; j++) {
							#if Debug
							Serial.print('\t');
							Serial.print(RFIDReader[readerNum].uid.uidByte[j], HEX);
							#endif
							EEPROM.write(i*RFIDTagLength*sizeof(RFIDReaderPins) + j, RFIDReader[readerNum].uid.uidByte[j]);
						}
						#if Debug
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
