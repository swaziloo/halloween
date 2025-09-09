// test-servo.ino
// blink on led pin
// initialize/play serial audio DFPlayer Mini
#include <Arduino.h>
#include <Servo.h>

const int ledPin = 14;
const int servoPin = 2;

int beakOpenDeg = 0;
int beakCloseDeg = 30;
int servoBeakDelayMs = 4000; 

unsigned long currentMillis;
unsigned long posBeakMillis;
int targetBeakPos = beakOpenDeg;

Servo BeakServo;

void setup() {  
  if (Serial) Serial.begin(115200); // Initialize Serial0 (Serial Monitor)

  // turn on and home beak
  BeakServo.attach(servoPin);
  BeakServo.write(targetBeakPos);

  posBeakMillis = millis();

  // turn on eyes 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  currentMillis = millis();
  if (currentMillis - posBeakMillis >= servoBeakDelayMs) {
    digitalWrite(ledPin, LOW);
    delay(300);
    digitalWrite(ledPin, HIGH);
    posBeakMillis = currentMillis;

    if (targetBeakPos == beakOpenDeg) {
      targetBeakPos = (beakCloseDeg/2);
    } else if (targetBeakPos == (beakCloseDeg/2)) {
      targetBeakPos = beakCloseDeg;
    } else if (targetBeakPos == beakCloseDeg) {
      targetBeakPos = beakOpenDeg;
    }

    BeakServo.write(targetBeakPos);
    Serial.print(F("Wrote BeakPos:"));
    Serial.println(targetBeakPos);
  }
}
