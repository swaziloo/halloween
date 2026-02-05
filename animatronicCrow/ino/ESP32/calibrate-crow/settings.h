#ifndef SETTINGS_H
#define SETTINGS_H
// ****************************************************************************
// Animatronic Crow User Configuration
// ****************************************************************************

// ============================================================================
// PIN DEFINITIONS - (defaults for CC5x12 v1.2 stepper1, servo1, led1, sensor1)
// ============================================================================
#define PIN_DFPLAYER_TX               TX
#define PIN_DFPLAYER_RX               RX
#define PIN_SERVO                     13    // SRV1 (2 on CC5x12 <= v1.1)
#define PIN_STEPPER_1                 10    // STEPPER1
#define PIN_STEPPER_2                 9
#define PIN_STEPPER_3                 8
#define PIN_STEPPER_4                 7
#define PIN_LED_EYES                  6    // LED1
#define PIN_MOTION_SENSOR             5    // SNSR1
#define PIN_NEOPIXEL_POWER            21

// Servo Settings
#define SERVO_PWM_OPEN                1100 // default fully open PWM
#define SERVO_PWM_CLOSED              1250 // default fully closed PWM
#define SERVO_EASING_FACTOR           3.00 // determines animation smooting (smaller is smoother)
#define SERVO_PWM_MIN                 1000 // safer min for calibration
#define SERVO_PWM_MAX                 1500 // safer max for calibration

// Audio Settings
#define DFPLAYER_VOLUME               22  	// default Volume 0-30
#define AUDIO_SYNC_DELAY_MS	          100   // sync delay

// Neck Movement Settings
#define NECK_RANGE                    1400  // Total range of motion

#endif