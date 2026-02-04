/*
 * Animatronic Crow Controller
 * ---------------------------
 *
 * Features:
 * - Dual-core: Core1 monitors motion sensor, Core0 runs animations
 * - Scolding, idle movements, random squawks
 * - Synchronized beak animations with audio files
 * - Non-blocking control
 * - Random eye blinking
 * - Test mode for sensor debugging
 * - LD1020 mode enables animation cooldown to prevent self-triggering
 * - BUTTON mode for "Try Me" functionality
 * 
 * >> "User Configuration" is located in settings.h <<
 * 
 */
#include <Arduino.h>
#include <Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <AccelStepper.h>

// SENSOR MODE CODES - Do not modify these values
#define SENSOR_MODE_PIR 0
#define SENSOR_MODE_LD1020 1
#define SENSOR_MODE_NONE 2
#define SENSOR_MODE_BUTTON 3  // connect button to PIN_MOTION_SENSOR/GND

#include "settings.h"
#include "animations.h"
#include "crow-utils.h"

// ============================================================================
// GLOBAL OBJECTS 
// ============================================================================
AccelStepper stepper(AccelStepper::HALF4WIRE, PIN_STEPPER_1, PIN_STEPPER_3, PIN_STEPPER_2, PIN_STEPPER_4);
Servo beakServo;
DFRobotDFPlayerMini dfPlayer;

#if SHOW_NEOPIXEL_STATUS
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel statusLED(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
#endif

// Neck position definitions
const int NECK_CENTER = 0;
const int NECK_SIDE = NECK_RANGE / 2;

// ============================================================================
// STATE TRACKING
// ============================================================================
enum CrowMode {
  MODE_IDLE,
  MODE_IDLE_MOVE,
  MODE_SCOLDING,
  MODE_SQUAWKING,
  MODE_RESETTING
};

volatile bool sensorCurrentlyHigh = false;
volatile bool buttonDefaultState = HIGH;
volatile bool buttonTriggered = false;
bool buttonSequenceActive = false;
unsigned long buttonWaitMs = 0;
unsigned long lastButtonStepTime = 0;
uint8_t buttonStep = 0;

CrowMode currentMode = MODE_IDLE;
unsigned long lastIdleMoveTime = 0;
unsigned long nextIdleMoveTime = 0;
unsigned long lastIdleSquawkTime = 0;
unsigned long nextIdleSquawkTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkTime = 0;
unsigned long lastAudioTime = 0;
unsigned long movementStart = 0;
unsigned long movementEnd = 0;

// ============================================================================
// SETUP - CORE 0
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(7500);

  Serial.println(F("\n========================================"));
  Serial.println(F("Crow Animation Controller"));
  if (TEST_MODE) {
    Serial.println(F("*** TEST MODE ACTIVE ***"));
    Serial.println(F("Eyes mirror sensor: ON=HIGH, OFF=LOW"));
  }
  if (SENSOR_MODE == SENSOR_MODE_LD1020) {
    Serial.println(F("*** LD1020 RADAR MODE ***"));
    Serial.print(F("Animation cooldown: "));
    Serial.print(LD1020_ANIMATION_COOLDOWN_MS);
    Serial.println(F("ms"));
  } else if (SENSOR_MODE == SENSOR_MODE_PIR) {
    Serial.println(F("*** PIR SENSOR MODE ***"));
  } else if (SENSOR_MODE == SENSOR_MODE_BUTTON) {
    Serial.println(F("*** BUTTON MODE ***"));
  } else {
    Serial.println(F("*** NO SENSOR CONFIGURED ***"));
  }
  Serial.println(F("========================================\n"));

  initializeNeopixel();
  showPixel(0, 50, 0); // NeoPixel: green

  randomSeed(analogRead(A0));

  initializeEyes();
  initializeBeak();
  initializeNeck();
  initializeDFPlayer();

  resetIdleTimers();

  // Start up sensor and Core1 for monitoring if available
  if (SENSOR_MODE != SENSOR_MODE_NONE) {
    initializeMotionSensor();
    rp2040.resumeOtherCore();
  } else {
    rp2040.idleOtherCore();
  }

  showPixel(0, 0, 0); // NeoPixel: off
  Serial.println(F("✓ Initialization complete. Crow is alive!"));
}

// ============================================================================
// SETUP - CORE 1 (Sensor Monitoring)
// ============================================================================
void setup1() {
  // Core1 setup - sensor monitoring
}

// ============================================================================
// LOOP - CORE 1 (Sensor Monitoring Thread)
// ============================================================================
void loop1() {
  if (SENSOR_MODE == SENSOR_MODE_NONE) {
    delay(1000);  // Just sleep forever
    return;
  }

  const unsigned long DEBOUNCE_MS = 50;
  if (SENSOR_MODE == SENSOR_MODE_BUTTON) {
    // Button mode: Detect state changes (debounced)
    static bool lastState = buttonDefaultState;
    static unsigned long lastChangeTime = 0;

    bool currentState = digitalRead(PIN_MOTION_SENSOR);

    // Detect change from default state (button pressed)
    if (currentState != buttonDefaultState && currentState != lastState) {
      if (millis() - lastChangeTime > DEBOUNCE_MS) {
        if (!buttonSequenceActive) {
          buttonTriggered = true;
        }
        lastChangeTime = millis();
      }
    }

    lastState = currentState;
    delay(DEBOUNCE_MS);

  } else {
    // PIR or LD1020 mode: Monitor for HIGH state
    sensorCurrentlyHigh = digitalRead(PIN_MOTION_SENSOR);
    delay(DEBOUNCE_MS);
  }
}

// ============================================================================
// MAIN LOOP - CORE 0
// ============================================================================
void loop() {
  unsigned long now = millis();

  // Always run stepper
  stepper.run();

  // Animate Beak 
  updateBeak();

  // BUTTON MODE: Handle button sequence
  if (SENSOR_MODE == SENSOR_MODE_BUTTON) {
    // Handle blinking in test mode or during sequence
    if (TEST_MODE) {
      digitalWrite(PIN_LED_EYES, buttonTriggered ? HIGH : LOW);
    } else if (buttonSequenceActive) {
      handleBlinking(now);
    }

    // Check for button trigger
    if (buttonTriggered && !buttonSequenceActive) {
      lastButtonStepTime = millis();
      buttonStep = 0;
    }
    executeButtonSequence(now);
    return;  // Skip all other mode logic
  }

  // Handle eye behavior based on mode
  if (TEST_MODE) {  // Test mode: eyes mirror sensor state
    digitalWrite(PIN_LED_EYES, sensorCurrentlyHigh ? HIGH : LOW);
  } else {  // Normal mode: random blinking
    handleBlinking(now);
  }

  // LD1020 Mode: Check if cooldown period has elapsed
  bool ld1020Clear = true;
  if (SENSOR_MODE == SENSOR_MODE_LD1020) {
    long moveTime = max(movementEnd, movementStart);
    ld1020Clear = (now - moveTime >= LD1020_ANIMATION_COOLDOWN_MS);
    if (!ld1020Clear) return;  // Skip other behaviors during cooldown
  }

  // Prevent rapidly-repeating squawks and scolds
  bool squawkEnabled = (now - lastAudioTime >= SCOLD_SQUAWK_BLOCK_MS);

  // Scold when triggered and available
  if (currentMode != MODE_SCOLDING && sensorCurrentlyHigh && !animating && squawkEnabled && ld1020Clear) {
    Serial.println(F("[Scold]  Motion detected! Scolding..."));
    // Interrupt idle neck movement if in progress
    if (currentMode == MODE_IDLE && stepper.distanceToGo() != 0) {
      Serial.println(F("[Break]  Stopping idle movement for scold"));
      stepper.stop();
    }
    startScoldSequence();
    return;  // Skip other behaviors this loop
  }

  // Handle current mode
  switch (currentMode) {
    case MODE_IDLE:
      handleIdleMode(now, squawkEnabled, ld1020Clear);
      break;

    case MODE_IDLE_MOVE:
      if (stepper.distanceToGo() == 0) {
        currentMode = MODE_IDLE;
        movementEnd = millis();
      }
      break;

    case MODE_SCOLDING:
      // Wait for animation to complete
      if (!animating && stepper.distanceToGo() == 0) {
        if (SENSOR_MODE == SENSOR_MODE_LD1020) {
          Serial.print(F("[LD1020] Scold complete, entering "));
          Serial.print(LD1020_ANIMATION_COOLDOWN_MS);
          Serial.println(F("ms cooldown"));
        } else {
          Serial.println(F("[Scold]  Complete. Returning to idle"));
        }
        currentMode = MODE_IDLE;
        resetIdleMoveTime();
        addBlockToSquawkTime();
        lastAudioTime = millis();
        movementEnd = millis();
      }
      break;

    case MODE_SQUAWKING:
      // Wait for animation to complete
      if (!animating && stepper.distanceToGo() == 0) {
        currentMode = MODE_IDLE;
        Serial.println(F("[Squawk] Complete. Returning to idle"));
        lastAudioTime = millis();
        movementEnd = millis();
      }
      break;

    case MODE_RESETTING:
      // Wait for neck to center
      if (stepper.distanceToGo() == 0) {
        currentMode = MODE_IDLE;
      }
      break;
  }
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void initializeEyes() {
  pinMode(PIN_LED_EYES, OUTPUT);
  digitalWrite(PIN_LED_EYES, HIGH);
  Serial.println(F("[Init]   Eyes online"));
  delay(500);

  if (!TEST_MODE) {
    digitalWrite(PIN_LED_EYES, LOW);
    delay(500);
  }
}

void initializeBeak() {
  showPixel(25, 25, 25); // NeoPixel: white
  int mid = (SERVO_PWM_OPEN + SERVO_PWM_CLOSED) / 2;
  beakServo.writeMicroseconds(mid);  // start center
  beakServo.attach(PIN_SERVO);
  delay(200);
  for (int p = mid; p > SERVO_PWM_OPEN; p--) {  // move open
    beakServo.writeMicroseconds(p);
    delay(2);
  }
  for (int p = SERVO_PWM_OPEN; p < SERVO_PWM_CLOSED; p++) {  // move closed
    beakServo.writeMicroseconds(p);
    delay(2);
  }
  beakServo.writeMicroseconds(SERVO_PWM_CLOSED);
  delay(200);
  beakServo.detach();
  Serial.println(F("[Init]   Beak servo online"));
  delay(500);
}

void initializeNeck() {
  showPixel(0, 50, 25); // NeoPixel: teal
  setNeckSpeedSlow();

  // Center the neck through a calibration sequence
  Serial.println(F("[Init]   Centering neck..."));
  stepper.setCurrentPosition(0);
  stepper.moveTo(NECK_RANGE + 100);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  stepper.moveTo(-(NECK_RANGE / 2 + 50));
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  Serial.println(F("[Init]   Neck centered and online"));

  setNeckSpeedFast();
  delay(500);
}

void initializeDFPlayer() {
  showPixel(50, 25, 0); // NeoPixel: orange
  Serial1.setTX(PIN_DFPLAYER_TX);
  Serial1.setRX(PIN_DFPLAYER_RX);
  Serial1.begin(9600);
  delay(1000);

  if (!dfPlayer.begin(Serial1, true, true)) {
    Serial.println(F("[Init]   ✗ DFPlayer Mini failed!"));
    showPixel(50, 0, 0); // NeoPixel: red
    delay(2000);
  } else {
    Serial.println(F("[Init]   DFPlayer Mini online"));
    showPixel(50, 0, 50); // NeoPixel: purple
    
    dfPlayer.volume(DFPLAYER_VOLUME);
    delay(200);
    dfPlayer.play(11);
  }
  delay(2500);
}

void initializeMotionSensor() {
  if (SENSOR_MODE == SENSOR_MODE_BUTTON) {
    // Button mode: Use INPUT_PULLUP and detect default state
    pinMode(PIN_MOTION_SENSOR, INPUT_PULLUP);
    delay(100);  // Let pin stabilize

    buttonDefaultState = digitalRead(PIN_MOTION_SENSOR);
    showPixel(0, 0, 50); // NeoPixel: blue

    Serial.println(F("[Init]   Button sensor online (INPUT_PULLUP)"));
    Serial.print(F("[Init]   Button default state: "));
    Serial.println(buttonDefaultState == HIGH ? "HIGH (NO)" : "LOW (NC)");
    Serial.println(F("[Init]   Waiting for button press test..."));

    unsigned long startTime = millis();
    bool detected = false;

    while (millis() - startTime < 5000) {
      if (digitalRead(PIN_MOTION_SENSOR) != buttonDefaultState) {
        Serial.println(F("[Init]   ✓ Button press detected!"));
        detected = true;
        // Wait for release
        while (digitalRead(PIN_MOTION_SENSOR) != buttonDefaultState) {
          delay(50);
        }
        break;
      }
      delay(100);
    }

    if (!detected) {
      Serial.println(F("[Init]   No button press detected (this is OK)"));
    }

  } else {
    // PIR or LD1020 mode: Standard INPUT
    pinMode(PIN_MOTION_SENSOR, INPUT);
    showPixel(0, 0, 50); // NeoPixel: blue

    Serial.println(F("[Init]   Motion sensor online"));
    Serial.println(F("[Init]   Waiting for motion test..."));

    unsigned long startTime = millis();
    bool detected = false;

    while (millis() - startTime < 5000) {
      if (digitalRead(PIN_MOTION_SENSOR) == HIGH) {
        Serial.println(F("[Init]   ✓ Motion detected!"));
        detected = true;
        break;
      }
      delay(100);
    }

    if (!detected) {
      Serial.println(F("[Init]   No motion detected (this is OK)"));
    }
  }

  delay(500);
}

void initializeNeopixel() {
  // Initialize status LED (conditional)
#if SHOW_NEOPIXEL_STATUS
  pinMode(PIN_NEOPIXEL_POWER, OUTPUT);
  digitalWrite(PIN_NEOPIXEL_POWER, HIGH);
  statusLED.begin();
  statusLED.setBrightness(50);
#endif
}

// ============================================================================
// MODE HANDLERS
// ============================================================================

void executeButtonSequence(unsigned long now) {
  if (buttonTriggered) {
    if (!buttonSequenceActive) {
      buttonSequenceActive = true;
      Serial.println(F("[Button] ===== STARTING BUTTON SEQUENCE ====="));
    }

    // Check if we need to wait before advancing
    if (now - lastButtonStepTime < buttonWaitMs) return;

    switch (buttonStep) {
      case 0:
        Serial.println(F("[Button] Eyes ON"));
        digitalWrite(PIN_LED_EYES, HIGH);
        nextBlinkTime = now + random(BLINK_MIN_INTERVAL_MS, BLINK_MAX_INTERVAL_MS);
        buttonWaitMs = 800;
        lastButtonStepTime = now;
        buttonStep++;
        break;
      case 1:
        Serial.println(F("[Button] Scolding"));
        startScoldSequence();
        buttonStep++;
        break;
      case 2:
        if (!animating && stepper.distanceToGo() == 0) {
          buttonWaitMs = random(1000, 3000);
          lastButtonStepTime = now;
          buttonStep++;
        }
        break;
      case 3:
        Serial.println(F("[Button] Movement"));
        startIdleMove(now);
        buttonStep++;
        break;
      case 4:
        if (stepper.distanceToGo() == 0) {
          buttonWaitMs = random(1200, 2400);
          lastButtonStepTime = now;
          buttonStep++;
        }
        break;
      case 5:
        Serial.println(F("[Button] Squawk"));
        animateAudio(random(8, 15));
        buttonStep++;
        break;
      case 6:
        if (!animating && stepper.distanceToGo() == 0) {
          buttonWaitMs = random(1000, 3000);
          lastButtonStepTime = now;
          buttonStep++;
        }
        break;
      case 7:
        Serial.println(F("[Button] Centering Neck"));
        setNeckSpeedSlow();
        stepper.moveTo(NECK_CENTER);
        buttonStep++;
      case 8:
        if (stepper.distanceToGo() == 0) {
          Serial.println(F("[Button] Eyes OFF"));
          digitalWrite(PIN_LED_EYES, LOW);
          Serial.println(F("[Button] ===== SEQUENCE COMPLETE ====="));
          buttonSequenceActive = false;
          buttonTriggered = false;
        }
        break;
    }
  }
}

void handleIdleMode(unsigned long now, bool squawkEnabled, bool ld1020Clear) {

  // Random neck movements
  if (ld1020Clear && now >= nextIdleMoveTime && stepper.distanceToGo() == 0 && !animating) {

    // Randomly scold if there is no sensor
    if (SENSOR_MODE == SENSOR_MODE_NONE) {
      if (squawkEnabled && random(0, 4) == 0) {
        Serial.println(F("[Scold]  Idle scold! Scolding..."));
        startScoldSequence();
        resetIdleMoveTime();
        return;
      }
    }
    // Trigger idle movement and (re)set volume
    dfPlayer.volume(DFPLAYER_VOLUME);
    startIdleMove(now);
  }

  // Random idle squawks
  if (ld1020Clear && squawkEnabled && now >= nextIdleSquawkTime && !animating) {
    startIdleSquawk();
    resetIdleSquawkTime();
  }
}

void startScoldSequence() {
  currentMode = MODE_SCOLDING;
  movementStart = millis();
  lastAudioTime = millis();

  // Move neck to +/- 20% position
  setNeckSpeedFast();
  int rangePercent = random(0, NECK_RANGE_SCOLD_PERCENT + 1);
  int direction = random(0, 2) == 0 ? 1 : -1;
  int scoldPos = (NECK_SIDE * rangePercent / 100) * direction;
  stepper.moveTo(scoldPos);

  Serial.print(F("[Scold]  Turning head to "));
  Serial.println(scoldPos);
  // Choose random scold sound (tracks 1-7)
  animateAudio(random(1, 8));
}

void startIdleSquawk() {
  Serial.println(F("[Squawk] Random squawk..."));
  currentMode = MODE_SQUAWKING;
  movementStart = millis();
  lastAudioTime = millis();
  // Choose random squawk sound (tracks 8-14)
  animateAudio(random(8, 15));
}

void startIdleMove(unsigned long now) {
  // Randomly choose slow or fast movement
  if (random(0, 2) == 0) setNeckSpeedFast();
  else setNeckSpeedSlow();

  // Generate random position: 30-100% of range in either direction
  int rangePercent = random(IDLE_NECK_MIN_PERCENT, IDLE_NECK_MAX_PERCENT + 1);
  int direction = random(0, 2) == 0 ? 1 : -1;  // Left or right
  int targetPos = (NECK_SIDE * rangePercent / 100) * direction;

  stepper.moveTo(targetPos);

  Serial.print(F("[Idle]   Moving neck to "));
  Serial.print(targetPos);
  Serial.print(F(" ("));
  Serial.print(rangePercent);
  Serial.println(F("% range)"));
  currentMode = MODE_IDLE_MOVE;
  movementStart = now;

  resetIdleMoveTime();
}

void resetIdleTimers() {
  resetIdleMoveTime();
  resetIdleSquawkTime();
}

void resetIdleMoveTime() {
  nextIdleMoveTime = millis() + random(IDLE_MOVE_MIN_MS, IDLE_MOVE_MAX_MS);
}

void resetIdleSquawkTime() {
  nextIdleSquawkTime = millis() + random(IDLE_SQUAWK_MIN_MS, IDLE_SQUAWK_MAX_MS);
}

void addBlockToSquawkTime() {
  nextIdleSquawkTime = millis() + SCOLD_SQUAWK_BLOCK_MS;
}

// ============================================================================
// NECK CONTROL
// ============================================================================

void setNeckSpeedSlow() {
  stepper.setMaxSpeed(NECK_SPEED_SLOW_MAX);
  stepper.setAcceleration(NECK_SPEED_SLOW_ACCEL);
}

void setNeckSpeedFast() {
  stepper.setMaxSpeed(NECK_SPEED_FAST_MAX);
  stepper.setAcceleration(NECK_SPEED_FAST_ACCEL);
}

void resetNeckToCenter() {
  stepper.stop();
  setNeckSpeedFast();
  stepper.moveTo(NECK_CENTER);

  // Wait for completion
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

// ============================================================================
// AUDIO CONTROL
// ============================================================================

void animateAudio(uint8_t trackNum) {
  uint8_t idx = trackNum - 1;

  if (idx < NUM_ANIMATIONS) {
    Serial.print(F("► Audio  Playing track "));
    Serial.print(trackNum);
    Serial.print(F(" at time "));
    Serial.println(millis() / 1000);

    dfPlayer.play(trackNum);

    // queue animation with delay to get DFPlayer started
    queuePendingAnimation(idx, millis() + AUDIO_SYNC_DELAY_MS);
  } else {
    Serial.println(F("✗ Audio  Track index out of bounds!"));
  }
}

// ============================================================================
// EYE BLINKING
// ============================================================================

void handleBlinking(unsigned long now) {
  static bool eyesOpen = true;
  static unsigned long blinkStartTime = 0;

  if (eyesOpen) {
    // Check if it's time to blink
    if (now >= nextBlinkTime) {
      digitalWrite(PIN_LED_EYES, LOW);
      eyesOpen = false;
      blinkStartTime = now;
    }
  } else {
    // Check if blink is complete
    if (now - blinkStartTime >= BLINK_DURATION_MS) {
      digitalWrite(PIN_LED_EYES, HIGH);
      eyesOpen = true;
      nextBlinkTime = now + random(BLINK_MIN_INTERVAL_MS, BLINK_MAX_INTERVAL_MS);
    }
  }
}

// ============================================================================
// NeoPixel (on-board LED) status
// ============================================================================
void showPixel(int r, int g, int b) {
#if SHOW_NEOPIXEL_STATUS
  statusLED.setPixelColor(0, statusLED.Color(r, g, b));
  statusLED.show();
#endif
}