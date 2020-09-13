#include <SoftwareSerial.h>
SoftwareSerial serialPort(D1,D0);
#include <ArduinoJson.h>

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "DYLNiU66yHBL8I09OrJ0g5X4r_AbS66J";

// Your WiFi credentials.
// Set password to "" for open networks.
// char ssid[] = "Cijaiz_Home";
// char pass[] = "M00n5050";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";
char ssid[] = "Cijaiz complex";
char pass[] = "9000150001";

BlynkTimer timer;
long systemUptime, uptimesec;
long distance;
float tankPercentage;
float availableLitres; 
float consumedLitres;

void setup() {
  Blynk.begin(auth, ssid, pass);
  // Setup a function to be called every second
  timer.setInterval(1000L, uploadtoBlynk);
  
  Serial.begin(115200);
  serialPort.begin(115200);
  while (!Serial) continue;
}

void ExtractSensorData() {
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(serialPort);

  long uptimemls = millis();
  uptimesec = uptimemls/1000;
  
  Serial.println("Esp uptime ");
  Serial.println(uptimesec);

  if (!serialPort.available() > 0) {
    //Serial.println("received Data");
    distance = -100;
    Serial.println("Didnt Receive Data");
  }  
  
  if(root == JsonObject::invalid()) 
    return;

  Serial.println("JSON Received and Parsed");
  root.prettyPrintTo(Serial);
  Serial.println("");

  systemUptime=root["ArduinoUptime"];
  distance=root["SensorDistance"];
  tankPercentage=root["TankLevelPercentage"];
  availableLitres = root["AvailableLitres"];
  consumedLitres = root["ConsumedLitres"];
        
  //Serial.println("ArduinoUptime ");
  //Serial.print(systemUptime);
  //Serial.println("");
  
  //Serial.println("SensorDistance ");
  //Serial.print(distance);
  //Serial.println("");

  //Serial.println("TankLevelPercentage ");
  //Serial.print(tankPercentage);
  //Serial.println("");

  //Serial.println("AvailableLitres ");
  //Serial.print(availableLitres);
  //Serial.println("");

  //Serial.println("ConsumedLitres ");
  //Serial.print(consumedLitres);
  //Serial.println("");
  
  //Serial.println("----------------------");
}

void uploadtoBlynk(){
  Blynk.virtualWrite(V0, tankPercentage);
  Blynk.virtualWrite(V1, distance);
  Blynk.virtualWrite(V2, consumedLitres);
  Blynk.virtualWrite(V3, availableLitres);
  Blynk.virtualWrite(V5, systemUptime);  
  Blynk.virtualWrite(V10, uptimesec);
}

void loop() {
  ExtractSensorData();
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
}
