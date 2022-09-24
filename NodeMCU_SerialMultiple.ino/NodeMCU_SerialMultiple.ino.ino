// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLe3Z3HbRn"
#define BLYNK_DEVICE_NAME "Main Tank Monitor"
#define BLYNK_FIRMWARE_VERSION        "0.1.2"
#define BLYNK_PRINT Serial 

#define APP_DEBUG

#include "BlynkEdgent.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "ThingSpeak.h"

#define BLYNK_PRINT Serial  

SoftwareSerial serialPort(D1,D0);
BlynkTimer uploadBlynkTimer;
BlynkTimer uploadThingSpeakTimer;
BlynkTimer notifyTimer;
BlynkTimer systemTimer;

WidgetBridge bridge(V0);
WiFiClient client;
WidgetTerminal terminal(V50);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";

char auth[] = "3S2mjm0uyjmgkmZ_WXi3L3TgFEWz6b1E";

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

String setupConfiguration = "---";
String serialPortStatus = "----";
String thingspeakStatus = "----";
String receivedJson = "RECEIVED JSON";
String lastDataReceivedTime = "";
String wifiStatus = "";

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

String currentDate;
long systemUptime, uptimesec;
long distance, cdistance, mdistance, lastDistance;;
int tankPercentage, ctankPercentage, mtankPercentage;
float availableLitres, cavailableLitres, mavailableLitres, waterlevelAt, cwaterlevelAt, mwaterlevelAt; 
float consumedLitres, cconsumedLitres, mconsumedLitres;
int isSlow, isShigh, isClow, isChigh, isMlow, isMhigh;
int isSlowNotify, isShighNotify, isClowNotify, isChighNotify, isMlowNotify, isMhighNotify;

void setup() {
  BlynkEdgent.begin();
  setupConfiguration = "";
  setupConfiguration = "Blynk v" BLYNK_VERSION ": Device started";

  Serial.begin(9600);
  Serial.begin(115200);
  delay(10);
  serialPort.begin(115200);
  delay(10);

  Serial.println("----------------SETUP INITIATED--------------------------");
  
  //setupWifi();
  setupTimers();

  printDeviceBanner();
    
  Serial.println("Setting ThingSpeak..");
  //Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  delay(200);
  Serial.println("----------- END Setup... ----------");
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
  timeClient.begin();
  timeClient.setTimeOffset(19764);
  timeClient.update();

  String formattedTime = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  //Serial.print("Epoch Time: ");
  //Serial.println(epochTime);

  //Serial.print("Formatted Time: ");
  //Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  //Serial.print("Hour: ");
  //Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  //Serial.print("Minutes: ");
  //Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  //Serial.print("Seconds: ");
  //Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  //Serial.print("Week Day: ");
  //Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  //Serial.print("Month day: ");
  //Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  //Serial.print("Month: ");
  //Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  //Serial.print("Month name: ");
  //Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  //Serial.print("Year: ");
  //Serial.println(currentYear);

  //Print complete date:
  currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) + ":" + formattedTime;
  //Serial.print("Current date: ");
  //Serial.println(currentDate);

  //Serial.println("");
}

void setupTimers() {
  Serial.println("Resetting Timers..");         
  
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(10000L, uploadtoBlynk); // 1 second
  uploadThingSpeakTimer.setInterval(120000L, uploadToThingSpeak); // (120000 -- 2 minutes)
  
  notifyTimer.setInterval(900000L, notifyToApp); // 15 mins  
  systemTimer.setInterval(900000L, setupDateTime); // 15 mins 
}

void simulateSensor(){
  tankPercentage = 35;
  distance = 43;
  consumedLitres = 255;
  
  availableLitres = 566;
  waterlevelAt = 93;

  ctankPercentage= 34;
  cdistance = 53;
  cconsumedLitres = 2344;
    
  cavailableLitres = 2000;
  cwaterlevelAt = 44;
  mtankPercentage = 90;
  mdistance = 98;
  mconsumedLitres = 222;
    
  mavailableLitres = 433;
  mwaterlevelAt = 23;
}

void extractSensorData() {  
  Serial.print(".");
  Serial.println(WiFi.status());

  //TODO: Comment this piece in production code.
  //simulateSensor();
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(serialPort);

  long uptimemls = millis();
  uptimesec = uptimemls/1000;
  
  //Serial.println("Esp uptime ");
  //Serial.println(uptimesec);

//  Serial.println("Check port available..");
//  Serial.println(serialPort.available());
  int portStatus = serialPort.available();
  serialPortStatus = portStatus;
  
  if (serialPort.available() > 0) {
      //distance = -100;
      distance = lastDistance;
      Serial.println("Didnt Receive Data");
      //Serial.println(portStatus);
    //Serial.println(serialPort.available());
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

  if(isMlowNotify == 0 && isMlow == 1) {
     Blynk.notify("Mini Cement Tank is Empty!! Please switch On Motor.");
    isMlowNotify = 1;
  }
  else if(isMlow == 0) {
    isMlowNotify = 0;
  }

  if(isMhighNotify == 0 && isMhigh == 1) {
     Blynk.notify("Mini Cement Tank is Full!! Please switch Off Motor.");
    isMhighNotify = 1;
  }
  else if(isMhigh == 0) {
    isMhighNotify = 0;
  }
}

void uploadtoBlynk() { 
  Blynk.virtualWrite(V0, tankPercentage);
  Blynk.virtualWrite(V1, distance);
  Blynk.virtualWrite(V2, consumedLitres);
  
  Blynk.virtualWrite(V3, availableLitres);
  Blynk.virtualWrite(V4, waterlevelAt);
  
  Blynk.virtualWrite(V5, currentDate);  

  Blynk.virtualWrite(V6, uptimesec);

  Blynk.virtualWrite(V10, ctankPercentage);
  Blynk.virtualWrite(V11, cdistance);
  Blynk.virtualWrite(V12, cconsumedLitres);
  
  Blynk.virtualWrite(V13, cavailableLitres);
  Blynk.virtualWrite(V14, cwaterlevelAt);

  Blynk.virtualWrite(V15, mtankPercentage);
  Blynk.virtualWrite(V16, mdistance);
  Blynk.virtualWrite(V17, mconsumedLitres);
  
  Blynk.virtualWrite(V18, mavailableLitres);
  Blynk.virtualWrite(V19, mwaterlevelAt);
    
  //Bridge Transmit
  bridge.virtualWrite(V0, tankPercentage);
  bridge.virtualWrite(V10, ctankPercentage);
}

void uploadToThingSpeak() {
  wifiStatus = WiFi.status();
  
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

  thingspeakStatus = thingspeakStatus + currentDate;
  terminal.println("Last Wifi Status.. " + wifiStatus);
  terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
  terminal.flush();
}

BLYNK_CONNECTED() {
  bridge.setAuthToken("ODbXgkyA-fZohqppkwa0qm8QusGnDXCa");
  Blynk.email("{DEVICE_NAME} Successfully Connected", "{DEVICE_NAME} Connected");
  Blynk.notify("{DEVICE_NAME} Successfully Connected");
}

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V50)
{  
  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("help") == param.asStr()) {
    terminal.println("You said: 'help'") ;
    terminal.println("I said: 'printing all help commands'") ;
    terminal.println("lss-serial port status, lrd-lastreceivedjson, lst-lastreceivedtime, crd-clear, sdata-setupconfiguration, lws-lastwifistatus, ssys-setupdatetime, sys-systemtime") ;
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
  } else if (String("ssys") == param.asStr()) {
    setupDateTime();
  } else if (String("sys") == param.asStr()) {  
    terminal.println("System Time.." + currentDate);
    terminal.println("---END of MSG--");
  } else if (String("echck") == param.asStr()) {
    terminal.println("Email Check Triggered.." + currentDate);
    Blynk.email("Email Check", "This is the Test Email Check from Terminal Window");
    Blynk.logEvent("email_sent"); 
    terminal.println("Email Sent Successfully.." + currentDate);
  }
  else {
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
  BlynkEdgent.run();
  
  //Initialize Timers.
  uploadBlynkTimer.run();
  uploadThingSpeakTimer.run();
  systemTimer.run();
  notifyTimer.run();

  extractSensorData();
  setupDateTime();
}
