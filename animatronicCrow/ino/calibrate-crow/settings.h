#ifndef SETTINGS_H
#define SETTINGS_H
// ****************************************************************************
// Animatronic Crow User Configuration
// ****************************************************************************

// ============================================================================
// PIN DEFINITIONS - (defaults for CC5x12 v1.2 stepper1, servo1, led1, sensor1)
// ============================================================================
#define PIN_DFPLAYER_TX               0
#define PIN_DFPLAYER_RX               1
#define PIN_SERVO                     2     // SRV1 (2 on CC5x12 <= v1.1)
#define PIN_STEPPER_1                 5     // STEPPER1
#define PIN_STEPPER_2                 6
#define PIN_STEPPER_3                 7
#define PIN_STEPPER_4                 8
#define PIN_LED_EYES                  14    // LED1
#define PIN_MOTION_SENSOR             15    // SNSR1
#define PIN_NEOPIXEL_POWER            11

#define SHOW_NEOPIXEL_STATUS          false // true: display status color on RP2040-Zero RGB LED

// SENSOR MODE - Choose one mode: SENSOR_MODE_PIR, SENSOR_MODE_LD1020, SENSOR_MODE_BUTTON, SENSOR_MODE_NONE
#define SENSOR_MODE                   SENSOR_MODE_PIR

// Servo Settings
#define SERVO_PWM_OPEN                1200 // default fully open PWM
#define SERVO_PWM_CLOSED              1250 // default fully closed PWM
#define SERVO_EASING_FACTOR           3.0  // determines animation smooting (smaller is smoother)

// Audio Settings
#define DFPLAYER_VOLUME				  22  	// default Volume 0-30
#define AUDIO_SYNC_DELAY_MS	          65   // sync delay

// Neck Movement Settings
#define NECK_RANGE                    1400  // Total range of motion

#endif