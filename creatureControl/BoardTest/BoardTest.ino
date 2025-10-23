/*
 * Creature Control 5x12 Test Suite
 * Uses dual-core processing: Core0 for components, Core1 for sensor monitoring
 * - Cycles through Steppers -> Servos -> MP3
 * - Sensor(s) states are reflected in LED(s)
 */
#include <Arduino.h>
#include <AccelStepper.h>
#include <DFRobotDFPlayerMini.h>
#include <Servo.h>

// ============================================================================
// CONFIGURATION - Enable/Disable Components Here
// ============================================================================
#define ENABLE_NEOPIXEL_STATUS false    // Set false for boards without onboard RGB LED
#define ENABLE_LED1            true    // LED on GP14 triggered by SENSOR1
#define ENABLE_LED2            false   // LED on GP13 triggered by SENSOR2
#define ENABLE_SENSOR1         true    // Sensor on GP15
#define ENABLE_SENSOR2         false   // Sensor on GP26
#define ENABLE_DFPLAYER        true    // DFPlayer Mini on GP0/GP1
#define ENABLE_STEPPER1        true    // Stepper on GP5-GP8
#define ENABLE_STEPPER2        false   // Stepper on GP9-GP12
#define ENABLE_SERVO1          true    // Servo on GP2
#define ENABLE_SERVO2          false   // Servo on GP3
#define ENABLE_SERVO3          false   // Servo on GP4

// Test settings
#define STEPPER_TEST_STEPS     400     // Steps per test cycle
#define MP3_VOLUME             15      // Volume range 0-30
#define MP3_TEST_DELAY         5000    // Milliseconds Delay after playing MP3
#define SERVO_TEST_DELAY       1000    // Milliseconds delay at each position
#define SERVO1_PWM_MIN         1000    // default: adjust to actualS
#define SERVO1_PWM_MAX         2000    // default: adjust to actual
#define SERVO2_PWM_MIN         1000    // default: adjust to actual
#define SERVO2_PWM_MAX         2000    // default: adjust to actual
#define SERVO3_PWM_MIN         1000    // default: adjust to actual
#define SERVO3_PWM_MAX         2000    // default: adjust to actual

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define LED1_PIN               14
#define LED2_PIN               13
#define SENSOR1_PIN            15
#define SENSOR2_PIN            26
#define DFPLAYER_TX            0
#define DFPLAYER_RX            1
#define STEPPER1_PIN1          5
#define STEPPER1_PIN2          6
#define STEPPER1_PIN3          7
#define STEPPER1_PIN4          8
#define STEPPER2_PIN1          9
#define STEPPER2_PIN2          10
#define STEPPER2_PIN3          11
#define STEPPER2_PIN4          12
#define SERVO1_PIN             2
#define SERVO2_PIN             3
#define SERVO3_PIN             4
#define NEOPIXEL_PIN           16      // Onboard RGB LED
#define NEOPIXEL_POWER         11      // Onboard RGB power pin

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
#if ENABLE_NEOPIXEL_STATUS
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel rgbLED(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

DFRobotDFPlayerMini dfPlayer;
AccelStepper stepper1(AccelStepper::HALF4WIRE, STEPPER1_PIN1, STEPPER1_PIN3, STEPPER1_PIN2, STEPPER1_PIN4);
AccelStepper stepper2(AccelStepper::HALF4WIRE, STEPPER2_PIN1, STEPPER2_PIN3, STEPPER2_PIN2, STEPPER2_PIN4);
Servo servo1, servo2, servo3;

// Sensor state tracking (shared between cores)
volatile bool sensor1State = false;
volatile bool sensor2State = false;

// ============================================================================
// RGB LED COLORS (only defined if NeoPixel enabled)
// ============================================================================
#if ENABLE_NEOPIXEL_STATUS
uint32_t COLOR_IDLE = rgbLED.Color(10, 10, 10);      // Dim white
uint32_t COLOR_STEPPER = rgbLED.Color(0, 0, 50);     // Blue
uint32_t COLOR_SERVO = rgbLED.Color(50, 25, 0);      // Orange
uint32_t COLOR_MP3 = rgbLED.Color(50, 0, 50);        // Purple
uint32_t COLOR_ERROR = rgbLED.Color(50, 0, 0);       // Red
#endif

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(5000); // Wait for Serial Monitor to switch on

  Serial.println("\n========================================");
  Serial.println("Creature Control 5x12 Test Suite");
  Serial.println("========================================\n");

  // Initialize onboard RGB LED (conditional)
  #if ENABLE_NEOPIXEL_STATUS
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);
  rgbLED.begin();
  rgbLED.setBrightness(50);
  rgbLED.setPixelColor(0, COLOR_IDLE);
  rgbLED.show();
  Serial.println("[✓] NeoPixel Status LED enabled");
  #else
  Serial.println("[•] NeoPixel Status LED disabled");
  #endif

  // Initialize LEDs
  if (ENABLE_LED1) {
    pinMode(LED1_PIN, OUTPUT);
    digitalWrite(LED1_PIN, LOW);
    Serial.println("[✓] LED1 (GP14) initialized");
  }
  if (ENABLE_LED2) {
    pinMode(LED2_PIN, OUTPUT);
    digitalWrite(LED2_PIN, LOW);
    Serial.println("[✓] LED2 (GP13) initialized");
  }

  // Initialize Sensors (will run on Core1)
  if (ENABLE_SENSOR1) {
    pinMode(SENSOR1_PIN, INPUT);
    Serial.println("[✓] Sensor1 (GP15) initialized");
  }
  if (ENABLE_SENSOR2) {
    pinMode(SENSOR2_PIN, INPUT);
    Serial.println("[✓] Sensor2 (GP26) initialized");
  }

  // Initialize DFPlayer Mini
  if (ENABLE_DFPLAYER) {
    Serial1.setTX(DFPLAYER_TX);
    Serial1.setRX(DFPLAYER_RX);
    Serial1.begin(9600);

    if (dfPlayer.begin(Serial1)) {
      dfPlayer.volume(MP3_VOLUME);
      int trackCount = dfPlayer.readFileCounts();
      Serial.print("[✓] DFPlayer Mini initialized with ");
      Serial.print(trackCount);
      Serial.println(" tracks on the SD card.");
    } else {
      Serial.println("[✗] DFPlayer Mini initialization failed");
      #if ENABLE_NEOPIXEL_STATUS
      rgbLED.setPixelColor(0, COLOR_ERROR);
      rgbLED.show();
      #endif
      if (dfPlayer.available()) printDFPlayerDetail(dfPlayer.readType(), dfPlayer.read());
      delay(1000);
    }
  }

  // Initialize Steppers
  if (ENABLE_STEPPER1) {
    stepper1.setMaxSpeed(4000);
    stepper1.setAcceleration(500);
    stepper1.setSpeed(3000);
    Serial.println("[✓] Stepper1 (GP5-8) initialized");
  }
  if (ENABLE_STEPPER2) {
    stepper2.setMaxSpeed(4000);
    stepper2.setAcceleration(500);
    stepper2.setSpeed(3000);
    Serial.println("[✓] Stepper2 (GP9-12) initialized");
  }

  // Initialize Servos
  if (ENABLE_SERVO1) {
    servo1.attach(SERVO1_PIN, SERVO1_PWM_MIN, SERVO1_PWM_MAX);
    servo1.write(90);
    Serial.println("[✓] Servo1 (GP2) initialized");
  }
  if (ENABLE_SERVO2) {
    servo2.attach(SERVO2_PIN, SERVO2_PWM_MIN, SERVO2_PWM_MAX);
    servo2.write(90);
    Serial.println("[✓] Servo2 (GP3) initialized");
  }
  if (ENABLE_SERVO3) {
    servo3.attach(SERVO3_PIN, SERVO3_PWM_MIN, SERVO3_PWM_MAX);
    servo3.write(90);
    Serial.println("[✓] Servo3 (GP4) initialized");
  }

  Serial.println("\n========================================");
  Serial.println("Starting Test Loop...");
  Serial.println("========================================\n");

  // Launch sensor monitoring on Core1
  if (ENABLE_SENSOR1 || ENABLE_SENSOR2) {
    rp2040.fifo.begin(2);  // Initialize FIFO for core communication
    rp2040.idleOtherCore();
    rp2040.resumeOtherCore();
  }

  delay(3000);
}

// ============================================================================
// CORE1 SETUP - Sensor Monitoring Thread
// ============================================================================
void setup1() {
  // Core1 handles sensor monitoring
}

// ============================================================================
// CORE1 LOOP - Sensor Monitoring Thread
// ============================================================================
void loop1() {
  static bool lastSensor1 = false;
  static bool lastSensor2 = false;

  if (ENABLE_SENSOR1) {
    sensor1State = digitalRead(SENSOR1_PIN);
    if (ENABLE_LED1) {
      digitalWrite(LED1_PIN, sensor1State);
    }

    if (sensor1State != lastSensor1) {
      lastSensor1 = sensor1State;
      // Note: Serial from Core1 can be unreliable, but state is tracked
    }
  }

  if (ENABLE_SENSOR2) {
    sensor2State = digitalRead(SENSOR2_PIN);
    if (ENABLE_LED2) {
      digitalWrite(LED2_PIN, sensor2State);
    }

    if (sensor2State != lastSensor2) {
      lastSensor2 = sensor2State;
    }
  }

  delay(50);  // Debounce delay
}

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

void testSteppers() {
  if (!ENABLE_STEPPER1 && !ENABLE_STEPPER2) return;

  #if ENABLE_NEOPIXEL_STATUS
  rgbLED.setPixelColor(0, COLOR_STEPPER);
  rgbLED.show();
  #endif
  Serial.println("\n--- Testing Steppers ---");

  if (ENABLE_STEPPER1) {
    Serial.print("Stepper1: Moving ");
    Serial.print(STEPPER_TEST_STEPS);
    Serial.println(" steps...");
    stepper1.move(STEPPER_TEST_STEPS);
  }

  if (ENABLE_STEPPER2) {
    Serial.print("Stepper2: Moving ");
    Serial.print(STEPPER_TEST_STEPS);
    Serial.println(" steps...");
    stepper2.move(STEPPER_TEST_STEPS);
  }

  // Run steppers to target
  while ((ENABLE_STEPPER1 && stepper1.distanceToGo() != 0) ||
         (ENABLE_STEPPER2 && stepper2.distanceToGo() != 0)) {
    if (ENABLE_STEPPER1) stepper1.run();
    if (ENABLE_STEPPER2) stepper2.run();
  }

  Serial.println("Steppers completed.");
  delay(500);
}

void testServos() {
  if (!ENABLE_SERVO1 && !ENABLE_SERVO2 && !ENABLE_SERVO3) return;

  #if ENABLE_NEOPIXEL_STATUS
  rgbLED.setPixelColor(0, COLOR_SERVO);
  rgbLED.show();
  #endif
  Serial.println("\n--- Testing Servos ---");

  // Move to 0 degrees
  Serial.println("Moving servos to 0°...");
  if (ENABLE_SERVO1) servo1.write(0);
  if (ENABLE_SERVO2) servo2.write(0);
  if (ENABLE_SERVO3) servo3.write(0);
  delay(SERVO_TEST_DELAY);

  // Move to 180 degrees
  Serial.println("Moving servos to 180°...");
  if (ENABLE_SERVO1) servo1.write(180);
  if (ENABLE_SERVO2) servo2.write(180);
  if (ENABLE_SERVO3) servo3.write(180);
  delay(SERVO_TEST_DELAY);

  // Return to center
  Serial.println("Moving servos to 90° (center)...");
  if (ENABLE_SERVO1) servo1.write(90);
  if (ENABLE_SERVO2) servo2.write(90);
  if (ENABLE_SERVO3) servo3.write(90);
  delay(SERVO_TEST_DELAY);

  Serial.println("Servos completed.");
}

void testMP3() {
  if (!ENABLE_DFPLAYER) return;

  #if ENABLE_NEOPIXEL_STATUS
  rgbLED.setPixelColor(0, COLOR_MP3);
  rgbLED.show();
  #endif
  Serial.println("\n--- Testing DFPlayer Mini ---");
  dfPlayer.next();
  delay(MP3_TEST_DELAY);

  Serial.println("MP3 test completed.");
}

// ============================================================================
// MAIN LOOP - Core0
// ============================================================================
void loop() {
  // Display sensor status from Core1
  static unsigned long lastSensorReport = 0;
  if (millis() - lastSensorReport > 2000) {
    if (ENABLE_SENSOR1 || ENABLE_SENSOR2) {
      Serial.print("[Sensors] ");
      if (ENABLE_SENSOR1) {
        Serial.print("S1:");
        Serial.print(sensor1State ? "HIGH " : "LOW  ");
      }
      if (ENABLE_SENSOR2) {
        Serial.print("S2:");
        Serial.print(sensor2State ? "HIGH " : "LOW  ");
      }
      Serial.println();
    }
    lastSensorReport = millis();
  }

  // Run component tests in sequence
  testSteppers();
  testServos();
  testMP3();

  // Idle period
  #if ENABLE_NEOPIXEL_STATUS
  rgbLED.setPixelColor(0, COLOR_IDLE);
  rgbLED.show();
  #endif
  Serial.println("\n--- Idle ---");
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("Starting next test cycle...");
  Serial.println("========================================");
}

void printDFPlayerDetail(uint8_t type, int value){
  switch (type) {
  case TimeOut:
    Serial.println(F("Time Out!"));
    break;
  case WrongStack:
    Serial.println(F("Stack Wrong!"));
    break;
  case DFPlayerCardInserted:
    Serial.println(F("Card Inserted!"));
    break;
  case DFPlayerCardRemoved:
    Serial.println(F("Card Removed!"));
    break;
  case DFPlayerCardOnline:
    Serial.println(F("Card Online!"));
    break;
  case DFPlayerUSBInserted:
    Serial.println("USB Inserted!");
    break;
  case DFPlayerUSBRemoved:
    Serial.println("USB Removed!");
    break;
  case DFPlayerPlayFinished:
    Serial.print(F("Number:"));
    Serial.print(value);
    Serial.println(F(" Play Finished!"));
    break;
  case DFPlayerError:
    Serial.print(F("DFPlayerError:"));
    switch (value) {
    case Busy:
      Serial.println(F("Card not found"));
      break;
    case Sleeping:
      Serial.println(F("Sleeping"));
      break;
    case SerialWrongStack:
      Serial.println(F("Get Wrong Stack"));
      break;
    case CheckSumNotMatch:
      Serial.println(F("Check Sum Not Match"));
      break;
    case FileIndexOut:
      Serial.println(F("File Index Out of Bound"));
      break;
    case FileMismatch:
      Serial.println(F("Cannot Find File"));
      break;
    case Advertise:
      Serial.println(F("In Advertise"));
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}