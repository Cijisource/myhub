#include <SoftwareSerial.h>
SoftwareSerial serialPort(D1,D0);
#include <ArduinoJson.h>

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "ODbXgkyA-fZohqppkwa0qm8QusGnDXCa";

// Your WiFi credentials.
// Set password to "" for open networks.
// char ssid[] = "Cijaiz_Home";
// char pass[] = "M00n5050";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";
char ssid[] = "Cijaiz complex";
char pass[] = "9000150001";

void setup() {
  Blynk.begin(auth, ssid, pass);
    
  Serial.begin(9600);
}

BLYNK_WRITE(V0) {
  int compressorTankPercentage = param.asInt();
  Blynk.virtualWrite(V10, compressorTankPercentage);
}

void loop() {
  Blynk.run();
}
