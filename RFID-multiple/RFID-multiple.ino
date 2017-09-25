#include <MFRC522.h>
#include <SPI.h>

/*#define RFIDReader1Pin 			2		//connect it to the SDA pin of the first MFRC522
#define RFIDReader2Pin 			3		//connect it to the SDA pin of the second MFRC522
#define RFIDReader3Pin 			4		//connect it to the SDA pin of the third MFRC522
#define RFIDReader4Pin 			5		//connect it to the SDA pin of the fourth MFRC522
#define RFIDReader5Pin 			6		//connect it to the SDA pin of the fifth MFRC522
#define RFIDReader6Pin 			7		//connect it to the SDA pin of the sixth MFRC522

MFRC522 RFIDReader1(Reader1Pin, 255);
MFRC522 RFIDReader2(Reader2Pin, 255);
MFRC522 RFIDReader3(Reader3Pin, 255);
MFRC522 RFIDReader4(Reader4Pin, 255);
MFRC522 RFIDReader5(Reader5Pin, 255);
MFRC522 RFIDReader6(Reader6Pin, 255);*/

const uint8_t RFIDReaderPins[] = {2, 3, 4, 5, 6, 7}; //connect it to the SDA pins of the RFID readers
MFRC522* RFIDReader[sizeof(RFIDReaderPins)];
bool RFIDReaderIsConnected[sizeof(RFIDReaderPins)];


void setup() {
	for (uint8_t i = 0; i < sizeof(RFIDReaderPins); i++) {
		pinMode(RFIDReaderPins[i], OUTPUT);
		digitalWrite(RFIDReaderPins[i], HIGH);
	}
	Serial.begin(9600);
	SPI.begin();
	for (uint8_t i = 0; i < sizeof(RFIDReaderPins); i++) {
		digitalWrite(RFIDReaderPins[i], LOW);
		SPI.transfer(0xA2);
		RFIDReaderIsConnected[i] = SPI.transfer(0);
		digitalWrite(RFIDReaderPins[i], HIGH);
		if (RFIDReaderIsConnected[i]) {
			RFIDReader[i] = new MFRC522(RFIDReaderPins[i], 255);
			RFIDReader[i]->PCD_Init();
		}
	}
	//for (uint8_t i = 0; i < RFIDReaderCount; i++)
	
	//if ()
	
	//if (ReaderTest.PCD_ReadRegister(0x22))
}


void loop() {
	
}
