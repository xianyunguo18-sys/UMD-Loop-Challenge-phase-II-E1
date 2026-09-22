#include <SoftwareSerial.h>

SoftwareSerial wire(2, 3);   // RX=2 (unused), TX=3

unsigned long lastSend = 0;

void setup() {
  Serial.begin(9600);
  wire.begin(9600);
  randomSeed(analogRead(A0));
  Serial.println("Uno sender ready");
}

void loop() {
  // Auto mode: send a random color every 2 seconds
  if (millis() - lastSend > 2000) {
    lastSend = millis();
    char hex[8];
    sprintf(hex, "#%02X%02X%02X", random(0, 256), random(0, 256), random(0, 256));
    wire.println(hex);
    Serial.print("sent: ");
    Serial.println(hex);
  }

  // Manual mode: forward anything typed into the Serial Monitor
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s.length() > 0) {
      wire.println(s);
      Serial.print("sent: ");
      Serial.println(s);
    }
  }
}
