/*
 * Controls a beak servo motor and an associated LED.
 * The servo cycles through three positions at a regular interval,
 * and the LED blinks with each movement.
 */
#include <Arduino.h>
#include <Servo.h>

// ============================================================================
// CONFIGURATION - 
// ============================================================================
// Servo position settings (in degrees)
#define SERVO_PWM_MIN           1000    // default: adjust to actual
#define SERVO_PWM_MAX           2000    // default: adjust to actual
#define BEAK_OPEN_DEG           0       // Fully open position
#define BEAK_HALF_DEG           25      // Half open position
#define BEAK_CLOSED_DEG         55      // Fully closed position
#define BEAK_MOVE_INTERVAL_MS   4000    // Interval between servo movements

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define PIN_SERVO               2       // SRV1 (29 on CC5x12 v1.2)
#define PIN_LED_EYES            14      // LED1

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
Servo beakServo;

// ============================================================================
// STATE TRACKING
// ============================================================================
unsigned long lastBeakMoveMillis = 0;
int targetBeakPos = BEAK_OPEN_DEG; 

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200); // Initialize Serial Monitor [cite: 4]
  Serial.println(F("\n--- Servo Test Initializing ---"));

  // Attach servo and set initial position
  beakServo.attach(PIN_SERVO, SERVO_PWM_MIN, SERVO_PWM_MAX);
  beakServo.write(targetBeakPos);
  Serial.print(F("Servo initialized to position: "));
  Serial.println(targetBeakPos);

  // Initialize LED pin and turn it on [cite: 5]
  pinMode(PIN_LED_EYES, OUTPUT);
  digitalWrite(PIN_LED_EYES, HIGH);
  Serial.println(F("LED initialized and turned ON."));

  lastBeakMoveMillis = millis(); // Initialize the timer
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to move the servo again [cite: 6]
  if (currentMillis - lastBeakMoveMillis >= BEAK_MOVE_INTERVAL_MS) {
    lastBeakMoveMillis = currentMillis; // Reset the timer

    // Blink the LED to indicate movement
    digitalWrite(PIN_LED_EYES, LOW);
    delay(300);
    digitalWrite(PIN_LED_EYES, HIGH);

    // Cycle through the three target positions
    if (targetBeakPos == BEAK_OPEN_DEG) {         // From open to half-open
      targetBeakPos = BEAK_HALF_DEG;
    } else if (targetBeakPos == BEAK_HALF_DEG) {  // From half-open to closed
      targetBeakPos = BEAK_CLOSED_DEG;
    } else {                                      // From closed to open
      targetBeakPos = BEAK_OPEN_DEG;
    }

    // Move the servo to the new target position
    beakServo.write(targetBeakPos);
    Serial.print(F("Moving servo to position: "));
    Serial.println(targetBeakPos);
  }
}
