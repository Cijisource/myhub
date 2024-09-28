// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPL0tRLYzze"
#define BLYNK_TEMPLATE_NAME "Sintex Tank Monitor"
#define BLYNK_DEVICE_NAME "Sintex Tank Monitor"
#define BLYNK_FIRMWARE_VERSION        "0.1.1"
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

SoftwareSerial serialPort(D1,D0); //Rx and Tx

BlynkTimer uploadBlynkTimer;
BlynkTimer uploadThingSpeakTimer;
BlynkTimer notifyTimer;
BlynkTimer systemTimer;
BlynkTimer wifiChecker;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long myChannelNumber = 1184761;
const char * myWriteAPIKey = "Z85MB42QWY3T4VGG";

char auth[] = "ODbXgkyA-fZohqppkwa0qm8QusGnDXCa";

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "MOJITO";
//char pass[] = "cijaiz@123";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";

//char ssid[] = "Cijaiz complex";
//char pass[] = "9000150001";

char ssid[] = "jeimahil";
char pass[] = "mahilvis2017";

WiFiClient client;
WidgetTerminal terminal(V50);

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
long distance, cdistance;

int tankPercentage, ctankPercentage, compressorTankPercentage, cementTankPercentage;
float availableLitres, cavailableLitres; 
float consumedLitres, cconsumedLitres, waterlevelat;

bool isSTankLowEmailSent = true;
bool isSTankFullEmailSent = true;
bool isCTankLowEmailSent = true;
bool isCTankFullEmailSent = true;

int isSlow, isShigh;
int isSlowNotify, isShighNotify;

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
  isSTankLowEmailSent = false;
  isSTankFullEmailSent = false;
  isCTankLowEmailSent = false;
  isCTankFullEmailSent = false;

  //setupWifi();
  setupTimers();

  printDeviceBanner();
  
  Serial.println("Setting Blynk & ThingSpeak..");
  //Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  delay(200);  
  Serial.println("----------------SETUP COMPLETED--------------------------");
}

void setupTimers() {
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(10000L, uploadtoBlynk); // 1 second
  uploadThingSpeakTimer.setInterval(140000L, uploadToThingSpeak); // (120000 -- 2 minutes & 20 seconds)
  
  notifyTimer.setInterval(900000L, notifyToApp); // 15 mins  
  systemTimer.setInterval(900000L, setupDateTime); // 15 mins 
  wifiChecker.setInterval(900000L, setupWifi); // 15 mins 
}

void setupWifi() {
  String wifiChecklog = "Performing Wifi Check.. ..";
  Serial.print(wifiChecklog);
  Serial.print(WiFi.status());
  if (WiFi.status() == WL_CONNECTED) { // Skip since network connected..
    WiFi.begin(ssid, pass); // Connect to the network
    wifiChecklog = wifiChecklog + "Wifi Connection Exists.. Hence Skipping..";
    terminal.println(wifiChecklog);
    terminal.flush();
    return;
  }
  
  WiFi.begin(ssid, pass); // Connect to the network
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

  wifiChecklog = wifiChecklog + "Wifi connection lost.. Hence Reconnecting...";
  wifiChecklog = wifiChecklog + "\n" + "Connection established!";
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

  //Serial.println("");
}

void simulateSensor(){
  tankPercentage = 1;
  distance = 1;
  consumedLitres = 1;
  
  availableLitres = 1;
}

void ExtractSensorData() {
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

  int portStatus = serialPort.available();
  serialPortStatus = portStatus;
  
  if (serialPort.available() > 0) {
    Serial.println("received Data");
    distance = -100;
    //Serial.println("Didnt Receive Data");
  }    
  if(root == JsonObject::invalid()) 
    return;

  //Serial.println("JSON Received and Parsed");
  //root.prettyPrintTo(Serial);
  //Serial.println("");

  receivedJson = "";
  root.prettyPrintTo(receivedJson);

  systemUptime=root["ArduinoUptime"];
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

void notifyToApp() 
{
  if(compressorTankPercentage > 90 && isSTankFullEmailSent == false) {
     Blynk.email("Compressor Tank - Full", "Compressor Tank is Full");  
     isSTankFullEmailSent = true;

     terminal.println("Compressor Tank is Full");
     terminal.flush();
  }
  else if(compressorTankPercentage < 90 && compressorTankPercentage > 10) {
    isSTankFullEmailSent = false;
  }
  
  if (compressorTankPercentage < 25 && compressorTankPercentage > 10 && isSTankLowEmailSent == false) {
      Blynk.email("Compressor Tank - Empty", "Compressor Tank is Empty. Please Refill.");
      terminal.println("Tank is Empty. Please Refill");
      terminal.flush();
      isSTankLowEmailSent = true;
  }
  else if(compressorTankPercentage > 40) {
    isSTankLowEmailSent = false; 
  }
  
  if(cementTankPercentage > 95 && isCTankFullEmailSent == false) {
      Blynk.email("Cement Tank - Full", "Cement Tank is Full.");
      terminal.println("Cement Tank is Full.");
      terminal.flush();
      isCTankFullEmailSent = true;  
  }
  else if(cementTankPercentage > 20) {
    isCTankFullEmailSent = false;
  }
  
  if(cementTankPercentage < 25 && cementTankPercentage > 10 && isCTankLowEmailSent == false) {
      Blynk.email("Cement Tank - Empty", "Cement Tank is Empty. Please Refill.");
      isCTankLowEmailSent = true;
      terminal.println("Tank is Empty. Please Refill.");
      terminal.flush();
  }
  else if(cementTankPercentage > 40) {
    isCTankLowEmailSent = false;
  }
    
  if(isSlowNotify == 0 && isSlow == 1) {
    Blynk.notify("Sintex Tank is Empty!! Please switch On Motor.");
    Blynk.email("Sintex Tank - Empty", "Sintex Tank is Empty!! Please switch On Motor.");
    isSlowNotify = 1;

    terminal.println("Sintex Tank is Empty!! Please switch On Motor.");
    terminal.flush();
  }
  else if(isSlow == 0) {
    isSlowNotify = 0;
  }

  if(isShighNotify == 0 && isShigh == 1) {
    Blynk.notify("Sintex Tank is Full!! Please switch Off Motor.");
    Blynk.email("Sintex Tank - Full", "Sintex Tank is Full!! Please switch Off Motor.");
    isShighNotify = 1;

    terminal.println("Sintex Tank is Full!! Please switch Off Motor.");
    terminal.flush();
  }
  else if(isShigh == 0) {
    isShighNotify = 0;
  }
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
  //Serial.println(compressorTankPercentage);

  Blynk.virtualWrite(V8, cementTankPercentage);
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
  }

  thingspeakStatus = thingspeakStatus + currentDate;
  terminal.println("Thingspeak Upload Status.. " + thingspeakStatus);
  terminal.println("Last WIFI Status.. " + wifiStatus);
  terminal.flush();
}

BLYNK_CONNECTED(){
  Blynk.email("{DEVICE_NAME} Successfully Connected", "{DEVICE_NAME} Connected");
  Blynk.notify("{DEVICE_NAME} Successfully Connected");
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
  } else if (String("lws") == param.asStr()) {
    terminal.println("last wifi status" + wifiStatus);
    terminal.println("---END of MSG--"); 
  } else if (String("ssys") == param.asStr()) {
    setupDateTime();
  } else if (String("swifi") == param.asStr()) {
    setupWifi();
  } else if (String("sys") == param.asStr()) {  
    terminal.println("System Time.." + currentDate);
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

void loop() {
  BlynkEdgent.run();

  // Initiates SimpleTimer
  uploadBlynkTimer.run(); 
  uploadThingSpeakTimer.run();
  notifyTimer.run();
  systemTimer.run();
  wifiChecker.run();

  ExtractSensorData();
  setupDateTime();
}
