/*
 * Beak Animation & Audio Tester
 */
#include <Arduino.h>
#include <Servo.h>
#include <DFRobotDFPlayerMini.h>
#include "animations.h"

// ============================================================================
// CONFIGURATION
// ============================================================================
#define SERVO_PWM_MIN           544     // default 544: adjust to actual
#define SERVO_PWM_MAX           2400    // default 2400: adjust to actual
#define BEAK_OPEN_DEG           0       // Fully open position
#define BEAK_CLOSED_DEG         55      // Fully closed position
#define DFPLAYER_VOLUME         15      // Volume 0-30 (default)
#define PAUSE_BETWEEN_TRACKS_MS 3000    // Pause between animations

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define PIN_DFPLAYER_TX         0
#define PIN_DFPLAYER_RX         1
#define PIN_SERVO               2       // SRV1 (29 on CC5x12 v1.2)
#define PIN_LED_EYES            14      // LED1

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
Servo beakServo;
DFRobotDFPlayerMini dfPlayer;

// Animation state
bool animatingBeak = false;
BeakKeyframe* currentAnimation = nullptr;
uint8_t currentKeyframe = 0;
uint8_t totalKeyframes = 0;
unsigned long animationStartTime = 0;
int currentTrack = 0;
uint8_t currentVolume = DFPLAYER_VOLUME;

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(5000);
  
  Serial.println(F("\n========================================"));
  Serial.println(F("Beak Animation & Audio Tester"));
  Serial.println(F("========================================\n"));
  
  // Initialize servo
  beakServo.attach(PIN_SERVO, SERVO_PWM_MIN, SERVO_PWM_MAX);
  closeBeak();
  delay(500);
  Serial.println(F("[Init] Beak servo online"));
  
  // Initialize DFPlayer
  Serial1.setTX(PIN_DFPLAYER_TX);
  Serial1.setRX(PIN_DFPLAYER_RX);
  Serial1.begin(9600);
  delay(1000);
  
  if (!dfPlayer.begin(Serial1, true, true)) {
    Serial.println(F("[Init] ✗ DFPlayer Mini failed!"));
    Serial.println(F("Check connections and SD card"));
    while(1) delay(1000);
  }
  
  Serial.println(F("[Init] DFPlayer Mini online"));
  dfPlayer.volume(currentVolume);
  delay(100);
  
  Serial.println(F("\n========================================"));
  Serial.println(F("Ready to test animations"));
  Serial.println(F("========================================"));
  Serial.print(F("\nEnter track number (1-"));
  Serial.print(NUM_ANIMATIONS);
  Serial.println(F(") to play:"));
  Serial.println(F("Type VOL0-VOL30 to change volume"));
  Serial.println(F("Type 'list' or '?' for track list\n"));

  pinMode(PIN_LED_EYES, OUTPUT);
  digitalWrite(PIN_LED_EYES, HIGH);
  
  printTrackList();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // Check for serial input
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase(); // For VOL command
    
    // Check for volume command
    if (input.startsWith("VOL")) {
      String volStr = input.substring(3);
      int vol = volStr.toInt();
      
      if (vol >= 0 && vol <= 30) {
        currentVolume = vol;
        dfPlayer.volume(currentVolume);
        Serial.print(F("Volume set to: "));
        Serial.println(currentVolume);
      } else {
        Serial.println(F("Invalid volume. Use VOL0 to VOL30"));
      }
    }
    // Check for track number
    else {
      int trackNum = input.toInt();
      
      if (trackNum >= 1 && trackNum <= NUM_ANIMATIONS) {
        if (!animatingBeak) {
          playTrack(trackNum);
        } else {
          Serial.println(F("Animation in progress, please wait..."));
        }
      } else if (input.equals("LIST") || input == "?") {
        printTrackList();
      } else if (input.length() > 0) {
        Serial.print(F("Invalid command: "));
        Serial.println(input);
        Serial.print(F("Enter 1-"));
        Serial.print(NUM_ANIMATIONS);
        Serial.println(F(", VOL0-VOL30, or 'list'"));
      }
    }
  }
  
  // Update animation
  updateBeakAnimation();
  
  // Check if animation complete
  static bool wasAnimating = false;
  if (wasAnimating && !animatingBeak) {
    Serial.println(F("Animation complete\n"));
    Serial.print(F("Enter track number (1-"));
    Serial.print(NUM_ANIMATIONS);
    Serial.print(F(") [Volume: "));
    Serial.print(currentVolume);
    Serial.println(F("]:"));
    closeBeak();
  }
  wasAnimating = animatingBeak;
}

// ============================================================================
// ANIMATION CONTROL
// ============================================================================
void playTrack(int trackNum) {
  // Find animation by track number
  for (int i = 0; i < NUM_ANIMATIONS; i++) {
    if (soundAnimations[i].trackNum == trackNum) {
      SoundAnimation &anim = soundAnimations[i];
      
      Serial.println(F("\n----------------------------------------"));
      Serial.print(F("Playing Track "));
      Serial.print(anim.trackNum);
      Serial.print(F(": "));
      Serial.println(anim.name);
      Serial.print(F("Keyframes: "));
      Serial.print(anim.numKeyframes);
      Serial.print(F(" | Volume: "));
      Serial.println(currentVolume);
      Serial.println(F("----------------------------------------\n"));
      
      dfPlayer.play(anim.trackNum);
      delay(100);
      
      startBeakAnimation(anim.animation, anim.numKeyframes);
      return;
    }
  }
}

void printTrackList() {
  Serial.println(F("Available tracks:"));
  Serial.println(F("----------------------------------------"));
  for (int i = 0; i < NUM_ANIMATIONS; i++) {
    Serial.print(F("  "));
    Serial.print(soundAnimations[i].trackNum);
    Serial.print(F(". "));
    Serial.println(soundAnimations[i].name);
  }
  Serial.println(F("----------------------------------------"));
  Serial.print(F("Current volume: "));
  Serial.println(currentVolume);
  Serial.println(F("\nCommands:"));
  Serial.print(F("  1-"));
  Serial.print(NUM_ANIMATIONS);
  Serial.println(F("      - Play track"));
  Serial.println(F("  VOL0-30   - Set volume (e.g. VOL15)"));
  Serial.println(F("  LIST or ? - Show this list\n"));
}

void startBeakAnimation(BeakKeyframe* animation, uint8_t numFrames) {
  currentAnimation = animation;
  totalKeyframes = numFrames;
  currentKeyframe = 0;
  animationStartTime = millis();
  animatingBeak = true;
  
  setBeakPosition(animation[0].position);
}

void updateBeakAnimation() {
  if (!animatingBeak || currentAnimation == nullptr) return;
  
  unsigned long elapsed = millis() - animationStartTime;
  
  if (currentKeyframe < totalKeyframes) {
    if (elapsed >= currentAnimation[currentKeyframe].timeMs) {
      setBeakPosition(currentAnimation[currentKeyframe].position);
      currentKeyframe++;
      
      if (currentKeyframe >= totalKeyframes) {
        animatingBeak = false;
      }
    }
  }
}

// ============================================================================
// BEAK CONTROL
// ============================================================================
void setBeakPosition(uint8_t percent) {
  beakServo.write(map(percent, 0, 100, BEAK_CLOSED_DEG, BEAK_OPEN_DEG));
}

void closeBeak() {
  beakServo.write(BEAK_CLOSED_DEG);
}

void openBeak() {
  beakServo.write(BEAK_OPEN_DEG);
}