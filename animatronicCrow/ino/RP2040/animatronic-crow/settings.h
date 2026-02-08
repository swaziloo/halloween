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
#define PIN_SERVO                     29    // SRV1 (2 on CC5x12 <= v1.1)
#define PIN_STEPPER_1                 5     // STEPPER1
#define PIN_STEPPER_2                 6
#define PIN_STEPPER_3                 7
#define PIN_STEPPER_4                 8
#define PIN_LED_EYES                  14    // LED1
#define PIN_MOTION_SENSOR             15    // SNSR1
#define PIN_NEOPIXEL_POWER            11

// TEST MODE - Set to true to mirror sensor state with eyes (for debugging)
#define TEST_MODE                     false // true: eyes mirror sensor, false: normal blinking
#define SHOW_NEOPIXEL_STATUS          false // true: display status color on RP2040-Zero RGB LED

// SENSOR MODE - Choose one mode: SENSOR_MODE_PIR, SENSOR_MODE_LD1020, SENSOR_MODE_BUTTON, SENSOR_MODE_NONE
#define SENSOR_MODE                   SENSOR_MODE_PIR

/**
 * Setting the beak range changed with software in Feb 2026.
 * Old defaults were PWM 1000/2000 with a 0-45 or 50 range.
 * If you wish to mimic those settings, set:
 * SERVO_PWM_OPEN   1000
 * SERVO_PWM_CLOSED 1250
 */
// Servo Settings
#define SERVO_PWM_OPEN                1050  // fully open PWM
#define SERVO_PWM_CLOSED              1250  // fully closed PWM
#define SERVO_EASING_FACTOR           3.00  // determines animation smooting (smaller is smoother)

// Audio Settings
#define DFPLAYER_VOLUME               25    // Volume 0-30
#define AUDIO_SYNC_DELAY_MS           100   // sync delay

// Motion Detection Settings
#define LD1020_ANIMATION_COOLDOWN_MS  8500  // Wait after any animation before checking sensor (LD1020 mode only)
#define SCOLD_SQUAWK_BLOCK_MS         7000  // Delay squawks after scolds and vice-versa

// Idle Behavior Settings
// NOTE: For LD1020 mode, ensure these are > LD1020_ANIMATION_COOLDOWN_MS to prevent self-triggering from animations
// NOTE: For BUTTON mode, most of these timers are ignored
#define IDLE_MOVE_MIN_MS              9000  // Min time between idle movements (must be > LD1020_ANIMATION_COOLDOWN_MS for LD1020)
#define IDLE_MOVE_MAX_MS              18000 // Max time between idle movements
#define IDLE_SQUAWK_MIN_MS            15000 // Min time between random squawks (or scolds without sensor)
#define IDLE_SQUAWK_MAX_MS            45000 // Max time between random squawks (or scolds without sensor)
#define IDLE_NECK_MIN_PERCENT         30    // Min percent of range for idle moves
#define IDLE_NECK_MAX_PERCENT         100   // Max percent of range for idle moves

// Eye Blink Settings
#define BLINK_DURATION_MS             90    // How long eyes stay closed
#define BLINK_MIN_INTERVAL_MS         3000  // Min time between blinks
#define BLINK_MAX_INTERVAL_MS         8000  // Max time between blinks

// Neck Movement Settings
#define NECK_RANGE                    1400  // Total range of motion
#define NECK_SPEED_SLOW_MAX           3250  // Slow movement max speed
#define NECK_SPEED_SLOW_ACCEL         500   // Slow movement acceleration
#define NECK_SPEED_FAST_MAX           6000  // Fast movement max speed
#define NECK_SPEED_FAST_ACCEL         4000  // Fast movement acceleration
#define NECK_RANGE_SCOLD_PERCENT        20  // Percent of range to move during scold (+/-)

#endif
