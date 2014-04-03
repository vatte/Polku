#include <Adafruit_NeoPixel.h>
#define N_SCALES 6

const int n_leds = 5;
const int ledPins [] = { A10, A11, A12, A13, A14, A15 };
const int scalePins [] = { A0, A1, A2, A3, A4, A5 };
const float filter = 0.0;
//const float colorFilter = 0.97;
const int colorSpeed = 2;
const int looptime = 10;


Adafruit_NeoPixel leds [] = { Adafruit_NeoPixel(n_leds, ledPins[0], NEO_GRB + NEO_KHZ800), 
                              Adafruit_NeoPixel(n_leds, ledPins[1], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[2], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[3], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[4], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[5], NEO_GRB + NEO_KHZ800) };
                              
float readValue [] = { 0, 0, 0, 0, 0, 0 };
int colors [6][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
int targetColors [6][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };

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

  
  
  for(int i=0; i < N_SCALES; i++) {
  //for(int i=1; i < 2; i++) {
    
    /*
    long readTime = millis();
    
    while (millis()-readTime < 1000) {
      analogRead(scalePins[i]);
      delay(10);
    }*/
    analogRead(scalePins[i]);
    delay(10);
    
    readValue[i] = readValue[i] * filter + analogRead(scalePins[i]) * (1.0 - filter);
    Serial.print(int(readValue[i]));
    if(i != N_SCALES - 1) Serial.print(",");
    
    
    for(int j = 0; j < 3; j++) {
      int diff = colors[i][j] - targetColors[i][j];
      if(abs(diff) < colorSpeed) {
        colors[i][j] == targetColors[i][j];
      } else if(diff < 0) {
        colors[i][j] += colorSpeed;
      } else if(diff > 0) {
        colors[i][j] -= colorSpeed;
      }
      //colors[i][j] = colors[i][j] * colorFilter + targetColors[i][j] * (1.0 - colorFilter);
    }
    
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, colors[i][0], colors[i][1], colors[i][2]);
    }
    leds[i].show();
    
  }
  Serial.println();
  
  //Serial.println(millis()-timer);
  long time = millis() - timer;
  if(time < looptime) {
    delay(looptime-time);
  }
}
