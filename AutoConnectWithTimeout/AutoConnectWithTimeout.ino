#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID      "TMPL0tRLYzze"
#define BLYNK_TEMPLATE_NAME    "Sintex Tank Monitor"
#define BLYNK_AUTH_TOKEN       "2NMuxK5u-e8X0yB7nF0Ye459GIGH21jC"
#define DEVICE_NAME "Sintex Tank Monitor"
#define DEVICE_SOFTWARE "ESP_SINTEX_12_10_2024{DD_MM_YYYY}"
#define BLYNK_FIRMWARE_VERSION "1.0.0"

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include "ThingSpeak.h"

#include "classfile.hpp"
#include <ArduinoJson.h>

SoftwareSerial serialPort(D1,D2); //Rx and Tx

BlynkTimer uploadBlynkTimer;
BlynkTimer uploadThingSpeakTimer;
BlynkTimer extractSensorTimer;
BlynkTimer systemTimer;
BlynkTimer wifiChecker;

WiFiClient client;
String str;

bool isSTankLowEmailSent = true;
bool isSTankFullEmailSent = true;
bool isCTankLowEmailSent = true;
bool isCTankFullEmailSent = true;

int isSlow, isShigh;
int isSlowNotify, isShighNotify;

bool isThingPart1Complete = false;
bool isThingPart2Complete = false;

unsigned long myChannelNumber = 2362424;
const char * myWriteAPIKey = "3RIPHUIU73AV3SIY";
char auth[] = "WvaGbJC0GIQQH18BITsrz6H3XMxGIB7x";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Serial.begin(115200);
  delay(10);
  serialPort.begin(9600);
  delay(10);
  
  Serial.println("----------------SETUP INITIATED--------------------------");
  setupConfiguration = DEVICE_NAME "--" DEVICE_SOFTWARE;
  
  //WiFiManager setup..
  setupWifiManager(0);

  setupDateTime();
  setupTimers();

  //Configure and connect blynk by extracting SSID and Password from WIFI Manager..
  blynkConnect();
  Serial.println("----------------SETUP COMPLETED--------------------------");
}

void setupTimers() {
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(12000L, uploadtoBlynk); // 10 second
  uploadThingSpeakTimer.setInterval(20000L, uploadToThingSpeakPart1); // (108000L -- 1.8 minutes)
  uploadThingSpeakTimer.setInterval(40000L, uploadToThingSpeakPart2); // (108000L -- 1.8 minutes)
  
  //extractSensorTimer.setInterval(1000L, ExtractSensorData); // 1 secoond  
  systemTimer.setInterval(1000L, setupDateTime); // 1 secoond  
  wifiChecker.setInterval(900000L, connectionCheck); // 30 mins 900000L // 
}

void ExtractSensorData() {  
  //TODO: Comment this piece in production code.
  //simulateSensor();
  
  if (serialPort.available()) {
      //str = serialPort.readString();
      //Serial.print("esp: ");
      //Serial.println(str);
      serialPort.flush();
      
      StaticJsonBuffer<1000> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(serialPort);
  
      //long uptimemls = millis();
      //uptimesec = uptimemls/1000;
    
      //Serial.println("Esp uptime ");
      //Serial.println(uptimesec);
    
      //int portStatus = serialPort.available();
      //serialPortStatus = portStatus;
    
      if(root == JsonObject::invalid()) 
        return;
    
      Serial.println("JSON Received and Parsed");
      Serial.print("Port Status: ");
      Serial.println(serialPortStatus);
      
      root.prettyPrintTo(Serial);
      Serial.println("");
    
      lastDataReceivedTime = currentDate;
    
      receivedJson = "";
      root.prettyPrintTo(receivedJson);
    
      systemUptime=root["ArduinoUptime"];
      uptimesec = systemUptime;
      distance=root["SSensorDistance"];
      tankPercentage=root["STankLevelPercentage"];
      
      availableLitres = root["SAvailableLitres"];
      consumedLitres = root["SConsumedLitres"];
      waterlevelat = root["SWaterLevel"];
      
      isSlow = root["isSlow"];
      isShigh = root["isShigh"];
      
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
}

void uploadtoBlynk(){
  Blynk.virtualWrite(V4, tankPercentage);
  Blynk.virtualWrite(V1, distance);
  Blynk.virtualWrite(V2, consumedLitres);
  Blynk.virtualWrite(V3, availableLitres);
  
  //Blynk.virtualWrite(V5, systemUptime);  
  Blynk.virtualWrite(V6, uptimesec);
  Blynk.virtualWrite(V9, waterlevelat);

  //Blynk.virtualWrite(V7, compressorTankPercentage);
  //Serial.println(compressorTankPercentage);

  //Blynk.virtualWrite(V8, cementTankPercentage);
  //Serial.println(cementTankPercentage);
}

void uploadToThingSpeak()
{ 
  wifiStatus = wifiStatus + WiFi.status();
  
  //Upload to Thinkspeak
  ThingSpeak.setField(7, tankPercentage);
  ThingSpeak.setField(8, consumedLitres);

  thingspeakStatus = "";
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Channel write successful.");
    thingspeakStatus = "Channel write successful.";
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
    thingspeakStatus = "Problem writing to channel. HTTP error code " + String(httpCode);
    Blynk.logEvent("attentionrequired", thingspeakStatus + currentDate);
  }

  thingspeakStatus = thingspeakStatus + currentDate;
  terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
  terminal.println("Last WIFI Status.. " + wifiStatus);
  terminal.flush();
}

void uploadToThingSpeakPart1()
{ 
    Serial.println("1");
    if(isThingPart1Complete == false){    
        //Upload to Thinkspeak
        tankPercentage = 50;
        ThingSpeak.setField(1, tankPercentage);
      
        thingspeakStatus = "";
        int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if (httpCode == 200) {
          Serial.println("Channel write successful.");
          thingspeakStatus = "Channel write successful.";
        }
        else {
          Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
          thingspeakStatus = "Problem writing to channel via part1. HTTP error code " + String(httpCode);
          Blynk.logEvent("attentionrequired", String(httpCode) + "--" + thingspeakStatus);
        }
      
        thingspeakStatus = thingspeakStatus + currentDate;
        terminal.println("Last Wifi Status.. " + wifiStatus);
        terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
        terminal.flush();
  
    isThingPart1Complete = true;
    isThingPart2Complete = false;  
  }
}

void uploadToThingSpeakPart2()
{ 
  Serial.println("2");
  if(isThingPart2Complete == false){
      Serial.println("Upload Call from Thing2 -- " + currentDate);

      wifiStatus = wifiStatus + WiFi.status();
      
      //Upload to Thinkspeak
      ThingSpeak.setField(2, consumedLitres);
    
      thingspeakStatus = "";
      int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if (httpCode == 200) {
        Serial.println("Channel write successful.");
        thingspeakStatus = "Channel write successful.";
      }
      else {
        Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
        thingspeakStatus = "Problem writing to channel via part2. HTTP error code " + String(httpCode);
        Blynk.logEvent("attentionrequired", thingspeakStatus + currentDate);
      }
    
      thingspeakStatus = thingspeakStatus + currentDate;
      terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
      terminal.println("Last WIFI Status.. " + wifiStatus);
      terminal.flush();
  
     isThingPart1Complete = false;
     isThingPart2Complete = true;  
  }
}

BLYNK_CONNECTED(){
  //Blynk.email("{DEVICE_NAME} Successfully Connected", "{DEVICE_NAME} Connected");
  Blynk.logEvent("forinformation", String("Successfully Connected") + DEVICE_NAME);
}

//BLYNK_WRITE(V0) {
//  compressorTankPercentage = param.asInt();
//}
//
//BLYNK_WRITE(V10) {
//  cementTankPercentage = param.asInt();
//}

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V50)
{  
  terminalCall(param.asStr());

  if (String("resetwifi") == param.asStr()) {
    setupWifiManager(1);
  }
}

void setupWifiManager(int isReset) {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  if(isReset == 1) {
    //reset settings - for testing
    wifiManager.resetSettings();
  }
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setConfigPortalTimeout(60);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Sintex_AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(300);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(500);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
}

void blynkConnect() {
  //convert the wifi manager ssid to char array so that it can be used inside the blynk.begin.
  String str = WiFi.SSID();
  char str_array[str.length()];
  str.toCharArray(str_array, str.length());
  char* ssid = str_array;

  //convert the wifi manager pass to char array so that it can be used inside the blynk.begin.
  String strPsk = WiFi.psk();
  char strPsk_array[strPsk.length()];
  str.toCharArray(strPsk_array, strPsk.length());
  char* pass = strPsk_array;

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void connectionCheck() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(3000);
    //ESP.restart();
    Serial.println(WiFi.SSID() + " Connection Failed... Try to Restart the Board..");
    delay(5000);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  ExtractSensorData();

  // Initiates SimpleTimer
  uploadBlynkTimer.run(); 
  //uploadThingSpeakTimer.run();
  extractSensorTimer.run();
  systemTimer.run();
  wifiChecker.run();
}
