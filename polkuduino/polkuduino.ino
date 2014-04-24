#include <Adafruit_NeoPixel.h>
#define N_SCALES 6
#define SIZE_COLORSCALE 4
#define PI 3.1415927

const int n_leds = 5;
const int ledPins [] = { A10, A11, A12, A13, A14, A15 };
const int scalePins [] = { A0, A1, A2, A3, A4, A5 };
const float filter = 0.9;
//const float colorFilter = 0.99;
const int colorSpeed = 3;
const int looptime = 10;

const int calibrated_max[] = { 640, 732, 609, 708, 650, 623 };
const int calibrated_min[] = { 38, 115, 61, 77, 24, 16 };
const float min_scale = 0.15;

const int colorScale [SIZE_COLORSCALE][3] = { { 0, 0, 255 }, { 0, 255, 0 },  { 255, 0, 0 }, { 255, 0, 255 } };
const float colorScaleValues [SIZE_COLORSCALE] = { 0.15, 0.8, 1.2, 1.8 };
const int idleColor [] = {100, 100, 100};
const long stabilizingTime = 500; //milliseconds to wait before scale lits up and pattern begins
const long patternBeginTime = 3000; //milliseconds between scale lit up and pattern start 
const long patternTime = 400; //milliseconds between pattern changes 
const long patternEndTime = 1200; //milliseconds between pattern starts from beginning
const long idlePatternEndTime = 2400; //milliseconds between pattern starts from beginning


Adafruit_NeoPixel leds [] = { 
  Adafruit_NeoPixel(n_leds, ledPins[0], NEO_GRB + NEO_KHZ800), 
  Adafruit_NeoPixel(n_leds, ledPins[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(n_leds, ledPins[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(n_leds, ledPins[3], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(n_leds, ledPins[4], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(n_leds, ledPins[5], NEO_GRB + NEO_KHZ800) };

float readValue [] = { 0, 0, 0, 0, 0, 0 };
int colors [6][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0    }, { 0, 0, 0 }, { 0, 0, 0 } };
int targetColors [6][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0    }, { 0, 0, 0 }, { 0, 0, 0 } };
int scaleStates [] = { 0, 0, 0, 0, 0, 0 }; //0 - Resting, 1 - Weight detected, 2 - lit up, 3 - 9 pattern states, -1 lit up due to pattern
long scaleTimers [] = { 0, 0, 0, 0, 0, 0 };
long idlePatternTimer = 0;
int idle_index = N_SCALES;
float colorRotator = 0.0;
float colorRotatorSpeed = 0.2;
float colorRotatorBase = 0.0;

void setup() {

  Serial.begin(57600);
  Serial.setTimeout(1);

  //analogReference(INTERNAL2V56);

  for (int i=0; i < N_SCALES; i++) {

    leds[i].begin();
    leds[i].show(); // Initialize all pixels to 'off'
  }

}

void loop() {
  long timer = millis();


  //READING SCALE VALUES and SENDING OVER SERIAL
  for(int i=0; i < N_SCALES; i++) {
    analogRead(scalePins[i]);
    delay(1);

    readValue[i] = readValue[i] * filter + analogRead(scalePins[i]) * (1.0 - filter);
    Serial.print(int(readValue[i]));
    if(i != N_SCALES - 1) Serial.print(",");    
  }
  Serial.println();
  
  updatePattern();
  updateIdlePattern();
  /*
  for(int i=0; i < N_SCALES; i++) {
    lightFromWeight(i);
  }*/
  lightsFromSerial();
  //updateLights();
  updateLightsRotate();
    
  //Serial.println(millis()-timer);
  long time = millis() - timer;
  if(time < looptime) {
    delay(looptime-time);
  }
}


float getCalibratedValue(float value, int scale_i) {
  return (value - calibrated_min[scale_i]) / (calibrated_max[scale_i] - calibrated_min[scale_i]);
}

void updateIdlePattern() {
  boolean idle = true;
  for(int i = 0; i < N_SCALES; i++) {
    if(scaleStates[i] > 0) {
      idle = false;
    }
  }
  if(idle && ( (idle_index == N_SCALES && (millis() - idlePatternTimer) > idlePatternEndTime ) || ( idle_index < N_SCALES && ( millis() - idlePatternTimer) > patternTime) ) ) {
    idlePatternTimer = millis();
    if(idle_index < N_SCALES) {
      targetColors[idle_index][0] = 0;
      targetColors[idle_index][1] = 0;
      targetColors[idle_index][2] = 0;
    }
    idle_index = (idle_index + 1) % (N_SCALES + 1);
    if(idle_index < N_SCALES) {
      targetColors[idle_index][0] = idleColor[0];
      targetColors[idle_index][1] = idleColor[0];
      targetColors[idle_index][2] = idleColor[0];
    }
  }
  else if(!idle) {
    if(idle_index != N_SCALES) {
      if(scaleStates[idle_index] == 0) {
        targetColors[idle_index][0] = 0;
        targetColors[idle_index][1] = 0;
        targetColors[idle_index][2] = 0;
      }
      idle_index = N_SCALES;
    }
    idlePatternTimer = millis();
  }
}
//Controlling light patterns
void updatePattern() {
  for (int i=0; i < N_SCALES; i++) {
    if( getCalibratedValue(readValue[i], i) > min_scale ) {
      if( scaleStates[i] <= 0 ) {
        scaleStates[i] = 1;
        scaleTimers[i] = millis();
      }
      else if( scaleStates[i] == 1 && (millis() - scaleTimers[i]) > stabilizingTime ) {
        lightFromWeight(i);
        scaleStates[i] = 2;
        scaleTimers[i] = millis();
      }
      else if( (scaleStates[i] == 2 && (millis() - scaleTimers[i]) > patternBeginTime) || (scaleStates[i] == 3 && (millis() - scaleTimers[i]) > patternEndTime) || (scaleStates[i] > 3 && (millis() - scaleTimers[i]) > patternTime) ) {
        scaleTimers[i] = millis();
        int currentScale = i + scaleStates[i] - 3;
        int nextScale =  currentScale + 1;
        if(scaleStates[currentScale] == -1) {
          scaleStates[currentScale] = 0;
          for(int j = 0; j < 3; j++) {
            targetColors[currentScale][j] = 0;
          }
        }
        if( nextScale < N_SCALES) {
          if( scaleStates[nextScale] <= 0 ) {
            scaleStates[nextScale] = -1;
            for(int j = 0; j < 3; j++) {
              targetColors[nextScale][j] = targetColors[i][j];
            }
          }
          scaleStates[i]++;
        }
        else {
          scaleStates[i] = 3;
        }
      }
    }
    else if(scaleStates[i] > 0) {
      int currentScale = i + scaleStates[i] - 3;
      if(currentScale < N_SCALES && scaleStates[currentScale] == -1) {
        for(int j = 0; j < 3; j++) {
          targetColors[currentScale][j] = 0;
        }
        scaleStates[currentScale] = 0;
      }
      for(int j = 0; j < 3; j++) {
        targetColors[i][j] = 0;
      }
      scaleStates[i] = 0;
    }
  }
}

//updates all lights based on the targetColors and colorspeed
void updateLights() {
  for (int i=0; i < N_SCALES; i++) {
    int diff[3];
    for(int j = 0; j < 3; j++) {
      diff[j] =  targetColors[i][j] - colors[i][j];
    }
    int tot_diff = abs(diff[0]) + abs(diff[1]) + abs(diff[2]);
    if(tot_diff <= colorSpeed) {
      for(int j = 0; j < 3; j++) {
        colors[i][j] = targetColors[i][j];
      }
    }
    else {
      for(int j = 0; j < 3; j++) {
        int cSpeed = colorSpeed * diff[j] / tot_diff;
        colors[i][j] += cSpeed;
      }
    }
    
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, colors[i][0], colors[i][1], colors[i][2]);
    }
    leds[i].show();
  }
}

void updateLightsRotate() {
  float color_x = sin(colorRotator) * (1 - colorRotatorBase);
  float color_y = cos(colorRotator) * (1 - colorRotatorBase);
  float mult[5] = {1, max(color_x, 0) + colorRotatorBase, max(color_y, 0) + colorRotatorBase, -1 * min(color_x, 0) + colorRotatorBase, -1 * min(color_y, 0) + colorRotatorBase };  
  colorRotator += colorRotatorSpeed;
  while(colorRotator > 2*PI) {
    colorRotator -= 2*PI;
  }
  
  for (int i=0; i < N_SCALES; i++) {
    int diff[3];
    for(int j = 0; j < 3; j++) {
      diff[j] =  targetColors[i][j] - colors[i][j];
    }
    int tot_diff = abs(diff[0]) + abs(diff[1]) + abs(diff[2]);
    if(tot_diff <= colorSpeed) {
      for(int j = 0; j < 3; j++) {
        colors[i][j] = targetColors[i][j];
      }
    }
    else {
      for(int j = 0; j < 3; j++) {
        int cSpeed = colorSpeed * diff[j] / tot_diff;
        colors[i][j] += cSpeed;
      }
    }
    
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, mult[j] * float(colors[i][0]), mult[j] * float(colors[i][1]), mult[j] * float(colors[i][2]));
    }
    leds[i].show();
  }
}

void lightsFromSerial() {
  //SETTING COLORS BASED ON RECEIVED SERIAL MESSAGES
   if (Serial.available() > 0) {
     char prefix = Serial.read();
     if(prefix == 'I') {
       int leds_i = Serial.parseInt();
       
       int r = Serial.parseInt();
       int g = Serial.parseInt();
       int b = Serial.parseInt();
       
       targetColors[leds_i][0] = r;
       targetColors[leds_i][1] = g;
       targetColors[leds_i][2] = b;
     }
   }
}


//This sets the color directly from the scale value
void lightFromWeight(int i) {
  //SETTING LED ACCORDING TO SCALE VALUES DIRECTLY
    for(int k = 0; k < SIZE_COLORSCALE - 1; k++) {
      float calibrated = getCalibratedValue(readValue[i], i);
      if(calibrated < colorScaleValues[0]) {
        for(int j = 0; j < 3; j++) {
          targetColors[i][j] = 0;
        }
      }
      else if(calibrated >= colorScaleValues[k] && calibrated < colorScaleValues[k+1]) {
        for(int j = 0; j < 3; j++) {
          float gradient = (calibrated - colorScaleValues[k]) / (colorScaleValues[k+1] - colorScaleValues[k]);
          targetColors[i][j] = (int) (colorScale[k][j] * (1.0 - gradient) + colorScale[k+1][j] * gradient);
        }
      }
    }
}



