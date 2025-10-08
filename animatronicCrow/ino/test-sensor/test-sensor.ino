/*
 * Monitors a motion sensor and controls an LED.
 * The LED blinks on startup and then indicates the current motion status.
 */
#include <Arduino.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define PIN_LED_EYES            14
#define PIN_MOTION_SENSOR       15

// ============================================================================
// STATE TRACKING
// ============================================================================
int lastSensorState = LOW; // Stores the previous state of the sensor

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n--- Sensor Test Initializing ---"));

  // Initialize the LED pin
  pinMode(PIN_LED_EYES, OUTPUT);

  // Blink the LED 5 times to indicate the system is online
  Serial.print(F("Calibrating..."));
  for (int i = 0; i < 5; i++) {
    digitalWrite(PIN_LED_EYES, HIGH);
    delay(100);
    digitalWrite(PIN_LED_EYES, LOW);
    delay(100);
  }
  Serial.println(F(" Online!"));

  // Initialize the sensor pin
  pinMode(PIN_MOTION_SENSOR, INPUT);
  Serial.println(F("Waiting for movement..."));
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
  // Read the current state of the motion sensor
  int currentSensorState = digitalRead(PIN_MOTION_SENSOR);

  // Check if the sensor state has changed since the last reading
  if (currentSensorState != lastSensorState) {
    if (currentSensorState == HIGH) {
      // The sensor has just detected motion
      digitalWrite(PIN_LED_EYES, HIGH); // Turn the LED on
      Serial.println(F("Motion DETECTED"));
    } else {
      // The sensor has just stopped detecting motion
      digitalWrite(PIN_LED_EYES, LOW); // Turn the LED off
      Serial.println(F("All quiet..."));
    }
    // Update the last known state
    lastSensorState = currentSensorState;
  }

  // A small delay to prevent the loop from running too fast, saving resources.
  delay(50);
}