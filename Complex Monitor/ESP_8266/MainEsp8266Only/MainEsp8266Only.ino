#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLe3Z3HbRn"
#define BLYNK_TEMPLATE_NAME "Main Tank Monitor"
#define BLYNK_AUTH_TOKEN "w-R8a_nmrqsPSdWhD7WFTKn02G6ptVtu"
#define DEVICE_NAME "Main Tank Monitor"
#define DEVICE_SOFTWARE "ESP_MAINTANK_01_25_2025{DD_MM_YYYY}"
#define BLYNK_FIRMWARE_VERSION "2.0.2"

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
BlynkTimer systemTimer;
BlynkTimer wifiChecker;

StaticJsonBuffer<1000> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

WiFiClient client;
String str;

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";
char auth[] = "3S2mjm0uyjmgkmZ_WXi3L3TgFEWz6b1E";

const int strigger = 12;
const int secho = 14;

const int ctrigger = 4;
const int cecho = 5;

const int mtrigger = 13;
const int mecho = 15;

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

  //setupDateTime();
  setupTimers();

  //Configure and connect blynk by extracting SSID and Password from WIFI Manager..
  blynkConnect();

  //Initilize Thing speak..
  ThingSpeak.begin(client);
  delay(200);

  pinMode(strigger, OUTPUT); // Sets the trigPin as an Output
  pinMode(secho, INPUT); // Sets the echoPin as an Input

  pinMode(ctrigger, OUTPUT); // Sets the trigPin as an Output
  pinMode(cecho, INPUT); // Sets the echoPin as an Input

  pinMode(mtrigger, OUTPUT); // Sets the trigPin as an Output
  pinMode(mecho, INPUT); // Sets the echoPin as an Input

  Serial.println("----------------SETUP COMPLETED--------------------------");
}

void setupTimers() {
  // Setup a function to be called every second : 1000L = 1000ms = 1s
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(10000L, uploadtoBlynkPart1); // 10 second
  uploadBlynkTimer.setInterval(25000L, uploadtoBlynkPart2); // 15 second
  uploadThingSpeakTimer.setInterval(20000L, uploadToThingSpeakPart1); // (108000L -- 1.8 minutes)
  uploadThingSpeakTimer.setInterval(40000L, uploadToThingSpeakPart2); // (108000L -- 1.8 minutes)
  
  systemTimer.setInterval(1000L, setupDateTime); // 1 secoond  
  //wifiChecker.setInterval(1000L, checkWaterLevelInCompressorTank); // 30 mins 900000L // 
  //wifiChecker.setInterval(1500L, checkWaterLevelInCementTank); // 30 mins 900000L // 
  //wifiChecker.setInterval(2200L, checkWaterLevelInMiniTank); // 30 mins 900000L // 
  wifiChecker.setInterval(3200L, checkWaterLevel); // 3.2 seconds..// 
}

void checkWaterLevel(){
  checkWaterLevelInCompressorTank();
  delayMicroseconds(20);
  checkWaterLevelInCementTank();
  delayMicroseconds(20);
  checkWaterLevelInMiniTank();
  delayMicroseconds(20);
}

void checkWaterLevelInCompressorTank() {
  uptimemls = millis();
  long uptimesec = uptimemls/1000;
  
  long duration, distance;
  int tanklevelpercentage = 0;

  digitalWrite(strigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(strigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(strigger, LOW);
  duration = pulseIn(secho, HIGH);
  distance = (duration/2) / 29.1;

  //Simulate..
  //distance = 135;
  
  //Blynk.virtualWrite(V1, distance);
  root["ArduinoUptime"] = uptimesec;
  root["SensorDistance"] = distance;
  
  scalibrationvalue = ssensorrestorecalibration;
  if(distance > (stankheight + scalibrationvalue)) {
    //Serial.println(distance);
    distance = stankheight;
    scalibrationvalue = 0;
  } 
  float availablelitres = measureWater(distance, scalibrationvalue, stankheight, stankwidth, stanklength);
  float consumedlitres = consumedWater(distance, scalibrationvalue, stankheight, stankwidth, stanklength);

  //Blynk.virtualWrite(V3, availablelitres);
  root["AvailableLitres"] = availablelitres;

  //Blynk.virtualWrite(V2, consumedlitres);
  root["ConsumedLitres"] = consumedlitres;

  int waterlevelat = 0;
  if(distance > 0) {
    waterlevelat = stankheight - distance;
//    Serial.println(scalibrationvalue);
//    Serial.println("calibration value printed above");
  }
//  Serial.println("Waterlevelat");
//  Serial.println(waterlevelat);
  
  tanklevelpercentage = (waterlevelat + scalibrationvalue) / stankheight * 100;

  if(tanklevelpercentage < 0)  {
    tanklevelpercentage = 0;
  }
  
  if(tanklevelpercentage < 30 && tanklevelpercentage > 10) {
    isSlow = 1;
  }
  else if(tanklevelpercentage > 30) {
    isSlow = 0;
  }
  
  if(tanklevelpercentage > 95) {
    isShigh = 1;
  }
  else if(tanklevelpercentage < 95 && tanklevelpercentage > 10) {
    isShigh = 0;
  }
  
  root["isSlow"] = isSlow;
  root["isShigh"] = isShigh;
  
  //Blynk.virtualWrite(V0, tanklevelpercentage);
  root["TankLevelPercentage"] = tanklevelpercentage;
  
  root["SWaterlevelat"] = waterlevelat/30.48;
  //Blynk.virtualWrite(V5, uptimesec);

  //root.prettyPrintTo(Serial);
}

void checkWaterLevelInCementTank() {
  long duration, distance;
  int tanklevelpercentage = 0;

  digitalWrite(ctrigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(ctrigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(ctrigger, LOW);
  duration = pulseIn(cecho, HIGH);
  distance = (duration/2) / 29.1;

  //simulator.
  //distance = 80;
  
  Serial.println("cement duration");
  Serial.println(duration);

  Serial.println("cement distance");
  Serial.println(distance);
  
  //Blynk.virtualWrite(V1, distance);
  root["CSensorDistance"] = distance;
  
  ccalibrationvalue = csensorrestorecalibration;
  if(distance > (ctankheight + ccalibrationvalue)) {
//    Serial.println(distance);
    distance = ctankheight;
    ccalibrationvalue = 0;
  } 
  float availablelitres = measureWater(distance, ccalibrationvalue, ctankheight, ctankwidth, ctanklength);
  float consumedlitres = consumedWater(distance, ccalibrationvalue, ctankheight, ctankwidth, ctanklength);

  //Blynk.virtualWrite(V13, availablelitres);
  root["CAvailableLitres"] = availablelitres;

  //Blynk.virtualWrite(V12, consumedlitres);
  root["CConsumedLitres"] = consumedlitres;

  int waterlevelat = 0;
  if(distance > 0) {
    waterlevelat = ctankheight + ccalibrationvalue - distance;
//    Serial.println(ccalibrationvalue);
//    Serial.println("calibration value printed above");
  }
//  Serial.println("CWaterlevelat");
//  Serial.println(waterlevelat);
  tanklevelpercentage = waterlevelat / ctankheight * 100;
//  Serial.println("cement");
//  Serial.println(ctankheight);
//  Serial.println(waterlevelat);
//  Serial.println(tanklevelpercentage);
//  if(tanklevelpercentage > 100)  {
//    tanklevelpercentage = 100;
//  }
  if(tanklevelpercentage < 0)  {
    tanklevelpercentage = 0;
  }
    
  if(tanklevelpercentage < 30 && tanklevelpercentage > 10) {
    isClow = 1;
  }
  else if(tanklevelpercentage > 30) {
    isClow = 0;
  }
  
  if(tanklevelpercentage > 95) {
    isChigh = 1;
  }
  else if(tanklevelpercentage < 95 && tanklevelpercentage > 10) {
    isChigh = 0;
  }
  
  //Blynk.virtualWrite(V0, tanklevelpercentage);
  root["isClow"] = isClow;
  root["isChigh"] = isChigh;
    
  root["CTankLevelPercentage"] = tanklevelpercentage;
  root["CWaterlevelat"] = waterlevelat/30.48;
  //Blynk.virtualWrite(V5, uptimesec);
}

void checkWaterLevelInMiniTank() {
  long duration = 0, distance = 0;
  int tanklevelpercentage = 0;

  digitalWrite(mtrigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(mtrigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(mtrigger, LOW);
  duration = pulseIn(mecho, HIGH);
  
  //simulator.
  //duration = 500;
  distance = (duration/2) / 29.1;
 
  Serial.println("Mini duration");
  Serial.println(duration);

  Serial.println("Mini distance");
  Serial.println(distance);
  
  //Blynk.virtualWrite(V1, distance);
  root["MSensorDistance"] = distance;
  
  mcalibrationvalue = msensorrestorecalibration;
  if(distance > (mtankheight + mcalibrationvalue)) {
//    Serial.println(distance);
    distance = mtankheight;
    mcalibrationvalue = 0;
  } 
  float availablelitres = measureWater(distance, mcalibrationvalue, mtankheight, mtankwidth, mtanklength);
  float consumedlitres = consumedWater(distance, mcalibrationvalue, mtankheight, mtankwidth, mtanklength);

  //Blynk.virtualWrite(V13, availablelitres);
  root["MAvailableLitres"] = availablelitres;

  //Blynk.virtualWrite(V12, consumedlitres);
  root["MConsumedLitres"] = consumedlitres;

  int waterlevelat = 0;
  if(distance > 0) {
    waterlevelat = mtankheight + mcalibrationvalue - distance;
//    Serial.println(mcalibrationvalue);
//    Serial.println("calibration value printed above");
  }
//  Serial.println("mWaterlevelat");
//  Serial.println(waterlevelat);
  tanklevelpercentage = waterlevelat / mtankheight * 100;
//  Serial.println("cement");
//  Serial.println(ctankheight);
//  Serial.println(waterlevelat);
//  Serial.println(tanklevelpercentage);
//  if(tanklevelpercentage > 100)  {
//    tanklevelpercentage = 100;
//  }
  if(tanklevelpercentage < 0)  {
    tanklevelpercentage = 0;
  }
    
  if(tanklevelpercentage < 30 && tanklevelpercentage > 10) {
    isMlow = 1;
  }
  else if(tanklevelpercentage > 30) {
    isMlow = 0;
  }
  
  if(tanklevelpercentage > 95) {
    isMhigh = 1;
  }
  else if(tanklevelpercentage < 95 && tanklevelpercentage > 10) {
    isMhigh = 0;
  }
  
  //Blynk.virtualWrite(V0, tanklevelpercentage);
  root["isMlow"] = isMlow;
  root["isMhigh"] = isMhigh;
    
  root["MTankLevelPercentage"] = tanklevelpercentage;
  root["MWaterlevelat"] = waterlevelat/30.48;
  //Blynk.virtualWrite(V5, uptimesec);
}

void ExtractSensorData() {  
  //TODO: Comment this piece in production code.
  //simulateSensor();
  //Serial.println(".");

  //int portStatus = serialPort.available();
  //serialPortStatus = portStatus;
  
  //if (serialPort.available()) {
      //str = serialPort.readString();
      //Serial.print("esp: ");
      //Serial.println(str);
      //serialPort.flush();
      
      //StaticJsonBuffer<1000> jsonBuffer;
      //JsonObject& root = jsonBuffer.parseObject(serialPort);
      //StaticJsonBuffer<200> jsonBuffer;
      //JsonObject& root = jsonBuffer.createObject();
      
      //Serial.println("Esp uptime ");
      //Serial.println(uptimesec);
    
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
      mdistance=root["MSensorDistance"];
      mtankPercentage=root["MTankLevelPercentage"];
      mavailableLitres = root["MAvailableLitres"];
      mconsumedLitres = root["MConsumedLitres"];
      mwaterlevelAt = root["MWaterlevelat"];
      
      isSlow = root["isSlow"];
      isShigh = root["isShigh"];
      isClow = root["isClow"];
      isChigh = root["isChigh"];
      isMhigh = root["isMhigh"];
      isMlow = root["isMlow"];
    
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
  //}
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
  ExtractSensorData();
  
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

    isBlynkPart1Complete = true;
    isBlynkPart2Complete = false;
   }
}

void uploadtoBlynkPart2(){
  ExtractSensorData();
    
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

    isBlynkPart1Complete = false;
    isBlynkPart2Complete = true;
   }
}

void uploadToThingSpeak()
{ 
  ExtractSensorData();
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
    ExtractSensorData();
    Serial.println("1");
    if(isThingPart1Complete == false){    
        //Upload to Thinkspeak
        //tankPercentage = 50;
        //consumedLitres = 1000;
        //availableLitres = 2000;

        //ctankPercentage = 90;
        //cconsumedLitres = 500;
        //cavailableLitres = 2500;
        
        ThingSpeak.setField(1, tankPercentage);
        ThingSpeak.setField(2, consumedLitres);
        ThingSpeak.setField(3, availableLitres);
      
        ThingSpeak.setField(4, ctankPercentage);
        ThingSpeak.setField(5, cconsumedLitres);
        ThingSpeak.setField(6, cavailableLitres);
      
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
        terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
        terminal.flush();
  
    isThingPart1Complete = true;
    isThingPart2Complete = false;  
  }
}

void uploadToThingSpeakPart2()
{ 
  ExtractSensorData();
  Serial.println("2");
  if(isThingPart2Complete == false){
      Serial.println("Upload Call from Thing2 -- " + currentDate);
      
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

  // Initiates SimpleTimer
  systemTimer.run();
  uploadThingSpeakTimer.run();
  uploadBlynkTimer.run();
  wifiChecker.run();
}
