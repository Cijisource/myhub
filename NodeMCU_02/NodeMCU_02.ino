#include <SoftwareSerial.h>
SoftwareSerial serialPort(D1,D0);
#include <ArduinoJson.h>

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "ThingSpeak.h"

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";

char auth[] = "ODbXgkyA-fZohqppkwa0qm8QusGnDXCa";

// Your WiFi credentials.
// Set password to "" for open networks.
// char ssid[] = "Cijaiz_Home";
// char pass[] = "M00n5050";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";

char ssid[] = "Cijaiz complex";
char pass[] = "9000150001";

WiFiClient client;

BlynkTimer timer;
BlynkTimer uploadTimer;
long systemUptime, uptimesec;
long distance, cdistance;

int tankPercentage, ctankPercentage, compressorTankPercentage, cementTankPercentage;
float availableLitres, cavailableLitres; 
float consumedLitres, cconsumedLitres, waterlevelat;

bool isSTankLowEmailSent = true;
bool isSTankFullEmailSent = true;
bool isCTankLowEmailSent = true;
bool isCTankFullEmailSent = true;

void setup() {
  Serial.println("----------------SETUP INITIATED--------------------------");
  Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  // Setup a function to be called every second
  timer.setInterval(1000L, uploadtoBlynk);
  uploadTimer.setInterval(600000L, uploadToThingSpeak);

  isSTankLowEmailSent = false;
  isSTankFullEmailSent = false;
  isCTankLowEmailSent = false;
  isCTankFullEmailSent = false;
  
  Serial.begin(9600);
  serialPort.begin(115200);
  while (!Serial) continue;
  Serial.println("----------------SETUP COMPLETED--------------------------");
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
//    Serial.println("Didnt Receive Data");
  }  
  
  if(root == JsonObject::invalid()) 
    return;

  Serial.println("JSON Received and Parsed");
  root.prettyPrintTo(Serial);
  Serial.println("");

  systemUptime=root["ArduinoUptime"];
  distance=root["SSensorDistance"];
  tankPercentage=root["STankLevelPercentage"];
  
  availableLitres = root["SAvailableLitres"];
  consumedLitres = root["SConsumedLitres"];
  waterlevelat = root["SWaterLevel"];
  
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
  Blynk.virtualWrite(V4, tankPercentage);
  Blynk.virtualWrite(V1, distance);
  Blynk.virtualWrite(V2, consumedLitres);
  Blynk.virtualWrite(V3, availableLitres);
  
  Blynk.virtualWrite(V5, systemUptime);  
  Blynk.virtualWrite(V6, uptimesec);
  Blynk.virtualWrite(V9, waterlevelat);

  Blynk.virtualWrite(V7, compressorTankPercentage);
  Serial.println(compressorTankPercentage);

  Blynk.virtualWrite(V8, cementTankPercentage);
  Serial.println(cementTankPercentage);
}

BLYNK_CONNECTED(){
  Blynk.email("{DEVICE_NAME} Successfully Connected", "{DEVICE_NAME} Connected");
}

BLYNK_WRITE(V0) {
  compressorTankPercentage = param.asInt();

  if(compressorTankPercentage > 90) {
    if(!isSTankFullEmailSent) {
      Blynk.email("Compressor Tank", "Compressor Tank is Full");  
      isSTankFullEmailSent = true;
    }
  }
  else if(compressorTankPercentage > 20) {
    isSTankFullEmailSent = false;
  }
   
  if (compressorTankPercentage < 40 && compressorTankPercentage > 10) {
    if (!isSTankLowEmailSent) {
      Blynk.email("Compressor Tank", "Quarter Level reached. Please Refill.");
      Serial.println(isSTankLowEmailSent);
      Serial.println("Compressor mail sent..");
      isSTankLowEmailSent = true; 
    }
  }
  else if(compressorTankPercentage > 20) {
    isSTankLowEmailSent = false; 
  }
}

BLYNK_WRITE(V10) {
  cementTankPercentage = param.asInt();
  
  if(cementTankPercentage == 100) {
    if (!isCTankFullEmailSent) {
      Blynk.email("Cement Tank", "Cement Tank is Full.");
      Serial.println(isCTankFullEmailSent);
      Serial.println("Cement tank mail sent..");
      isCTankFullEmailSent = true;  
    }
  }
  else {
    isCTankFullEmailSent = false;
  }
  
  if(cementTankPercentage < 40 && cementTankPercentage > 10) {
    if (!isCTankLowEmailSent) {
        Blynk.email("Cement Tank", "Quarter Level reached. Please Refill.");
        Serial.println(isCTankLowEmailSent );
        Serial.println("Cement tank quarter mail sent..");
        
        isCTankLowEmailSent = true;  
    }
  }
  else {
    isCTankLowEmailSent = false;
  }
}

void uploadToThingSpeak()
{
  //Upload to Thinkspeak
  int httpCode = ThingSpeak.writeField(myChannelNumber, 7, tankPercentage, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Channel write successful.");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 8, consumedLitres, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Channel write successful.");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }
  httpCode = ThingSpeak.writeField(myChannelNumber, 9, availableLitres, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Channel write successful.");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }
}

void loop() {
  ExtractSensorData();
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
  uploadTimer.run();
}
