// test-all-with-loop
// Crow Board Test
// Uses Creature Control 5x12 stepper 1, servo 1, PIR 1, dfplayer, and LED1
#include <Arduino.h>
#include <Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>

const int txPin = 0;
const int rxPin = 1;
const int pirPin = 15;
const int ledPin = 14;
const int servoPin = 2;
const int stepPin1 = 5;
const int stepPin2 = 6;
const int stepPin3 = 7;
const int stepPin4 = 8;

unsigned int beakOpenDeg = 0;
unsigned int beakCloseDeg = 30;

int neckRange = 740;
int neckLeft = neckRange/2+1;
int neckRight = -(neckRange/2-1);
int center = 410;
unsigned int nextRndmMove = 8000;
int idlePositions[] = {neckLeft, 0, neckRight};
const int numPositions = sizeof(idlePositions) / sizeof(idlePositions[0]);
#define HALFSTEP 8
AccelStepper stepper1(HALFSTEP, stepPin1, stepPin3, stepPin2, stepPin4);

Servo beakServo;

#define FPSerial Serial1
DFRobotDFPlayerMini myDFPlayer;
unsigned int volume = 30; // volume range from 0 to 30

#define PIN 16 // Define the pin connected to the WS2812 LED
#define NUMPIXELS 1 // Number of LEDs in the strip
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long currentMillis = 0;
unsigned long movedMillis = 0;
unsigned long pirMillis = 0;
unsigned int pirBlock = 7500;

unsigned long blinkedMillis = 0;
unsigned int blinkDelayMs;
unsigned int blinkTimeMs = 90; // closed/dark duration

void printDetail(uint8_t type, int value);
void neckSpeed(float max, float accel, float speed);

// arduino initialization 
void setup() {
	pixels.begin(); // Initialize the NeoPixel library
  	// Indicate start of setup with green color
 	pixels.setPixelColor(0, pixels.Color(0, 64, 0)); // green
 	pixels.show(); 
	if (Serial) Serial.begin(115200); // Initialize Serial0 (Serial Monitor)
	randomSeed(analogRead(0)); 
	initializeEyes();
	initializeBeak();
	initializeNeck();
	initializeCaw();
	initializeMotionDetection();
	Serial.println(F("Initialization complete."));
	pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
    pixels.show();
    movedMillis = millis();
}

// arduino main loop
void loop() {
	currentMillis = millis(); 
	maybeBlink();
	checkSerialIn();
	randomNeck();
	stepper1.run();
	if (digitalRead(pirPin) == HIGH) scold();
	// if (myDFPlayer.available()) printDetail(myDFPlayer.readType(), myDFPlayer.read());
}

void scold() {
	if (currentMillis - pirMillis > pirBlock) {
		neckReset();
	    myDFPlayer.next();
	    beakServo.write(beakOpenDeg);
	    delay(200);
	    beakServo.write(beakCloseDeg/2);
	    delay(400);
	    beakServo.write(beakOpenDeg);
	    delay(200);
	    beakServo.write(beakCloseDeg/2);
	    delay(750);
	    beakServo.write(beakOpenDeg);
	    delay(400);
	    beakServo.write(beakCloseDeg);
	    pirMillis = millis();
	    movedMillis = millis();
	}
}

void randomNeck() {
	if (currentMillis - movedMillis > nextRndmMove) {
		int rndmIndx = random(0, numPositions);
		int rndmPos = idlePositions[rndmIndx];
		Serial.print(F("random move to "));
		Serial.println(rndmPos);
		stepper1.moveTo(rndmPos);
		movedMillis = millis();
		nextRndmMove = random(5000,12000);
	}
}

void neckReset() {
	stepper1.stop();
	neckSpeedFast();
	stepper1.moveTo(0);    
	while (stepper1.distanceToGo() != 0) {
    	stepper1.run();
    }
}

void neckSpeedSlow() {
	stepper1.setMaxSpeed(4000.0);
	stepper1.setAcceleration(500.0);
	stepper1.setSpeed(3000);
}

void neckSpeedFast() {
	stepper1.setMaxSpeed(10000.0);
	stepper1.setAcceleration(5000.0);
	stepper1.setSpeed(5000);
}

void checkSerialIn() {
	if (Serial.available() > 0) {
		String cmd = Serial.readString();
		cmd.trim();

		if (cmd == "caw") myDFPlayer.next();
		else if (cmd == "blink") blink();
		else if (cmd == "open") openShut();
		else if (cmd == "shake") shake();
		else if (cmd == "scold") scold();
		else Serial.print(F("Unknown command: "));
		Serial.println(cmd);
	}
}

void maybeBlink () {
	if (currentMillis - blinkedMillis >= blinkDelayMs) {
		digitalWrite(ledPin ,LOW);
	}
	if (currentMillis - blinkedMillis >= blinkDelayMs + blinkTimeMs) {
		blinkedMillis = currentMillis;
		blinkDelayMs = random(4000, 10000);
		digitalWrite(ledPin, HIGH);
	}
}

void initializeEyes() {
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);
	delay(3000);
	blink();
	Serial.println(F("eyes online."));
	delay(2000);
}

void blink() {
	digitalWrite(ledPin, LOW);
	delay(blinkTimeMs);
	digitalWrite(ledPin, HIGH);
}

void initializeBeak() {
	beakServo.attach(servoPin);
	openShut();
	Serial.println(F("beakServo online."));
	delay(2000);
}

void openShut() {
	beakServo.write(beakOpenDeg);
	delay(1000);
	beakServo.write(beakCloseDeg/2);
	delay(500);
	beakServo.write(beakCloseDeg);
}

void initializeNeck() {
	neckSpeedSlow();
	int ref = 0;
	while (true) {
		if (stepper1.distanceToGo() == 0) {
			if (ref == 0) {
				stepper1.setCurrentPosition(0);
				stepper1.moveTo(neckRange+100);
				ref = center;
			} else if (ref == center) {
				stepper1.setCurrentPosition(0);
				stepper1.moveTo(-center);
				ref = -center;
			} else {
				stepper1.setCurrentPosition(0);
				ref = -1;
				break;
			}
		}
		stepper1.run();
	}
	Serial.print(F("stepper1 centered at "));
	Serial.println(-center);
	neckSpeedFast();
	shake();
	Serial.println(F("stepper1 online."));
	delay(2000);
}

void shake() {
	int ref = -1;
	while (true) {
		if (stepper1.distanceToGo() == 0) {
			if (ref == -1) ref = neckRight;
			else if (ref == neckRight) ref = neckLeft;
			else if (ref == neckLeft) ref = 0;
			else break;
			stepper1.moveTo(ref);
		}
		stepper1.run();
	}
}

void initializeCaw() {
	pixels.setPixelColor(0, pixels.Color(64, 42, 0)); // orange
  	pixels.show();	
  	FPSerial.begin(9600); // Initialize Serial1 (DFPlayer Mini)
	delay(3000);
	if (!myDFPlayer.begin(FPSerial, true, true)) { // isAck true, doReset true
		Serial.println(F("Unable to bring DFPlayer Mini online."));
		pixels.setPixelColor(0, pixels.Color(64, 0, 0)); // red
      	pixels.show();
	    delay(3000);
	} else {
		Serial.println(F("DFPlayer Mini online."));
	    pixels.setPixelColor(0, pixels.Color(64, 0, 64)); // purple
	    pixels.show();
	    delay(3000);
	}
	myDFPlayer.volume(10);
	myDFPlayer.play(1);  //Play the first mp3
	delay(3000);
	myDFPlayer.volume(volume);
}

void initializeMotionDetection() {
  	pinMode(pirPin, INPUT);		
  	pixels.setPixelColor(0, pixels.Color(0, 0, 64)); // blue
    pixels.show();
	Serial.println(F("waiting for movement..."));
  	while (digitalRead(pirPin) == LOW) {
  		delay(10);
  	}
  	pirMillis = millis();
	Serial.println(F("motion detected."));
}

void printDetail(uint8_t type, int value){
	switch (type) {
	case TimeOut:
		Serial.println(F("Time Out!"));
		break;
	case WrongStack:
		Serial.println(F("Stack Wrong!"));
		break;
	case DFPlayerCardInserted:
		Serial.println(F("Card Inserted!"));
		break;
	case DFPlayerCardRemoved:
		Serial.println(F("Card Removed!"));
		break;
	case DFPlayerCardOnline:
		Serial.println(F("Card Online!"));
		break;
	case DFPlayerUSBInserted:
		Serial.println("USB Inserted!");
		break;
	case DFPlayerUSBRemoved:
		Serial.println("USB Removed!");
		break;
	case DFPlayerPlayFinished:
		Serial.print(F("Number:"));
		Serial.print(value);
		Serial.println(F(" Play Finished!"));
		break;
	case DFPlayerError:
		Serial.print(F("DFPlayerError:"));
		switch (value) {
		case Busy:
			Serial.println(F("Card not found"));
			break;
		case Sleeping:
			Serial.println(F("Sleeping"));
			break;
		case SerialWrongStack:
			Serial.println(F("Get Wrong Stack"));
			break;
		case CheckSumNotMatch:
			Serial.println(F("Check Sum Not Match"));
			break;
		case FileIndexOut:
			Serial.println(F("File Index Out of Bound"));
			break;
		case FileMismatch:
			Serial.println(F("Cannot Find File"));
			break;
		case Advertise:
			Serial.println(F("In Advertise"));
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}