#include <Adafruit_NeoPixel.h>
#define N_SCALES 6

const int n_leds = 5;
const int ledPins [] = { A10, A11, A12, A13, A14, A15 };
const int scalePins [] = { A0, A1, A2, A3, A4, A5 };
const float filter = 0.9;
const int looptime = 50;


Adafruit_NeoPixel leds [] = { Adafruit_NeoPixel(n_leds, ledPins[0], NEO_GRB + NEO_KHZ800), 
                              Adafruit_NeoPixel(n_leds, ledPins[1], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[2], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[3], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[4], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[5], NEO_GRB + NEO_KHZ800) };
                              
float readValue [] = { 0, 0, 0, 0, 0, 0 };

void setup() {
  
  Serial.begin(57600);
  
  for (int i=0; i < N_SCALES; i++) {
    pinMode(scalePins[i], INPUT);
    
    leds[i].begin();
    leds[i].show(); // Initialize all pixels to 'off'
  }

}

void loop() {
  long timer = millis();
  
  for(int i=0; i < N_SCALES; i++) {
    readValue[i] = readValue[i] * filter + analogRead(scalePins[i]) * (1.0 - filter);
    Serial.print(int(readValue[i]));
    Serial.print("\t");
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, max(readValue[i]-100, 0)/2, 0, max(readValue[i]-100, 0)/2);
    }
    leds[i].show();
    
  }
  Serial.println();
  /*
  if(Serial.available() > 0) {
    char prefix = Serial.read();
    if(prefix == 'I') {
      int leds_i = Serial.parseInt();
      int r = Serial.parseInt();
      int g = Serial.parseInt();
      int b = Serial.parseInt();
      
      for(int i = 0; i < n_leds; i++) {
        strip.setPixelColor(i, r, g, b);
      }
      strip.show();
    }
  }
  */
  
  long time = millis() - timer;
  if(time < looptime) {
    delay(looptime-time);
  }
}
