/*
 * Crow Calibration
 * ---------------------------
 * Util for direct manipulation of crow functions using the Ardunio Serial Monitor.
 * Enter '?' for command summary. 
 * 
 * Minimal crow setup requires correctly setting PWM (beak servo) range and (probably) volume
 * 
 * >> "User Configuration" is located in settings.h <<
 * 
 */
#include <Arduino.h>
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <AccelStepper.h>
#include "settings.h"
#include "animations.h"
#include "crow-utils.h"

// Reuse production objects
AccelStepper stepper(AccelStepper::HALF4WIRE, PIN_STEPPER_1, PIN_STEPPER_3, PIN_STEPPER_2, PIN_STEPPER_4);
Servo beakServo;
DFRobotDFPlayerMini dfPlayer;

unsigned int targetPulse = 1200;
unsigned int currentPulse = 1150;
unsigned long lastPulseUpdate = 0;

bool easingLUTSet = false;

unsigned int beakOpen = 0;
unsigned int beakClosed = 0;
unsigned int dfVolume = 0;
unsigned int audDelay = AUDIO_SYNC_DELAY_MS;
float eFactor = SERVO_EASING_FACTOR;

unsigned long moveStartTime = 0;
enum TestState {
  SWEEP,
  CENTER,
  NONE
};
TestState currentNeckState = NONE;
int stepperMoveIdx = 0;

bool eyesMirrorSensor = false;

void setup() {
  Serial.begin(115200);
  delay(7500);
  while(!Serial); 
  Serial.setTimeout(10);
  
  printInstructions();

  // configure servo
  beakServo.attach(PIN_SERVO); 
  beakServo.setTimerWidth(16);

  // Turn on Eye LEDs
  pinMode(PIN_LED_EYES, OUTPUT);
  digitalWrite(PIN_LED_EYES, HIGH);

    // Setup Motion Sensor
  pinMode(PIN_MOTION_SENSOR, INPUT);
  
  // Setup DFPlayer-Mini
  Serial1.begin(9600, SERIAL_8N1, PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
  dfPlayer.begin(Serial1);
  delay(2000);
  dfPlayer.volume(15);
  delay(200);
}

void loop() {
  unsigned long now = millis();
  // Maintain Animation State
  if (now - lastPulseUpdate < 3000 || !updateBeak()) moveServo(now);
  updateNeck();
  // Stepper Run Always
  stepper.run();
  if (eyesMirrorSensor) {
    digitalWrite(PIN_LED_EYES, digitalRead(PIN_MOTION_SENSOR));
  } 

  // Serial Command Processing
  if (Serial.available() > 0) {
    char cmd = tolower(Serial.read());
    
    switch(cmd) {
      case 'b': { // Manual PWM Position
        int val = Serial.parseInt();
        if (val >= 500 && val <= 2500) { // SG90: check your unit
            if (currentPulse == targetPulse) currentPulse-=10;
            targetPulse = val;
            Serial.print(F("Beak: moving to ")); Serial.println(val);
        }
        break;
      }
      case 's': {
        beakOpen = Serial.parseInt();
        beakClosed = Serial.parseInt();
        float power = Serial.parseFloat();
        if (beakOpen == 0 || beakClosed == 0) {
          Serial.println(F("Two limits (open and closed) are required"));
          break;
        }
        if (power > 0.0f) eFactor = power;
        hydrateEasingLUT(beakOpen, beakClosed, eFactor);
        easingLUTSet = true;
        Serial.println(F("Beak: limits set:"));
        Serial.print(F("SERVO_PWM_OPEN   ")); Serial.println(beakOpen);
        Serial.print(F("SERVO_PWM_CLOSED ")); Serial.println(beakClosed);
        break;
      }
      case 'f': {
        float power = Serial.parseFloat();
        if (!easingLUTSet) {
          Serial.println(F("Factor setting is blocked until Beak Servo limits are set"));
          break;
        }
        if (power > 0.0f) eFactor = power;
        hydrateEasingLUT(beakOpen, beakClosed, eFactor);
        Serial.println(F("Easing table regenerated"));
        }
        break;
      case 'v': {
        dfVolume = Serial.parseInt();
        dfPlayer.volume(dfVolume);
        Serial.print(F("Volume set to ")); Serial.println(dfVolume);
        break;
      }
      case 'a': {
        if (!easingLUTSet) {
          Serial.println(F("Animations are blocked until Beak Servo limits are set"));
          break;
        }
        int track = Serial.parseInt();
        int delay = Serial.parseInt();
        if (delay > 0) audDelay = delay;
        if (track >= 1 && track <= NUM_ANIMATIONS) {
          dfPlayer.play(track);
          queuePendingAnimation(track - 1, millis() + audDelay);
          Serial.print(F("Playing Track: ")); Serial.print(track);
          Serial.print(F(" with ")); Serial.print(audDelay); Serial.println(F("ms delay"));
        }
        break;
      }
      case 'n': {
        int val = Serial.parseInt();
        int max = Serial.parseInt();
        if (val == 0) { // stop
          stepper.moveTo(0);
          currentNeckState = NONE;
          Serial.println(F("Neck: stopping"));
        } else if (val == -1) { // center neck
          stepperMoveIdx = 0;
          currentNeckState = CENTER;
          Serial.println(F("Neck: centering"));
        } else { // sweep at accel
          int testMax = (max > 0) ? max : 7000;
          stepper.setMaxSpeed(testMax);
          stepper.setAcceleration(val);
          stepperMoveIdx = 0;
          currentNeckState = SWEEP;
          Serial.print(F("Neck: testing accel ")); Serial.print(val);
          Serial.print(F(" max ")); Serial.println(testMax);
        }
        break;
      }
      case 'e': {
        int val = Serial.parseInt();
        if (val == 0) {
          eyesMirrorSensor = false;
          digitalWrite(PIN_LED_EYES, HIGH);
          Serial.println(F("Eyes: ON"));
        } else if (val == 1) {
          eyesMirrorSensor = true;
          digitalWrite(PIN_LED_EYES, LOW);
          Serial.println(F("Eyes: Mirror sensor/button"));
        }
        break;
      }
      case 'p': {
        if (beakOpen > 0)                    { Serial.print(F("#define SERVO_PWM_OPEN      ")); Serial.println(beakOpen); }
        if (beakClosed > 0)                  { Serial.print(F("#define SERVO_PWM_CLOSED    ")); Serial.println(beakClosed); }
        if (dfVolume > 0)                    { Serial.print(F("#define DFPLAYER_VOLUME     ")); Serial.println(dfVolume); }
        if (audDelay != AUDIO_SYNC_DELAY_MS) { Serial.print(F("#define AUDIO_SYNC_DELAY_MS ")); Serial.println(audDelay); }
        if (eFactor != SERVO_EASING_FACTOR)  { Serial.print(F("#define SERVO_EASING_FACTOR ")); Serial.println(eFactor); }
        break;
      }
      case '?': {
        printInstructions();
      }
    }
  }
}

void moveServo(unsigned long now) {
  if (currentPulse != targetPulse) {
    if (now - lastPulseUpdate > 5) {
      int diff = targetPulse - currentPulse;
      if (abs(diff) <= 10) currentPulse = targetPulse;
      else currentPulse += (diff > 0) ? 20 : -20;
      beakServo.writeMicroseconds(currentPulse);
      lastPulseUpdate = now;
    }
  } 
}

void updateNeck() {
  switch(currentNeckState) {
    case CENTER:
      switch(stepperMoveIdx) {
        case 0:
          stepper.setMaxSpeed(2500);
          stepper.setAcceleration(500);
          stepper.moveTo(NECK_RANGE + 100);
          stepperMoveIdx++;
          break;
        case 1: 
          if (stepper.distanceToGo() == 0) {
            stepper.setCurrentPosition(0);
            stepper.moveTo(-(NECK_RANGE / 2 + 50));
            stepperMoveIdx++;
          }
          break;
        case 2:
          if (stepper.distanceToGo() == 0) {
            stepper.setCurrentPosition(0);
            currentNeckState = NONE;
          }
          break;
      }
      break;

    case SWEEP:
      switch(stepperMoveIdx) {
        case 0:
          stepper.moveTo(0);
          if (stepper.distanceToGo() == 0) {
            stepperMoveIdx++;
          }
          break;
        case 1:
          if (stepper.distanceToGo() == 0) {
            stepper.moveTo((NECK_RANGE-100)/2);
            stepperMoveIdx++;
          }
          break;
        case 2:
          if (stepper.distanceToGo() == 0) {
            stepper.moveTo(-(NECK_RANGE-100)/2);
            stepperMoveIdx--;
          }
          break;
      }
      break;

    case NONE:
      stepperMoveIdx = 0;
      break;
  }
}

void printInstructions() {
  Serial.println(F("--- Crow Diagnostic & Calibration Utility ------------------------------------"));
  Serial.println(F("Commands:"));
  Serial.println(F("  b <val>           : Move Beak Servo to PWM (e.g. 'm 1000')"));
  Serial.println(F("  s <open> <closed> : Set Beak Servo limits open closed (e.g. 's 1100 1400') "));
  Serial.println(F("  a <num> <delay>   : Play Animation/Track # (1-14) (+optional audio delay ms)"));
  Serial.println(F("  v <0-30>          : Set DFPlayer Volume"));
  Serial.println(F("  n -1              : Neck Stepper: Center"));
  Serial.println(F("  n 0               : Neck Stepper: Stop"));
  Serial.println(F("  n <accel> <max>   : Neck Stepper: Test accel (+optional max speed) sweep"));
  Serial.println(F("  f <float>         : Animation smoothing factor (1.0: smoother 4.0: snappier)"));
  Serial.println(F("  e <0-1>           : Eyes mirror button/sensor: 0 for NO, 1 for YES"));
  Serial.println(F("------------------------------------------------------------------------------"));
}