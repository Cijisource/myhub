#define BLYNK_TEMPLATE_ID "TMPLe3Z3HbRn"
#define BLYNK_TEMPLATE_NAME "Main Tank Monitor"
#define BLYNK_AUTH_TOKEN "w-R8a_nmrqsPSdWhD7WFTKn02G6ptVtu"
#define DEVICE_NAME "Main Tank Monitor"
#define DEVICE_SOFTWARE "ESP_MAINTANK_08_03_2025{DD_MM_YYYY}"
#define BLYNK_FIRMWARE_VERSION "3.0.0"

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include "ThingSpeak.h"
#include "classfile.hpp"

BlynkTimer uploadBlynkTimer;
BlynkTimer uploadThingSpeakTimer;
BlynkTimer systemTimer;
BlynkTimer wifiChecker;

WiFiClient client;

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "0T2KKZGK0INX3DF6";

void setup() {
  Serial.begin(9600); // Initialize hardware serial for debugging
  
  Serial.println("----------------SETUP INITIATED--------------------------");
  Serial.println("ESP8266 ready to receive data.");
  
  //WiFiManager setup..
  setupWifiManager(0);
  
  //setupDateTime();
  setupTimers();

  //Configure and connect blynk by extracting SSID and Password from WIFI Manager..
  blynkConnect();
  
  //Initilize Thing speak..
  ThingSpeak.begin(client);
  delay(200);
  
  Serial.println("----------------SETUP COMPLETED--------------------------");
  setupConfiguration = DEVICE_NAME "--" DEVICE_SOFTWARE;
  Serial.println(setupConfiguration);
}

void setupTimers() {
  // Setup a function to be called every second : 1000L = 1000ms = 1s
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(10000L, uploadtoBlynkPart1); // 10 second
  uploadBlynkTimer.setInterval(25000L, uploadtoBlynkPart2); // 15 second
  uploadThingSpeakTimer.setInterval(30000L, uploadToThingSpeakPart1); // (108000L -- 1.8 minutes)
  uploadThingSpeakTimer.setInterval(50000L, uploadToThingSpeakPart2); // (108000L -- 1.8 minutes)
  
  systemTimer.setInterval(1000L, setupDateTime); // 1 secoond  
}

void ExtractSensorData() {  
  StaticJsonDocument<200> doc; //Extracted Source Object..  
  
    if (Serial.available()) { // Check if data is available on the hardware serial (connected to Arduino's SoftwareSerial TX)
      receivedJson = Serial.readStringUntil('\n'); // Read until newline character
      Serial.print("Received on ESP8266: ");
      Serial.println(receivedJson);

    //Deserialization Logic.
    DeserializationError error = deserializeJson(doc, receivedJson);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
      // Extract data
      systemUptime = doc["aurdinouptimesec"];
    
    //Compressor
      distance=doc["SensorDistance"];
      tankPercentage=doc["TankLevelPercentage"];
      availableLitres = doc["AvailableLitres"];
      consumedLitres = doc["ConsumedLitres"];
      waterlevelAt = doc["SWaterlevelat"];
    
    //Cement
      cdistance=doc["CSensorDistance"];
      ctankPercentage=doc["CTankLevelPercentage"];  
      cavailableLitres = doc["CAvailableLitres"];
      cconsumedLitres = doc["CConsumedLitres"];
      cwaterlevelAt = doc["CWaterlevelat"];
      
    //Mini
    mdistance=doc["MSensorDistance"];
      mtankPercentage=doc["MTankLevelPercentage"];
      mavailableLitres = doc["MAvailableLitres"];
      mconsumedLitres = doc["MConsumedLitres"];
      mwaterlevelAt = doc["MWaterlevelat"];
      
      isSlow = doc["isSlow"];
      isShigh = doc["isShigh"];
      isClow = doc["isClow"];
      isChigh = doc["isChigh"];
      isMhigh = doc["isMhigh"];
      isMlow = doc["isMlow"];
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
  if (!wifiManager.autoConnect("MainTank_AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(300);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(500);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
}

void uploadtoBlynk(){
  Blynk.virtualWrite(V4, tankPercentage);
  Blynk.virtualWrite(V1, distance);
  Blynk.virtualWrite(V2, consumedLitres);
  //Blynk.virtualWrite(V3, availableLitres);
  
  //Blynk.virtualWrite(V5, systemUptime);  
  Blynk.virtualWrite(V6, systemUptime);
  //Blynk.virtualWrite(V9, waterlevelat);

  //Blynk.virtualWrite(V7, compressorTankPercentage);
  //Serial.println(compressorTankPercentage);

  //Blynk.virtualWrite(V8, cementTankPercentage);
  //Serial.println(cementTankPercentage);
}

void uploadtoBlynkPart1(){
  if(isBlynkPart1Complete == false){

    blynkStatus = "";
    Blynk.virtualWrite(V6, systemUptime);
    Blynk.virtualWrite(V5, currentDate); 
    
    //Compressor
    Blynk.virtualWrite(V0, tankPercentage);
    Blynk.virtualWrite(V1, distance);
    Blynk.virtualWrite(V4, waterlevelAt);

    //Cement
    Blynk.virtualWrite(V10, ctankPercentage);
    Blynk.virtualWrite(V11, cdistance);
    Blynk.virtualWrite(V14, cwaterlevelAt);
    
    //Mini
    Blynk.virtualWrite(V15, mtankPercentage);
    Blynk.virtualWrite(V16, mdistance);
    Blynk.virtualWrite(V19, mwaterlevelAt);

    blynkStatus = "Blynk Upload Complete.. part1" + currentDate;
    terminal.println(blynkStatus);
  Serial.println(blynkStatus);

    isBlynkPart1Complete = true;
    isBlynkPart2Complete = false;
   }
}

void uploadtoBlynkPart2(){
  if(isBlynkPart1Complete == true && isBlynkPart2Complete == false){
    blynkStatus = "";

    //Compressor
    Blynk.virtualWrite(V2, consumedLitres);
    Blynk.virtualWrite(V3, availableLitres);

    //Mini
    Blynk.virtualWrite(V17, mconsumedLitres);
    Blynk.virtualWrite(V18, mavailableLitres);

    //Cement
    Blynk.virtualWrite(V12, cconsumedLitres);
    Blynk.virtualWrite(V13, cavailableLitres);

    Blynk.virtualWrite(V6, systemUptime);

    blynkStatus = "Blynk Upload Complete.. Part2 " + currentDate;
    terminal.println(blynkStatus);
  Serial.println(blynkStatus);

    isBlynkPart1Complete = false;
    isBlynkPart2Complete = true;
   }
}

void uploadToThingSpeak()
{ 
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
  terminal.flush();
}

void uploadToThingSpeakPart1()
{ 
    Serial.println("1");
    if(isThingPart1Complete == false){    
        //Upload to Thinkspeak        
        ThingSpeak.setField(1, tankPercentage);
        ThingSpeak.setField(2, consumedLitres);
        ThingSpeak.setField(3, availableLitres);
      
        ThingSpeak.setField(4, ctankPercentage);
        ThingSpeak.setField(5, cconsumedLitres);
        ThingSpeak.setField(6, cavailableLitres);
      
        thingspeakStatus = "";
        int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if (httpCode == 200) {
      thingspeakStatus = "Thinkspeak Channel write successful. CH:1";
          Serial.println(thingspeakStatus);
        }
        else {
          Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
          thingspeakStatus = "Problem writing to channel via part1. HTTP error code " + String(httpCode);
          Blynk.logEvent("attentionrequired", String(httpCode) + "--" + thingspeakStatus);
        }
      
        thingspeakStatus = thingspeakStatus + currentDate;
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
      
      //Upload to Thinkspeak
      //ThingSpeak.setField(2, consumedLitres);
    
      thingspeakStatus = "";
      int httpCode = 200; //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if (httpCode == 200) {
      thingspeakStatus = "ThingSpeak Channel write successful. -- CH:2 Dummy ONly";
          Serial.println(thingspeakStatus);
      }
      else {
        Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
        thingspeakStatus = "Problem writing to channel via part2. HTTP error code " + String(httpCode);
        Blynk.logEvent("attentionrequired", thingspeakStatus + currentDate);
      }
    
      thingspeakStatus = thingspeakStatus + currentDate;
      terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
      terminal.flush();
  
     isThingPart1Complete = false;
     isThingPart2Complete = true;  
  }
}

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V50)
{  
  terminalCall(param.asStr());

  if (String("resetwifi") == param.asStr()) {
    setupWifiManager(1);
  }
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

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();

  //Aurdino COllection.
  ExtractSensorData();
  
  // Initiates SimpleTimer
  systemTimer.run();
  uploadThingSpeakTimer.run();
  uploadBlynkTimer.run();
}
