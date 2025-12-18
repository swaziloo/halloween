/*
 * Animatronic Crow Controller
 *
 * Features:
 * - Dual-core: Core1 monitors motion sensor, Core0 runs animations
 * - Scolding, idle movements, random squawks
 * - Synchronized beak animations with audio files
 * - Non-blocking stepper control
 * - Random eye blinking
 * - Test mode for sensor debugging
 * - LD1020 mode with animation cooldown to prevent self-triggering
 */
#include <Arduino.h>
#include <Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <AccelStepper.h>
#include "animations.h"

// SENSOR MODE CODES - Do not modify these values
#define SENSOR_MODE_PIR               0
#define SENSOR_MODE_LD1020            1
#define SENSOR_MODE_NONE              2

// ============================================================================
// CONFIGURATION - Adjust behavior here
// ============================================================================

// TEST MODE - Set to true to mirror sensor state with eyes (for debugging)
#define TEST_MODE                     false   // true: eyes mirror sensor, false: normal blinking
#define SHOW_NEOPIXEL_STATUS          false   // true: display status color on RP2040-Zero RGB LED

// SENSOR MODE - Choose one mode
#define SENSOR_MODE                   SENSOR_MODE_NONE // Choose: SENSOR_MODE_PIR, SENSOR_MODE_LD1020, SENSOR_MODE_NONE

// Servo Settings
#define SERVO_PWM_MIN                 1000    // default: adjust to actual
#define SERVO_PWM_MAX                 2000    // default: adjust to actual
#define BEAK_OPEN_DEG                 0       // Fully open position
#define BEAK_CLOSED_DEG               45      // Fully closed position

// Audio Settings
#define DFPLAYER_VOLUME               20      // Volume 0-30

// Motion Detection Settings
#define LD1020_ANIMATION_COOLDOWN_MS  8500    // Wait after any animation before checking sensor (LD1020 mode only)
#define SCOLD_SQUAWK_BLOCK_MS         5000    // Delay squawks after scolds and vice-versa

// Idle Behavior Settings
// NOTE: For LD1020 mode, ensure these are > LD1020_ANIMATION_COOLDOWN_MS
// to prevent self-triggering from animations
#define IDLE_MOVE_MIN_MS              9000    // Min time between idle movements (must be > LD1020_ANIMATION_COOLDOWN_MS for LD1020)
#define IDLE_MOVE_MAX_MS              18000   // Max time between idle movements
#define IDLE_SQUAWK_MIN_MS            15000   // Min time between random squawks (or scolds without sensor)
#define IDLE_SQUAWK_MAX_MS            45000   // Max time between random squawks (or scolds without sensor)
#define IDLE_NECK_MIN_PERCENT         30      // Min percent of range for idle moves
#define IDLE_NECK_MAX_PERCENT         100     // Max percent of range for idle moves

// Eye Blink Settings
#define BLINK_DURATION_MS             90      // How long eyes stay closed
#define BLINK_MIN_INTERVAL_MS         3000    // Min time between blinks
#define BLINK_MAX_INTERVAL_MS         8000    // Max time between blinks

// Neck Movement Settings
#define NECK_RANGE                    1400    // Total range of motion
#define NECK_SPEED_SLOW_MAX           4000    // Slow movement max speed
#define NECK_SPEED_SLOW_ACCEL         500     // Slow movement acceleration
#define NECK_SPEED_FAST_MAX           6000    // Fast movement max speed
#define NECK_SPEED_FAST_ACCEL         4000    // Fast movement acceleration
#define NECK_RANGE_SCOLD_PERCENT      20      // Percent of range to move during scold (+/-)

// ============================================================================
// PIN DEFINITIONS (default CC5x12 v1.0-v1.1 stepper1, servo1, led1, sensor1)
// ============================================================================
#define PIN_DFPLAYER_TX               0
#define PIN_DFPLAYER_RX               1
#define PIN_SERVO                     29      // SRV1 (2 on CC5x12 <= v1.1)
#define PIN_STEPPER_1                 5       // STEPPER1
#define PIN_STEPPER_2                 6
#define PIN_STEPPER_3                 7
#define PIN_STEPPER_4                 8
#define PIN_LED_EYES                  14      // LED1
#define PIN_MOTION_SENSOR             15      // SNSR1
#define PIN_NEOPIXEL_POWER            11

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

// Beak animation state
bool animatingBeak = false;
BeakKeyframe* currentAnimation = nullptr;
uint8_t currentKeyframe = 0;
uint8_t totalKeyframes = 0;
unsigned long animationStartTime = 0;

// ============================================================================
// SETUP - CORE 0
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(5000);

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
  } else if (SENSOR_MODE == SENSOR_MODE_PIR ) {
    Serial.println(F("*** PIR SENSOR MODE ***"));
  } else {
    Serial.println(F("*** NO SENSOR CONFIGURED ***"));
  }
  Serial.println(F("========================================\n"));

  // Initialize status LED (conditional)
  #if SHOW_NEOPIXEL_STATUS
  pinMode(PIN_NEOPIXEL_POWER, OUTPUT);
  digitalWrite(PIN_NEOPIXEL_POWER, HIGH);
  statusLED.begin();
  statusLED.setBrightness(50);
  statusLED.setPixelColor(0, statusLED.Color(0, 50, 0)); // Green - starting
  statusLED.show();
  #endif

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

  #if SHOW_NEOPIXEL_STATUS
  statusLED.setPixelColor(0, statusLED.Color(0, 0, 0)); // Off
  statusLED.show();
  #endif

  Serial.println(F("\n✓ Initialization complete. Crow is alive!\n"));
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
    delay(1000); // Just sleep forever
    return;
  }
  sensorCurrentlyHigh = digitalRead(PIN_MOTION_SENSOR);
  delay(50); // Debounce
}

// ============================================================================
// MAIN LOOP - CORE 0
// ============================================================================
void loop() {
  unsigned long now = millis();

  // Always run stepper (non-blocking)
  stepper.run();

  // Always update beak animation if active
  updateBeakAnimation();

  // Handle eye behavior based on mode
  if (TEST_MODE) { // Test mode: eyes mirror sensor state
    digitalWrite(PIN_LED_EYES, sensorCurrentlyHigh ? HIGH : LOW);
  } else { // Normal mode: random blinking
    handleBlinking(now);
  }

  // LD1020 Mode: Check if cooldown period has elapsed
  bool ld1020Clear = true;
  if (SENSOR_MODE == SENSOR_MODE_LD1020) {
    long moveTime = max(movementEnd, movementStart);
    ld1020Clear = (now - moveTime >= LD1020_ANIMATION_COOLDOWN_MS);
    if (!ld1020Clear) return; // Skip other behaviors during cooldown
  }

  // Prevent rapidly-repeating squawks and scolds
  bool squawkEnabled = (now - lastAudioTime >= SCOLD_SQUAWK_BLOCK_MS);

  // Scold when triggered and available
  if (currentMode != MODE_SCOLDING && sensorCurrentlyHigh && !animatingBeak && squawkEnabled && ld1020Clear) {
    Serial.println(F("[Scold]  Motion detected! Scolding..."));
    // Interrupt idle neck movement if in progress
    if (currentMode == MODE_IDLE && stepper.distanceToGo() != 0) {
      Serial.println(F("[Break]  Stopping idle movement for scold"));
      stepper.stop();
    }
    startScoldSequence();
    return; // Skip other behaviors this loop
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
      if (!animatingBeak && stepper.distanceToGo() == 0) {
        // 5% chance to just keep scolding
        if (random(0, 20) == 0) {
          Serial.println(F("[Scold]  Double Down!"));
          startScoldSequence();
          break;
        }
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
      if (!animatingBeak && stepper.distanceToGo() == 0) {
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
  beakServo.attach(PIN_SERVO, SERVO_PWM_MIN, SERVO_PWM_MAX);
  closeBeak();
  delay(300);
  openBeak();
  delay(300);
  closeBeak();
  Serial.println(F("[Init]   Beak servo online"));
  delay(500);
}

void initializeNeck() {
  setNeckSpeedSlow();

  // Center the neck through a calibration sequence
  Serial.println(F("[Init]   Centering neck..."));
  stepper.setCurrentPosition(0);
  stepper.moveTo(NECK_RANGE + 100);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  stepper.moveTo(-(NECK_RANGE/2+50));
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  Serial.println(F("[Init]   Neck centered and online"));

  setNeckSpeedFast();
  delay(500);
}

void initializeDFPlayer() {
  #if SHOW_NEOPIXEL_STATUS
  statusLED.setPixelColor(0, statusLED.Color(50, 25, 0)); // Orange
  statusLED.show();
  #endif

  Serial1.setTX(PIN_DFPLAYER_TX);
  Serial1.setRX(PIN_DFPLAYER_RX);
  Serial1.begin(9600);
  delay(1000);

  if (!dfPlayer.begin(Serial1, true, true)) {
    Serial.println(F("[Init]   ✗ DFPlayer Mini failed!"));
    #if SHOW_NEOPIXEL_STATUS
    statusLED.setPixelColor(0, statusLED.Color(50, 0, 0)); // Red
    statusLED.show();
    #endif
    delay(2000);
  } else {
    Serial.println(F("[Init]   DFPlayer Mini online"));
    #if SHOW_NEOPIXEL_STATUS
    statusLED.setPixelColor(0, statusLED.Color(50, 0, 50)); // Purple
    statusLED.show();
    #endif

    dfPlayer.volume(DFPLAYER_VOLUME);
    delay(100);
  }
  delay(500);
}

void initializeMotionSensor() {
  pinMode(PIN_MOTION_SENSOR, INPUT);
  #if SHOW_NEOPIXEL_STATUS
  statusLED.setPixelColor(0, statusLED.Color(0, 0, 50)); // Blue
  statusLED.show();
  #endif

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

  delay(500);
}

// ============================================================================
// MODE HANDLERS
// ============================================================================

void handleIdleMode(unsigned long now, bool squawkEnabled, bool ld1020Clear) {

  // Random neck movements
  if (ld1020Clear && now >= nextIdleMoveTime && stepper.distanceToGo() == 0 && !animatingBeak) {

    // Randomly scold if there is no sensor
    if (SENSOR_MODE == SENSOR_MODE_NONE) {
      if (squawkEnabled && random(0, 4) == 0) {
        Serial.println(F("[Scold]  Idle scold! Scolding..."));
        startScoldSequence();
        resetIdleMoveTime();
        return;
      }
    }

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

  // Random idle squawks
  if (ld1020Clear && squawkEnabled && now >= nextIdleSquawkTime && !animatingBeak) {
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
// BEAK CONTROL
// ============================================================================

void openBeak() {
  beakServo.write(BEAK_OPEN_DEG);
}

void closeBeak() {
  beakServo.write(BEAK_CLOSED_DEG);
}

void setBeakPosition(uint8_t percent) {
  // Convert 0-100% to servo angle
  beakServo.write(map(percent, 0, 100, BEAK_CLOSED_DEG, BEAK_OPEN_DEG));
}

void startBeakAnimation(BeakKeyframe* animation, uint8_t numFrames) {
  currentAnimation = animation;
  totalKeyframes = numFrames;
  currentKeyframe = 0;
  animationStartTime = millis();
  animatingBeak = true;

  // Set initial position
  setBeakPosition(animation[0].position);
}

void updateBeakAnimation() {
  if (!animatingBeak || currentAnimation == nullptr) return;

  unsigned long elapsed = millis() - animationStartTime;

  // Check if we need to advance to next keyframe
  if (currentKeyframe < totalKeyframes) {
    if (elapsed >= currentAnimation[currentKeyframe].timeMs) {
      setBeakPosition(currentAnimation[currentKeyframe].position);
      currentKeyframe++;

      // Check if animation is complete
      if (currentKeyframe >= totalKeyframes) {
        animatingBeak = false;
        currentAnimation = nullptr;
      }
    }
  }
}

// ============================================================================
// AUDIO CONTROL
// ============================================================================

void animateAudio(uint8_t trackNum) {
  Serial.print(F("► Audio  Playing track "));
  Serial.print(trackNum);
  Serial.print(F(" at time "));
  Serial.println(millis()/1000);

  dfPlayer.play(trackNum);

  // Find and start corresponding animation
  for (int i = 0; i < sizeof(soundAnimations) / sizeof(SoundAnimation); i++) {
    if (soundAnimations[i].trackNum == trackNum) {
      startBeakAnimation(soundAnimations[i].animation, soundAnimations[i].numKeyframes);
      break;
    }
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
