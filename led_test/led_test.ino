#include <Adafruit_NeoPixel.h>

#define N_SCALES 6

const int ledPins [] = { A10, A11, A12, A13, A14, A15 };

const int n_leds = 5;

Adafruit_NeoPixel leds [] = { Adafruit_NeoPixel(n_leds, ledPins[0], NEO_GRB + NEO_KHZ800), 
                              Adafruit_NeoPixel(n_leds, ledPins[1], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[2], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[3], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[4], NEO_GRB + NEO_KHZ800),
                              Adafruit_NeoPixel(n_leds, ledPins[5], NEO_GRB + NEO_KHZ800) };


void setup() {
  Serial.begin(57600);
  for (int i=0; i < N_SCALES; i++) {
    leds[i].begin();
    leds[i].show(); // Initialize all pixels to 'off'
  }

}

void loop() {
  Serial.println(analogRead(A5));
  color_loop(leds[0].Color(255, 0, 0), 100);
  color_loop(leds[0].Color(0, 255, 0), 100);
  color_loop(leds[0].Color(0, 0, 255), 100);
  
  delay(10);
}



void color_loop(uint32_t color, int wait) {
  
  for (int i=0; i < N_SCALES; i++) {
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, color);
    }
    leds[i].show();
    delay(wait);
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, 0, 0, 0);
    }
    leds[i].show();
  }
  for (int i=N_SCALES-1; i >= 0; i--) {
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, color);
    }
    leds[i].show();
    delay(wait);
    for(int j = 0; j < n_leds; j++) {
      leds[i].setPixelColor(j, 0, 0, 0);
    }
    leds[i].show();
  }
}
