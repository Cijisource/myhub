#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "ESPDateTime.h"

#define BLYNK_PRINT Serial  
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "ThingSpeak.h"

SoftwareSerial serialPort(D1,D0);
BlynkTimer uploadBlynkTimer;
BlynkTimer uploadThingSpeakTimer;
BlynkTimer notifyTimer;
BlynkTimer systemTimer;

WidgetBridge bridge(V0);
WiFiClient client;
WidgetTerminal terminal(V50);

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";

char auth[] = "DYLNiU66yHBL8I09OrJ0g5X4r_AbS66J";

// Your WiFi credentials.
// Set password to "" for open networks.
 char ssid[] = "Cijaiz_Home";
 char pass[] = "M00n5050";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";
//char ssid[] = "Cijaiz complex";
//char pass[] = "9000150001";
//char ssid[] = "Cijai_ComplexClone";
//char pass[] = "9000150002";

String setupConfiguration = "---";
String serialPortStatus = "----";
String thingspeakStatus = "----";
String receivedJson = "RECEIVED JSON";
String lastDataReceivedTime = "";
String wifiStatus = "";

long systemUptime, uptimesec;
long distance, cdistance;
int tankPercentage, ctankPercentage;
float availableLitres, cavailableLitres, waterlevelAt, cwaterlevelAt; 
float consumedLitres, cconsumedLitres;
int isSlow, isShigh, isClow, isChigh;
int isSlowNotify, isShighNotify, isClowNotify, isChighNotify;

void setup() {
  setupConfiguration = "";
  setupConfiguration = "Blynk v" BLYNK_VERSION ": Device started";

  Serial.begin(115200);
  delay(10);
  serialPort.begin(115200);
  delay(10);

  Serial.print("----------- Setup... ----------");
 
  setupWifi();
  setupTimers();
    
  Serial.println("Setting Blynk & ThingSpeak..");
  Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);

  setupDateTime();
  Serial.print("----------- END Setup... ----------");
}

void setupWifi() {
  WiFi.begin(ssid, pass);             // Connect to the network
  Serial.print("Connecting to ");
  setupConfiguration = setupConfiguration + "Connecting to " + ssid;
  terminal.print(ssid);
  
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  Serial.println("Checking Wifi Status..");
  Serial.println(WiFi.status());

//WL_NO_SHIELD = 255,
//WL_IDLE_STATUS = 0,
//WL_NO_SSID_AVAIL = 1
//WL_SCAN_COMPLETED = 2
//WL_CONNECTED = 3
//WL_CONNECT_FAILED = 4
//WL_CONNECTION_LOST = 5
//WL_DISCONNECTED = 6

  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  setupConfiguration = setupConfiguration + "\n" + "Connection established!" + "IP address:\t" + WiFi.localIP().toString();
}

void setupDateTime() { 
  if(DateTime.isTimeValid()) {
    terminal.println("datetime is valid.. hence skipping..");
	terminal.println("Last Wifi Status.. " + WiFi.status());
    terminal.flush();
    return;
  }
    
   // setup this after wifi connected 
   // you can use custom timeZone,server and timeout 
   DateTime.setTimeZone(+5.30); 
   DateTime.setServer("asia.pool.ntp.org"); 
   //DateTime.begin(15 * 1000); 
   DateTime.begin(); 
   if (!DateTime.isTimeValid()) { 
     terminal.println("Failed to get time from server."); 
     terminal.flush();
   }
   
   //  DateTimeParts p = DateTime.getParts(); 
//  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d %ld %+05d\n", p.getYear(), 
//                 p.getMonth(), p.getMonthDay(), p.getHours(), p.getMinutes(), 
//                 p.getSeconds(), p.getTime(), p.getTimeZone()); 
//   Serial.println("--------------------"); 
   time_t t = DateTime.now(); 
//   Serial.println(DateFormatter::format("%Y/%m/%d %H:%M:%S", t)); 
//   Serial.println(DateFormatter::format("%x - %I:%M %p", t)); 
   String dateTimenow = DateFormatter::format("%F %I:%M%p.", t);
   Serial.println(dateTimenow); 
   
   terminal.println("Date Sync completed successfully!!" + dateTimenow); 
   terminal.flush();
 }

void setupTimers() {
  Serial.println("Resetting Timers..");         
  
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(1000L, uploadtoBlynk); // 1 second
  uploadThingSpeakTimer.setInterval(120000L, uploadToThingSpeak); // (120000 -- 2 minutes)
  
  notifyTimer.setInterval(900000L, notifyToApp); // 15 mins  
  systemTimer.setInterval(900000L, setupDateTime); // 15 mins 
}

void extractSensorData() {  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(serialPort);

  long uptimemls = millis();
  uptimesec = uptimemls/1000;
  
//  Serial.println("Esp uptime ");
//  Serial.println(uptimesec);

//  Serial.println("Check port available..");
//  Serial.println(serialPort.available());
  int portStatus = serialPort.available();
  serialPortStatus = portStatus;
  
  if (serialPort.available() > 0) {
    //Serial.println("received Data");
    distance = -100;
//    Serial.println("Didnt Receive Data");
//    Serial.println(portStatus);
    //Serial.println(serialPort.available());
  }
  else {
      time_t t = DateTime.now(); 
      lastDataReceivedTime = DateFormatter::format("%F %I:%M%p.", t);
  }
  
  if(root == JsonObject::invalid()) 
    return;

//  Serial.println("JSON Received and Parsed");
//  root.prettyPrintTo(Serial);
  
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

void notifyToApp() {
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

void uploadtoBlynk() {  
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
    
  //Bridge Transmit
  bridge.virtualWrite(V0, tankPercentage);
  bridge.virtualWrite(V10, ctankPercentage);
}

void uploadToThingSpeak() {
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
    Serial.println("Channel write successful.");
    thingspeakStatus = "Channel write successful.";
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
    thingspeakStatus = "Problem writing to channel. HTTP error code " + String(httpCode);
  }

  time_t t = DateTime.now(); 
  String dateTimenow = DateFormatter::format("%F %I:%M%p.", t);
  
  thingspeakStatus = thingspeakStatus + dateTimenow;
  terminal.println("Last Wifi Status.. " + WiFi.status());
  terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
  terminal.flush();
}

BLYNK_CONNECTED() {
  bridge.setAuthToken("ODbXgkyA-fZohqppkwa0qm8QusGnDXCa");
}

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V50)
{  
  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param.asStr()) {
    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } else if (String("lss") == param.asStr()) {
    terminal.println(serialPortStatus);
    terminal.println("---END of MSG--");  
  } else if ( String("lrd") == param.asStr()) {
    terminal.println(receivedJson); 
    terminal.println("---END of MSG--");  
  } else if (String("lst") == param.asStr()) {
    terminal.println(lastDataReceivedTime);
    terminal.println("---END of MSG--");  
  } else if (String("crd") == param.asStr()) {
    receivedJson = "";
    terminal.clear();
  } else if (String("sdata") == param.asStr()) {
    terminal.println(setupConfiguration);
    terminal.println("---END of MSG--"); 
  } else if (String("lts") == param.asStr()) {
    terminal.println(thingspeakStatus);
    terminal.println("---END of MSG--"); 
  } else if (String("lws") == param.asStr()) {
    terminal.println("last wifi status" + wifiStatus);
    terminal.println("---END of MSG--"); 
  } else {
    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
    terminal.clear();
  }

  // Ensure everything is sent
  terminal.flush();
}

void loop() {
  Blynk.run();
  
  //Initialize Timers.
  uploadBlynkTimer.run();
  uploadThingSpeakTimer.run();
  systemTimer.run();

  extractSensorData();
}
