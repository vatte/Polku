#include <Adafruit_NeoPixel.h>

const int n_scales = 1;
const int n_leds = 5;
const int ledPins [] = { A1 };
const int scalePins [] = { A0 };

Adafruit_NeoPixel strip = Adafruit_NeoPixel(n_leds, ledPins[0], NEO_GRB + NEO_KHZ800);

float readValue;

void setup() {
  
  Serial.begin(9600);
  pinMode(scalePins[0], INPUT);
  
  //strip = Adafruit_NeoPixel(n_leds, ledPins[0], NEO_GRB + NEO_KHZ800);
  
  readValue = 0;
  
  strip.begin();
  strip.show();
}

void loop() {
  readValue = readValue*0.9 + analogRead(scalePins[0]) * 0.1;
  Serial.println(int(readValue));
  
  if(Serial.available() > 0) {
    char prefix = Serial.read();
    if(prefix == 'I') {
      int r = Serial.parseInt();
      int g = Serial.parseInt();
      int b = Serial.parseInt();
      
      for(int i = 0; i < n_leds; i++) {
        strip.setPixelColor(i, r, g, b);
      }
      strip.show();
    }
  }
  
  delay(1);
}
