#include "config.h"
#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  Serial1.println("\n");

  Serial1.println(symbols);
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial1.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(1000);
  }
  Serial1.println("Connected to WiFi");
  Serial1.print("IP Address: ");
  Serial1.println(WiFi.localIP());

  delay(5000);
}
