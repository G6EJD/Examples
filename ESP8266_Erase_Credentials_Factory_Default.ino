#include <ESP8266WiFi.h>

void setup() {
  resetToFactoryDefaults();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void resetToFactoryDefaults() {
  WiFi.disconnect(); // Erases WiFi Credentials
  delay(3000);
  ESP.reset();
  delay(3000);
  ESP.eraseConfig();
  delay(3000);
}


