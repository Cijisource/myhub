/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io 
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */

#define BLYNK_TEMPLATE_ID "TMPL3cHgxG7q_"
#define BLYNK_TEMPLATE_NAME "Research"
#define DEVICE_NAME "Sintex Tank Monitor"
#define DEVICE_SOFTWARE "ESP_SINTEX_24_11_2023{DD_MM_YYYY}"
#define BLYNK_FIRMWARE_VERSION "0.0.1"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

//#define BLYNK_PRINT Serial 

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
//#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "ThingSpeak.h"

SoftwareSerial serialPort(D1,D0); //Rx and Tx

BlynkTimer uploadBlynkTimer;
BlynkTimer uploadThingSpeakTimer;
BlynkTimer extractSensorTimer;
BlynkTimer systemTimer;
BlynkTimer wifiChecker;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

String GAS_ID = "AKfycbzrMQ_C1NPbQYd6tDzF8nh_Fu2JIuy06ZIzjk6pJbsxeQPLGdAVolq0pKQZNuJ8OFyg";

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";

char auth[] = "JV3j41e355JF15E-0VBih2ucZ55KiEGn";

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "MOJITO";
//char pass[] = "cijaiz@123";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";

//char ssid[] = "Cijaiz complex";
//char pass[] = "9000150001";

//char ssid[] = "jeimahil";
//char pass[] = "mahilvis2017";

char ssid[] = "Cijaiz_Zone0";
char pass[] = "Bb00m5050";

WiFiClient client;
WiFiClientSecure eclient; //--> Create a WiFiClientSecure object.
WidgetTerminal terminal(V50);

String setupConfiguration = "---";
String serialPortStatus = "----";
String thingspeakStatus = "----";
String receivedJson = "RECEIVED JSON";
String lastDataReceivedTime = "";
String wifiStatus = "";
String wifiChecklog = "ZeroCHeck";

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

String currentDate;
long systemUptime, uptimesec;
long distance, cdistance;

int tankPercentage, ctankPercentage, compressorTankPercentage, cementTankPercentage;
float availableLitres, cavailableLitres; 
float consumedLitres, cconsumedLitres, waterlevelat;

bool isThingPart1Complete = false;
bool isThingPart2Complete = false;
bool isCTankLowEmailSent = true;
bool isCTankFullEmailSent = true;

int isSlow, isShigh;
int isSlowNotify, isShighNotify;

void setup()
{
  setupWifi();
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  BlynkEdgent.begin();
  setupConfiguration = "";
  setupConfiguration = "Blynk v" BLYNK_VERSION ": Device started " DEVICE_SOFTWARE;

  Serial.begin(9600);
  Serial.begin(115200);
  delay(10);
  serialPort.begin(115200);
  delay(10);
  
  Serial.println("----------------SETUP INITIATED--------------------------");

  setupDateTime();
  setupTimers();
  printDeviceBanner();
  
  Serial.println("Setting Blynk & ThingSpeak..");
  //Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  delay(200);  
  Serial.println("----------------SETUP COMPLETED--------------------------");

  eclient.setInsecure();
}

void setupTimers() {
  // Setup a function to be called every second
  //uploadBlynkTimer.setInterval(10000L, uploadtoBlynk); // 10 second
  //uploadBlynkTimer.setInterval(10000L, dd);// 10 second
  //uploadThingSpeakTimer.setInterval(60000L, uploadToThingSpeak1); // (120000 -- 2 minutes & 20 seconds)
  //uploadThingSpeakTimer.setInterval(120000L, uploadToThingSpeak2); // (120000 -- 2 minutes & 20 seconds)
  
  //extractSensorTimer.setInterval(1000L, ExtractSensorData); // 1 secoond  
  systemTimer.setInterval(1000L, setupDateTime); // 1 secoond  
  wifiChecker.setInterval(120000, setupWifi); // 30 mins 900000L
}

void setupWifi() {
  bool result = Blynk.connected();
  if(result == false){
    //Blynk.logEvent("attentionrequired", String("Device Lost Server connection") + DEVICE_NAME);
    wifiChecklog = String("Device Lost Server connection") + DEVICE_NAME + currentDate;
  }
  
  wifiChecklog = ("Performing Wifi Check.. .." + WiFi.status());
  Serial.print(wifiChecklog);
  Serial.print(WiFi.status());
  if (WiFi.status() == WL_CONNECTED) { // Skip since network connected..
    wifiChecklog = (wifiChecklog + "Wifi Connection Exists.. Hence Skipping.. " + WiFi.SSID() + WiFi.localIP().toString() + currentDate);
    //Blynk.logEvent("forinformation", wifiChecklog);
    terminal.println(wifiChecklog);
    terminal.flush();
    return;
  }
  
  WiFi.begin(ssid, pass); // Connect to the network
  Serial.print("Connecting to ");
  setupConfiguration = setupConfiguration + "Connecting to " + WiFi.SSID();

  //Re connect blynkedgent..
  //Blynk.connect();
  //BlynkEdgent.begin();
  
  Blynk.logEvent("checkrequired", "ReConnecting to WIFI" + setupConfiguration);
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

  wifiChecklog = setupConfiguration + wifiChecklog + "Wifi connection lost.. Hence Reconnecting...";
  wifiChecklog = wifiChecklog + "\n" + "Connection established!" + currentDate;
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  setupConfiguration = setupConfiguration + "\n" + "Connection established!" + "IP address:\t" + WiFi.localIP().toString();
  
  terminal.println(wifiChecklog);
  terminal.flush();
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

  //Serial.println(currentDate);
}

void simulateSensor(){
  char comma= ',';
  String logValue;

  tankPercentage = 1;
  distance = 1;
  consumedLitres = 1;

  systemUptime = millis()/1000;
  uptimesec = millis()/1000;

  waterlevelat = 20;
  compressorTankPercentage = 30;
  cementTankPercentage = 35;
  
  availableLitres = 1;
  logValue =(String(tankPercentage)+ comma + String(distance) + comma + String(consumedLitres));
  terminal.println(logValue);
}

bool BlynkUploadPart1() {
  //CustomDelay(5000, 1);
  Serial.println("called walkone3.." + currentDate);

  Blynk.virtualWrite(V4, tankPercentage);
  Blynk.virtualWrite(V1, distance);
  Blynk.virtualWrite(V2, consumedLitres);
  Blynk.virtualWrite(V5, systemUptime); 
  Serial.println(systemUptime);
  return true;
}

bool BlynkUploadPart2() {
  //CustomDelay(5000, 1);
  Serial.println("called walkone7.." + currentDate);

  Blynk.virtualWrite(V3, availableLitres);
  Blynk.virtualWrite(V6, uptimesec);
  Blynk.virtualWrite(V9, waterlevelat);

  Blynk.virtualWrite(V7, compressorTankPercentage);
  //Serial.println(compressorTankPercentage);

  Blynk.virtualWrite(V8, cementTankPercentage);
  return true;
}

void ExtractSensorData() {
  //Serial.print(".");
  //Serial.println(WiFi.status());

  //TODO: Comment this piece in production code.
  simulateSensor();
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(serialPort);

  //long uptimemls = millis();
  //uptimesec = uptimemls/1000;
  
  //Serial.println("Esp uptime ");
  //Serial.println(uptimesec);

  int portStatus = serialPort.available();
  serialPortStatus = portStatus;
  
  if (serialPort.available() > 0) {
    Serial.println("received Data");
    distance = -100;
    //Serial.println("Didnt Receive Data");
  }    
  if(root == JsonObject::invalid()) 
    return;

  Serial.println("JSON Received and Parsed");
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

void uploadtoBlynk(){
  //Blynk.virtualWrite(V4, tankPercentage);
  //Blynk.virtualWrite(V1, distance);
  //Blynk.virtualWrite(V2, consumedLitres);
  //Blynk.virtualWrite(V3, availableLitres);
  
  //Blynk.virtualWrite(V5, systemUptime);  
  Blynk.virtualWrite(V6, uptimesec);
  //Blynk.virtualWrite(V9, waterlevelat);

  //Blynk.virtualWrite(V7, compressorTankPercentage);
  //Serial.println(compressorTankPercentage);

  //Blynk.virtualWrite(V8, cementTankPercentage);
  //Serial.println(cementTankPercentage);
}

void uploadToThingSpeak1()
{ 
    Serial.println("1");
    if(isThingPart1Complete == false){
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
          Blynk.logEvent("email_sent", String(httpCode) + "--" + thingspeakStatus);
        }
      
        thingspeakStatus = thingspeakStatus + currentDate;
        terminal.println("Last Wifi Status.. " + wifiStatus);
        terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
        terminal.flush();
  
    isThingPart1Complete = true;
    isThingPart2Complete = false;  
  }
}

void uploadToThingSpeak2()
{ 
  Serial.println("2");
  if(isThingPart2Complete == false){
      Serial.println("Upload Call from Thing2 -- " + currentDate);

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
  
     isThingPart1Complete = false;
     isThingPart2Complete = true;  
  }
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

BLYNK_CONNECTED(){
  //Blynk.email("{DEVICE_NAME} Successfully Connected", "{DEVICE_NAME} Connected");
  Blynk.logEvent("forinformation", String("Successfully Connected") + DEVICE_NAME);
}

BLYNK_WRITE(V0) {
  compressorTankPercentage = param.asInt();
}

BLYNK_WRITE(V10) {
  cementTankPercentage = param.asInt();
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
  } else if (String("lwc") == param.asStr()) {
    terminal.println("wifi check" + wifiChecklog);
    terminal.println("---END of MSG--"); 
  } else if (String("lws") == param.asStr()) {
    terminal.println("last wifi status" + wifiStatus);
    terminal.println("---END of MSG--"); 
  } else if (String("ssys") == param.asStr()) {
    setupDateTime();
    terminal.println("Synced Systemtime" + currentDate);
  } else if (String("swifi") == param.asStr()) {
    setupWifi();
  } else if (String("sys") == param.asStr()) {  
    terminal.println("System Time.." + currentDate);
    terminal.println("---END of MSG--");
  } else if (String("sconfig") == param.asStr()) {  
    terminal.println("Setup Configuration.." + setupConfiguration);
    terminal.println("---END of MSG--");
  } else if (String("dev") == param.asStr()) {  
    simulateSensor();
    terminal.println("---END of MSG-- Completed Run.." + currentDate);
  } else if (String("ssheet") == param.asStr()) { 
    terminal.println("Sending data to Google Sheel..");
    sendData(20, 20);
    terminal.println("---END of MSG--");
  } else if (String("help") == param.asStr()) {  
    terminal.println("lws -- last wifi status");
    terminal.println("lts -- last thinkspeak status");
    terminal.println("lrd -- last received sensor data");
    terminal.println("sys -- Get System Time");
    terminal.println("lst -- last received sensor data time");
    terminal.println("lwc -- wifi check log");
    terminal.println("sconfig -- Setup Configuration");
    terminal.println("ssheet -- Send Data to GoogleSheet");
    terminal.println("dev -- Run silent programs as per developer {Developer Usage Only}s");
    terminal.println("---END of MSG--");
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

// Subroutine for sending data to Google Sheets
void sendData(float tem, int hum) {
  terminal.println("==========");
  terminal.print("connecting to ");
  terminal.println(host);
  
  //----------------------------------------Connect to Google host
  if (!eclient.connect(host, httpsPort)) {
    terminal.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String water_level =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?WaterLevel=" + water_level;
  terminal.print("requesting URL: ");
  terminal.println(url);

  eclient.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  terminal.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (eclient.connected()) {
    String line = eclient.readStringUntil('\n');
    if (line == "\r") {
      terminal.println("headers received");
      break;
    }
  }
  String line = eclient.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    terminal.println("esp8266/Arduino CI successfull!");
  } else {
    terminal.println("esp8266/Arduino CI has failed");
  }
  terminal.print("reply was : ");
  terminal.println(line);
  terminal.println("closing connection");
  terminal.println("==========");
  terminal.println();
  //----------------------------------------
} 

void loop() {
  BlynkEdgent.run();

  // Initiates SimpleTimer
  uploadBlynkTimer.run(); 
  uploadThingSpeakTimer.run();
  //extractSensorTimer.run();
  systemTimer.run();
  wifiChecker.run();

  ExtractSensorData();
  //setupDateTime();
}
