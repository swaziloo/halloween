// ============================================================================
// ANIMATION DEFINITIONS
// ============================================================================
#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>

struct AnimKeyFrame {
  uint16_t timeMs;
  uint8_t position;
};

struct SoundAnimation {
  uint8_t trackNum;
  const AnimKeyFrame* animation;
  uint8_t numKeyframes;
};

const AnimKeyFrame anim_Scold1[] PROGMEM = {{0,0},{60,90},{330,60},{440,75},{700,60},{800,75},{1050,60},{1230,75},{1460,50},{1700,85},{1950,0}};
const AnimKeyFrame anim_Scold2[] PROGMEM = {{0,0},{300,5},{520,80},{620,95},{1100,40},{1700,40},{1800,95},{2300,80},{2400,80},{2550,0}};
const AnimKeyFrame anim_Scold3[] PROGMEM = {{0,0},{90,95},{500,65},{600,95},{1050,65},{1150,95},{1550,90},{1625,50},{1750,0}};
const AnimKeyFrame anim_Scold4[] PROGMEM = {{0,0},{225,95},{540,55},{725,95},{1054,55},{1300,95},{1650,55},{1950,95},{2300,55},{2650,95},{2800,90},{2970,40},{3020,0}};
const AnimKeyFrame anim_Scold5[] PROGMEM = {{0,0},{200,90},{320,35},{470,90},{600,35},{780,80},{900,35},{1110,80},{1230,40},{1500,90},{1620,35},{2060,80},{2185,40},{2560,80},{2670,40},{3200,60},{3320,0}};
const AnimKeyFrame anim_Scold6[] PROGMEM = {{0,0},{380,5},{480,70},{730,55},{975,70},{1180,45},{1620,40},{1720,80},{1980,60},{2180,85},{2460,60},{2660,80},{2930,50},{3190,70},{3340,65},{3440,0}};
const AnimKeyFrame anim_Scold7[] PROGMEM = {{0,0},{90,90},{230,60},{440,80},{580,60},{870,80},{1030,60},{1370,80},{1520,60},{1930,80},{2060,50},{2770,80},{2910,60},{3400,90},{3600,95},{3700,50},{3950,0}};
const AnimKeyFrame anim_Idle1[]  PROGMEM = {{0,0},{193,30},{480,80},{730,15},{1130,30},{1470,80},{1700,30},{1820,0}};
const AnimKeyFrame anim_Idle2[]  PROGMEM = {{0,0},{240,80},{420,50},{500,80},{650,50},{740,80},{880,50},{970,80},{1110,50},{1200,80},{1340,50},{1440,80},{1520,30},{1600,0}};
const AnimKeyFrame anim_Idle3[]  PROGMEM = {{0,0},{350,60},{875,30},{1130,55},{1465,35},{1600,50},{1900,45},{2000,55},{2250,0}};
const AnimKeyFrame anim_Idle4[]  PROGMEM = {{0,0},{150,35},{290,45},{510,75},{680,45},{970,65},{1150,0}};
const AnimKeyFrame anim_Idle5[]  PROGMEM = {{0,0},{210,80},{350,40},{700,80},{870,50},{1190,90},{1360,50},{1730,90},{1900,40},{2330,80},{2510,45},{3860,45},{3960,75},{4210,60},{4360,74},{4500,60},{4650,75},{4790,60},{5050,0}};
const AnimKeyFrame anim_Idle6[]  PROGMEM = {{0,0},{137,70},{200,40},{500,10},{700,90},{840,45},{1000,0}};
const AnimKeyFrame anim_Idle7[]  PROGMEM = {{0,0},{350,10},{425,70},{700,30},{1280,35},{1380,70},{1690,30},{2970,35},{3070,70},{3400,30},{4240,35},{4340,65},{4610,60},{4710,30},{4780,0}};

const SoundAnimation soundAnimations[] PROGMEM = {
  {1,  anim_Scold1, sizeof(anim_Scold1) / sizeof(AnimKeyFrame)},
  {2,  anim_Scold2, sizeof(anim_Scold2) / sizeof(AnimKeyFrame)},
  {3,  anim_Scold3, sizeof(anim_Scold3) / sizeof(AnimKeyFrame)},
  {4,  anim_Scold4, sizeof(anim_Scold4) / sizeof(AnimKeyFrame)},
  {5,  anim_Scold5, sizeof(anim_Scold5) / sizeof(AnimKeyFrame)},
  {6,  anim_Scold6, sizeof(anim_Scold6) / sizeof(AnimKeyFrame)},
  {7,  anim_Scold7, sizeof(anim_Scold7) / sizeof(AnimKeyFrame)},
  {8,  anim_Idle1,  sizeof(anim_Idle1)  / sizeof(AnimKeyFrame)},
  {9,  anim_Idle2,  sizeof(anim_Idle2)  / sizeof(AnimKeyFrame)},
  {10, anim_Idle3,  sizeof(anim_Idle3)  / sizeof(AnimKeyFrame)},
  {11, anim_Idle4,  sizeof(anim_Idle4)  / sizeof(AnimKeyFrame)},
  {12, anim_Idle5,  sizeof(anim_Idle5)  / sizeof(AnimKeyFrame)},
  {13, anim_Idle6,  sizeof(anim_Idle6)  / sizeof(AnimKeyFrame)},
  {14, anim_Idle7,  sizeof(anim_Idle7)  / sizeof(AnimKeyFrame)}
}; 
const uint8_t NUM_ANIMATIONS = sizeof(soundAnimations) / sizeof(SoundAnimation);

// Animation State
static const AnimKeyFrame* currentAnimation = nullptr;
static uint8_t totalKeyframes = 0;
static uint8_t currentKeyframe = 0;
static unsigned long animationStartTime = 0;
static volatile bool animating = false;

// Pending State (for the Audio Sync delay)
static const AnimKeyFrame* pendingAnimation = nullptr;
static uint8_t pendingTotalFrames = 0;
static unsigned long pendingAnimationStartTime = 0;

// Easing Lookup Table
static uint16_t easingLUT[101];

void hydrateEasingLUT(int openLimit, int closedLimit, float p) {
  for (int i = 0; i <= 100; i++) {
    float x = (float)i / 100.0;
    float eased;
    if (x < 0.5) eased = 0.5 * pow(2 * x, p);
    else eased = 1.0 - 0.5 * pow(2 * (1.0 - x), p);
    easingLUT[i] = closedLimit + (eased * (openLimit - closedLimit));
  }
}

inline void queuePendingAnimation(int idx, unsigned long startTime) {
    pendingAnimation = (const AnimKeyFrame*)pgm_read_ptr(&(soundAnimations[idx].animation));
    pendingTotalFrames = pgm_read_byte(&(soundAnimations[idx].numKeyframes));
    pendingAnimationStartTime = startTime;
}

// provides the current 0-100% closed-open position for the animation
// or -1 if the animation is not active 
inline int getAnimPos() {
  if (easingLUT[100] == 0) hydrateEasingLUT(SERVO_PWM_OPEN, SERVO_PWM_CLOSED, SERVO_EASING_FACTOR);

  unsigned long now = millis();

  if (pendingAnimation != nullptr) {
    if (now >= pendingAnimationStartTime) { // enable after sync delay
      currentAnimation = pendingAnimation;
      totalKeyframes = pendingTotalFrames;
      currentKeyframe = 0;
      animationStartTime = now;
      animating = true;      // Now the flag is set!
      pendingAnimation = nullptr;
    }  else return -1; // still waiting for sync
  }

  if (!animating) return -1;

  // animate
  unsigned long elapsed = now - animationStartTime;

  while (currentKeyframe < totalKeyframes - 1) {
    uint16_t nextTime = pgm_read_word(&(currentAnimation[currentKeyframe + 1].timeMs));
    if (elapsed >= nextTime) currentKeyframe++;
    else break;
  }

  if (currentKeyframe >= totalKeyframes - 1) {
    animating = false;
    return pgm_read_byte(&(currentAnimation[totalKeyframes - 1].position));
  }

  uint16_t t0 = pgm_read_word(&(currentAnimation[currentKeyframe].timeMs));
  uint16_t t1 = pgm_read_word(&(currentAnimation[currentKeyframe + 1].timeMs));
  uint8_t p0 = pgm_read_byte(&(currentAnimation[currentKeyframe].position));
  uint8_t p1 = pgm_read_byte(&(currentAnimation[currentKeyframe + 1].position));
  
  unsigned long sd = t1 - t0;
  float t = (sd > 0) ? (float)(elapsed - t0) / (float)sd: 1.0f;
  return p0 + t * (p1 - p0);
}

// provides the current, eased PWM position-in-range for the animation
// or -1 if the animation is not active
inline int getEasedAnimPWM() {
  int p = getAnimPos();
  if (p == -1) return -1;
  p = constrain(p, 0, 100);
  return easingLUT[p];
}

#endif