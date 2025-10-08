/*
 * 28BYJ-48 Stepper Speed Test Suite
 * For Waveshare RP2040-Zero with ULN2003A driver
 * Tests various speed/acceleration combinations to find maximum reliable speed
 *
 * Pins: GP5-GP8 (IN1-IN4 on ULN2003A)
 */
#include <AccelStepper.h>

// ============================================================================
// CONFIGURATION
// ============================================================================
struct SpeedTest {
  float maxSpeed;
  float acceleration;
};

// Define test parameters - progressively increasing acceleration
// Most movements are < 1 second, so the top speed won't matter
SpeedTest tests[] = {
  {7000, 3000},
  {7000, 4000},
  {7000, 5000},
  {7000, 6000},
  {7000, 7000}
};

// Test movement parameters
const int INITIAL_MOVE = 1400;      // Initial positioning move
const int CENTER_BACK = 750;        // Move back to center
const int TEST_MOVE = 600;          // Test movement distance (+/-)
const int RESET_SPEED = 2000;       // Safe speed for repositioning
const int RESET_ACCEL = 1000;        // Safe acceleration for repositioning

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define PIN1  5  // IN1
#define PIN2  6  // IN2
#define PIN3  7  // IN3
#define PIN4  8  // IN4

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
// Create stepper object (HALF4WIRE mode for 28BYJ-48)
AccelStepper stepper(AccelStepper::HALF4WIRE, PIN1, PIN3, PIN2, PIN4);

// ============================================================================
// STATE TRACKING
// ============================================================================
const int numTests = sizeof(tests) / sizeof(tests[0]);
int currentTest = 0;
unsigned long moveStartTime = 0;
enum TestState {
  STATE_MOVE_POSITIVE,
  STATE_MOVE_NEGATIVE,
  STATE_RETURN_CENTER,
  STATE_COMPLETE,
  STATE_RESET
};
TestState testState = STATE_RESET;

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(10000);

  Serial.println(F("\n========================================"));
  Serial.println(F("28BYJ-48 Stepper Speed Test Suite"));
  Serial.println(F("========================================\n"));
  Serial.println(F("Testing to find maximum reliable speed"));
  Serial.println(F("Watch for stuttering, missed steps, or jitter\n"));

  center();
  startNextTest();
}

// ============================================================================
// START NEXT TEST
// ============================================================================
void startNextTest() {
  if (currentTest >= numTests) {
    Serial.println(F("\n========================================"));
    Serial.println(F("ALL TESTS COMPLETED!"));
    Serial.println(F("========================================"));
    Serial.println(F("\nReview results above to determine maximum reliable speed."));
    Serial.println(F("Look for the highest speed without stuttering or missed steps."));
    while(1) { delay(1000); }  // Stop here
  }

  SpeedTest &test = tests[currentTest];

  Serial.println(F("\n----------------------------------------"));
  Serial.print(F("TEST #"));
  Serial.print(currentTest + 1);
  Serial.print(F(" of "));
  Serial.println(numTests);
  Serial.println(F("----------------------------------------"));
  Serial.print(F("MaxSpeed:     "));
  Serial.println(test.maxSpeed);
  Serial.print(F("Acceleration: "));
  Serial.println(test.acceleration);
  Serial.println();

  // Apply test parameters
  stepper.setMaxSpeed(test.maxSpeed);
  stepper.setAcceleration(test.acceleration);

  if (currentTest == 0) center;
  testState = STATE_MOVE_POSITIVE;

  delay(500);
}

// ============================================================================
// RUN TEST SEQUENCE
// ============================================================================
void loop() {
  static long lastPosition = 0;
  static bool setPos = false;
  static bool setNeg = false;
  static bool setCen = false;

  switch (testState) {
    case STATE_MOVE_POSITIVE:
      if (!setPos) {
        Serial.print(F("Moving to "));
        Serial.print(TEST_MOVE);
        Serial.println(F("..."));
        lastPosition = stepper.currentPosition();
        stepper.moveTo(TEST_MOVE);
        moveStartTime = millis();
        setPos = true;
      }

      stepper.run();

      if (stepper.distanceToGo() == 0) {
        Serial.print(F("Completed in "));
        Serial.print(millis() - moveStartTime);
        Serial.println(F(" ms"));
        testState = STATE_MOVE_NEGATIVE;
      }
      break;

    case STATE_MOVE_NEGATIVE:
      if (!setNeg) {
        Serial.print(F("Moving to -"));
        Serial.print(TEST_MOVE);
        Serial.println(F("..."));
        lastPosition = stepper.currentPosition();
        stepper.moveTo(-TEST_MOVE);
        moveStartTime = millis();
        setNeg = true;
      }

      stepper.run();

      if (stepper.distanceToGo() == 0) {
        Serial.print(F("Completed in "));
        Serial.print(millis() - moveStartTime);
        Serial.println(F(" ms"));
        testState = STATE_RETURN_CENTER;
      }
      break;

    case STATE_RETURN_CENTER:
      if (!setCen) {
        Serial.println(F("Returning to center..."));
        stepper.moveTo(0);
        moveStartTime = millis();
        setCen = true;
      }

      stepper.run();

      if (stepper.distanceToGo() == 0) {
        Serial.print(F("Returned in "));
        Serial.print(millis() - moveStartTime);
        Serial.println(F(" ms"));
        testState = STATE_COMPLETE;
      }
      break;

    case STATE_COMPLETE:
      Serial.println(F("\n*** Test Sequence Complete ***"));
      Serial.println(F("\nWaiting 3 seconds before next test..."));

      delay(3000);
      currentTest++;
      testState = STATE_RESET;
      break;

    case STATE_RESET:
      center();
      setPos = false;
      setNeg = false;
      setCen = false;
      startNextTest();
      break;
  }
}

void center() {
      // Set a safe speed for centering
      stepper.setMaxSpeed(RESET_SPEED);
      stepper.setAcceleration(RESET_ACCEL);

      Serial.print(F("--- Centering..."));

      // Initial positioning
      stepper.moveTo(INITIAL_MOVE);
      while (stepper.distanceToGo() != 0) {
        stepper.run();
      }
      stepper.moveTo(INITIAL_MOVE - CENTER_BACK);
      while (stepper.distanceToGo() != 0) {
        stepper.run();
      }
      stepper.setCurrentPosition(0);

      Serial.println(F("centered. ---\n"));
      delay(2000);
}
