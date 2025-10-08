// ============================================================================
// BEAK ANIMATION DEFINITIONS
// ============================================================================
#ifndef ANIMATIONS_H
#define ANIMATIONS_H

struct BeakKeyframe {
  uint16_t timeMs;
  uint8_t position;
};

BeakKeyframe beakAnim_Scold1[] = {{0,0},{60,95},{330,60},{440,75},{700,60},{800,75},{1050,60},{1230,75},{1460,50},{1700,85},{1950,0}};
BeakKeyframe beakAnim_Scold2[] = {{0,0},{400,95},{1100,40},{1700,95},{2400,0},{2500,0}};
BeakKeyframe beakAnim_Scold3[] = {{0,0},{90,95},{500,65},{600,95},{1050,65},{1150,95},{1650,0}};
BeakKeyframe beakAnim_Scold4[] = {{0,0},{225,95},{540,65},{725,95},{1054,65},{1300,95},{1650,65},{1950,95},{2300,65},{2650,95},{3020,0}};
BeakKeyframe beakAnim_Scold5[] = {{0,0},{200,90},{320,30},{470,90},{600,30},{780,80},{900,30},{1110,80},{1230,35},{1500,90},{1620,35},{2060,80},{2185,35},{2560,80},{2670,35},{3200,60},{3320,0}};
BeakKeyframe beakAnim_Scold6[] = {{0,0},{480,70},{730,55},{975,70},{1180,45},{1720,90},{1980,60},{2180,90},{2460,60},{2660,80},{2930,50},{3190,70},{3440,0}};
BeakKeyframe beakAnim_Scold7[] = {{0,0},{90,90},{230,70},{440,90},{580,70},{870,90},{1030,70},{1370,90},{1520,70},{1930,90},{2060,50},{2770,90},{2910,70},{3400,90},{3800,0}};
BeakKeyframe beakAnim_Idle1[] = {{0,0},{193,30},{480,80},{730,15},{1130,30},{1470,80},{1800,0}};
BeakKeyframe beakAnim_Idle2[] = {{0,0},{240,80},{420,50},{500,80},{650,50},{740,80},{880,50},{970,80},{1110,50},{1200,80},{1340,50},{1440,80},{1600,0}};
BeakKeyframe beakAnim_Idle3[] = {{0,0},{350,60},{875,30},{1130,55},{1465,35},{1600,50},{1900,35},{2150,0}};
BeakKeyframe beakAnim_Idle4[] = {{0,0},{150,35},{290,60},{510,70},{680,60},{970,35},{1150,0}};
BeakKeyframe beakAnim_Idle5[] = {{0,0},{210,80},{350,60},{700,80},{870,60},{1190,90},{1360,70},{1730,90},{1900,60},{2330,80},{2510,30},{3960,60},{4210,40},{4360,60},{4500,40},{4650,60},{4790,40},{5030,0}};
BeakKeyframe beakAnim_Idle6[] = {{0,0},{137,70},{200,40},{500,10},{750,90},{840,45},{1000,0}};
BeakKeyframe beakAnim_Idle7[] = {{0,0},{425,70},{700,30},{1380,70},{1690,30},{3070,70},{3400,30},{4340,50},{4710,0}};

struct SoundAnimation {
  uint8_t trackNum;
  BeakKeyframe* animation;
  uint8_t numKeyframes;
  const char* name;
};

SoundAnimation soundAnimations[] = {
  {1, beakAnim_Scold1, sizeof(beakAnim_Scold1) / sizeof(BeakKeyframe), "Scold 1"},
  {2, beakAnim_Scold2, sizeof(beakAnim_Scold2) / sizeof(BeakKeyframe), "Scold 2"},
  {3, beakAnim_Scold3, sizeof(beakAnim_Scold3) / sizeof(BeakKeyframe), "Scold 3"},
  {4, beakAnim_Scold4, sizeof(beakAnim_Scold4) / sizeof(BeakKeyframe), "Scold 4"},
  {5, beakAnim_Scold5, sizeof(beakAnim_Scold5) / sizeof(BeakKeyframe), "Scold 5"},
  {6, beakAnim_Scold6, sizeof(beakAnim_Scold6) / sizeof(BeakKeyframe), "Scold 6"},
  {7, beakAnim_Scold7, sizeof(beakAnim_Scold7) / sizeof(BeakKeyframe), "Scold 7"},
  {8, beakAnim_Idle1, sizeof(beakAnim_Idle1) / sizeof(BeakKeyframe), "Idle 1"},
  {9, beakAnim_Idle2, sizeof(beakAnim_Idle2) / sizeof(BeakKeyframe), "Idle 2"},
  {10, beakAnim_Idle3, sizeof(beakAnim_Idle3) / sizeof(BeakKeyframe), "Idle 3"},
  {11, beakAnim_Idle4, sizeof(beakAnim_Idle4) / sizeof(BeakKeyframe), "Idle 4"},
  {12, beakAnim_Idle5, sizeof(beakAnim_Idle5) / sizeof(BeakKeyframe), "Idle 5"},
  {13, beakAnim_Idle6, sizeof(beakAnim_Idle6) / sizeof(BeakKeyframe), "Idle 6"},
  {14, beakAnim_Idle7, sizeof(beakAnim_Idle7) / sizeof(BeakKeyframe), "Idle 7"}
};

const int NUM_ANIMATIONS = sizeof(soundAnimations) / sizeof(SoundAnimation);

#endif