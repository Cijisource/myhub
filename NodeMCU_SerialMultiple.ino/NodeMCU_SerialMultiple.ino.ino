#include <SoftwareSerial.h>
SoftwareSerial serialPort(D1,D0);
#include <ArduinoJson.h>

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "ThingSpeak.h"

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";

WidgetBridge bridge(V0);

char auth[] = "DYLNiU66yHBL8I09OrJ0g5X4r_AbS66J";

// Your WiFi credentials.
// Set password to "" for open networks.
// char ssid[] = "Cijaiz_Home";
// char pass[] = "M00n5050";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";
//char ssid[] = "Cijaiz complex";
//char pass[] = "9000150001";
char ssid[] = "Cijai_ComplexClone";
char pass[] = "9000150002";


WiFiClient client;

BlynkTimer timer;
//BlynkTimer uploadTimer;
BlynkTimer notifyTimer;

long systemUptime, uptimesec;
long distance, cdistance;
int tankPercentage, ctankPercentage;
float availableLitres, cavailableLitres, waterlevelAt, cwaterlevelAt; 
float consumedLitres, cconsumedLitres;
int isSlow, isShigh, isClow, isChigh;
int isSlowNotify, isShighNotify, isClowNotify, isChighNotify;

void setup() {
  Blynk.begin(auth, ssid, pass);
  //ThingSpeak.begin(client);
  // Setup a function to be called every second
  timer.setInterval(1000L, uploadtoBlynk);
  //uploadTimer.setInterval(60000L, uploadToThingSpeak);
  notifyTimer.setInterval(900000L, notifyToApp);
  
  Serial.begin(115200);
  serialPort.begin(115200);
  Serial.print("----------- Setup... ----------");
  while (!Serial) continue;
}

BLYNK_CONNECTED() {
  bridge.setAuthToken("ODbXgkyA-fZohqppkwa0qm8QusGnDXCa");
}

void ExtractSensorData() {  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(serialPort);

  long uptimemls = millis();
  uptimesec = uptimemls/1000;
  
//  Serial.println("Esp uptime ");
//  Serial.println(uptimesec);

  if (!serialPort.available() > 0) {
    //Serial.println("received Data");
    distance = -100;
//    Serial.println("Didnt Receive Data");
  }  
  
  if(root == JsonObject::invalid()) 
    return;

//  Serial.println("JSON Received and Parsed");
//  root.prettyPrintTo(Serial);
//  Serial.println("");

  systemUptime=root["ArduinoUptime"];
  distance=root["SensorDistance"];
  tankPercentage=root["TankLevelPercentage"];
  availableLitres = root["AvailableLitres"];
  consumedLitres = root["ConsumedLitres"];
  waterlevelAt = root["SWaterlevelat"];

  cdistance=root["CSensorDistance"];
  ctankPercentage=root["CTankLevelPercentage"];
  cavailableLitres = root["CAvailableLitres"];
  cconsumedLitres = root["CConsumedLitres"];
  cwaterlevelAt = root["CWaterlevelat"];
  
  isSlow = root["isSlow"];
  isShigh = root["isShigh"];
  isClow = root["isClow"];
  isChigh = root["isChigh"];

//  Serial.println(isSlow);
//  Serial.println(isShigh);
//  Serial.println(isClow);
//  Serial.println(isChigh);
  
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
  Blynk.virtualWrite(V4, waterlevelAt);
  
  Blynk.virtualWrite(V5, systemUptime);  
  Blynk.virtualWrite(V6, uptimesec);

  Blynk.virtualWrite(V10, ctankPercentage);
  Blynk.virtualWrite(V11, cdistance);
  Blynk.virtualWrite(V12, cconsumedLitres);
  
  Blynk.virtualWrite(V13, cavailableLitres);
  Blynk.virtualWrite(V14, cwaterlevelAt);
 
  //Notificaation Cloud sync...
//  Blynk.virtualWrite(V30, isSlowNotify);
//  Blynk.virtualWrite(V31, isShighNotify);
//  
//  Blynk.virtualWrite(V32, isClowNotify);
//  Blynk.virtualWrite(V33, isChighNotify);
//  
    //Notifications..
//  Blynk.virtualWrite(V20, isSlow);
//  Blynk.virtualWrite(V21, isShigh);
//  
//  Blynk.virtualWrite(V22, isClow);
//  Blynk.virtualWrite(V23, isChigh);
  
  //Bridge Transmit
  bridge.virtualWrite(V0, tankPercentage);
  bridge.virtualWrite(V10, ctankPercentage);
}

//Compressor Low notify Sync.
BLYNK_WRITE(V30) {
  isSlowNotify = param.asInt();
}

//Compressor High notify Sync.
BLYNK_WRITE(V31) {
  isShighNotify = param.asInt();
}

//Cement Low notify Sync.
BLYNK_WRITE(V32) {
  isClowNotify = param.asInt();
}

//Cement high notify Sync.
BLYNK_WRITE(V33) {
  isChighNotify = param.asInt();
}

//Compressor Low
BLYNK_WRITE(V20) {
  isSlow = param.asInt();

  if(isSlowNotify == 0 && isSlow == 1) {
     Blynk.notify("Compressor Tank is Empty!! Please switch On Motor.");
    isSlowNotify = 1;
  }
  else if(isSlow == 0) {
    isSlowNotify = 0;
  }

  //Update server.
  Blynk.virtualWrite(V30, isSlowNotify);  
}

  //Compressor High
BLYNK_WRITE(V21) {
  isShigh = param.asInt();

  if(isShighNotify == 0 && isShigh == 1) {
     Blynk.notify("Compressor Tank is Full!! Please switch Off Motor.");
    isShighNotify = 1;
  }
  else if(isShigh == 0) {
    isShighNotify = 0;
  }

 //Update server.
  Blynk.virtualWrite(V31, isShighNotify);
}

 //Cement Low
BLYNK_WRITE(V22) {
  isClow = param.asInt();

  if(isClowNotify == 0 && isClow == 1) {
     Blynk.notify("Cement Tank is Empty!! Please switch On Motor.");
    isClowNotify = 1;
  }
  else if(isClow == 0) {
    isClowNotify = 0;
  }
    //Update server.
    Blynk.virtualWrite(V32, isClowNotify);
}

//Cement High
BLYNK_WRITE(V23) {
  isChigh = param.asInt();

  if(isChighNotify == 0 && isChigh == 1) {
     Blynk.notify("Cement Tank is Full!! Please switch Off Motor.");
    isChighNotify = 1;
  }
  else if(isChigh == 0) {
    isChighNotify = 0;
  }
    //Update server.
    Blynk.virtualWrite(V33, isChighNotify);
}

void notifyToApp() 
{
  if(isSlowNotify == 0 && isSlow == 1) {
     Blynk.notify("Compressor Tank is Empty!! Please switch On Motor.");
    isSlowNotify = 1;
  }
  else if(isSlow == 0) {
    isSlowNotify = 0;
  }

  if(isShighNotify == 0 && isShigh == 1) {
     Blynk.notify("Compressor Tank is Full!! Please switch Off Motor.");
    isShighNotify = 1;
  }
  else if(isShigh == 0) {
    isShighNotify = 0;
  }

  if(isClowNotify == 0 && isClow == 1) {
     Blynk.notify("Cement Tank is Empty!! Please switch On Motor.");
    isClowNotify = 1;
  }
  else if(isClow == 0) {
    isClowNotify = 0;
  }

  if(isChighNotify == 0 && isChigh == 1) {
     Blynk.notify("Cement Tank is Full!! Please switch Off Motor.");
    isChighNotify = 1;
  }
  else if(isChigh == 0) {
    isChighNotify = 0;
  }
}

void uploadToThingSpeak()
{
  //Upload to Thinkspeak
  int httpCode = ThingSpeak.writeField(myChannelNumber, 1, tankPercentage, myWriteAPIKey);
//  if (httpCode == 200) {
//    Serial.println("Channel write successful.");
//  }
//  else {
//    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
//  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 2, consumedLitres, myWriteAPIKey);
//  if (httpCode == 200) {
//    Serial.println("Channel write successful.");
//  }
//  else {
//    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
//  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 3, availableLitres, myWriteAPIKey);
//  if (httpCode == 200) {
//    Serial.println("Channel write successful.");
//  }
//  else {
//    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
//  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 4, ctankPercentage, myWriteAPIKey);
//  if (httpCode == 200) {
//    Serial.println("Channel write successful.");
//  }
//  else {
//    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
//  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 5, cconsumedLitres, myWriteAPIKey);
//  if (httpCode == 200) {
//    Serial.println("Channel write successful.");
//  }
//  else {
//    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
//  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 6, cavailableLitres, myWriteAPIKey);
//  if (httpCode == 200) {
//    Serial.println("Channel write successful.");
//  }
//  else {
//    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
//  }
}

void loop() {
  ExtractSensorData();
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
  //uploadTimer.run();
  notifyTimer.run();
}
