#ifndef CROW_UTILS_H
#define CROW_UTILS_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>
#include "settings.h"
#include "animations.h"

// External objects defined in the main .ino
extern Servo beakServo;
extern DFRobotDFPlayerMini dfPlayer;

static uint8_t lastSentPWM = -1;

/**
 * Handles the logic updating servo position
 */
bool updateBeak() {
  int targetPWM = getEasedAnimPWM();

  if (targetPWM != -1) {
    if (targetPWM != lastSentPWM) {
      beakServo.writeMicroseconds(targetPWM);
      lastSentPWM = targetPWM;
      return true;
    }
  } else {
    lastSentPWM = -1; 
  }
  return false;
}

#endif