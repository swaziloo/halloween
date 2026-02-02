#ifndef CROW_UTILS_H
#define CROW_UTILS_H

#include <Arduino.h>
#include <Servo.h>
#include <DFRobotDFPlayerMini.h>
#include "settings.h"
#include "animations.h"

// External objects defined in the main .ino
extern Servo beakServo;
extern DFRobotDFPlayerMini dfPlayer;

static uint8_t lastSentPWM = -1;

/**
 * Handles the logic of attaching/detaching the servo and updating 
 * its position
 */
bool updateBeak() {
  int targetPWM = getEasedAnimPWM();

  if (targetPWM != -1) {
    if (targetPWM != lastSentPWM) {
      beakServo.writeMicroseconds(targetPWM);
      if (!beakServo.attached()) beakServo.attach(PIN_SERVO);
      lastSentPWM = targetPWM;
      return true;
    }
  } else if (beakServo.attached()) {
    // Animation finished
    beakServo.detach();
    lastSentPWM = -1; 
  }
  return false;
}

#endif