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
WidgetTerminal terminal(V50);

String setupConfiguration = "---";
String serialPortStatus = "----";
String thingspeakStatus = "----";
String receivedJson = "RECEIVED JSON";
String lastDataReceivedTime = "";
String wifiStatus = "";

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
  setupConfiguration = "";
  setupConfiguration = "Blynk v" BLYNK_VERSION ": Device started";
  
  Serial.begin(115200);
  delay(10);
  serialPort.begin(115200);
  delay(10);
  
  Serial.println("----------------SETUP INITIATED--------------------------");
  isSTankLowEmailSent = false;
  isSTankFullEmailSent = false;
  isCTankLowEmailSent = false;
  isCTankFullEmailSent = false;

  setupWifi();
  setupTimers();
  
  Serial.println("Setting Blynk & ThingSpeak..");
  Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  setupDateTime();
  
  Serial.println("----------------SETUP COMPLETED--------------------------");
}

void setupTimers() {
  // Setup a function to be called every second
  uploadBlynkTimer.setInterval(1000L, uploadtoBlynk); // 1 second
  uploadThingSpeakTimer.setInterval(140000L, uploadToThingSpeak); // (120000 -- 2 minutes & 20 seconds)
  
  notifyTimer.setInterval(900000L, notifyToApp); // 15 mins  
  systemTimer.setInterval(900000L, setupDateTime); // 15 mins 
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
    terminal.println("Last Wifi Status.. " + wifiStatus);
    terminal.flush();
    return;
  }
    
   // setup this after wifi connected 
   // you can use custom timeZone,server and timeout 
   DateTime.setTimeZone(+5.30); 
   DateTime.setServer("asia.pool.ntp.org"); 
   //DateTime.begin(3000 * 1000); 
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
 
void ExtractSensorData() {
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(serialPort);

  long uptimemls = millis();
  uptimesec = uptimemls/1000;
  
  //Serial.println("Esp uptime ");
  //Serial.println(uptimesec);

  int portStatus = serialPort.available();
  serialPortStatus = portStatus;
  
  if (serialPort.available() > 0) {
    //Serial.println("received Data");
    distance = -100;
//    Serial.println("Didnt Receive Data");
  }  
  else {
      time_t t = DateTime.now(); 
      lastDataReceivedTime = DateFormatter::format("%F %I:%M%p.", t);
      wifiStatus = WiFi.status();
  }
  
  if(root == JsonObject::invalid()) 
    return;

//  Serial.println("JSON Received and Parsed");
//  root.prettyPrintTo(Serial);
//  Serial.println("");

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
     Blynk.email("Compressor Tank", "Compressor Tank is Full");  
     isSTankFullEmailSent = true;

     terminal.println("Compressor Tank is Full");
     terminal.flush();
  }
  else if(compressorTankPercentage < 90 && compressorTankPercentage > 10) {
    isSTankFullEmailSent = false;
  }
  
  if (compressorTankPercentage < 40 && compressorTankPercentage > 10 && isSTankLowEmailSent == false) {
      Blynk.email("Compressor Tank", "Quarter Level reached. Please Refill.");
      terminal.println("Quarter Level reached. Please Refill.");
      terminal.flush();
      isSTankLowEmailSent = true;
  }
  else if(compressorTankPercentage > 40) {
    isSTankLowEmailSent = false; 
  }
  
  if(cementTankPercentage > 95 && isCTankFullEmailSent == false) {
      Blynk.email("Cement Tank", "Cement Tank is Full.");
      terminal.println("Cement Tank is Full.");
      terminal.flush();
      isCTankFullEmailSent = true;  
  }
  else if(cementTankPercentage > 20) {
    isCTankFullEmailSent = false;
  }
  
  if(cementTankPercentage < 40 && cementTankPercentage > 10 && isCTankLowEmailSent == false) {
      Blynk.email("Cement Tank", "Quarter Level reached. Please Refill.");
      isCTankLowEmailSent = true;
      terminal.println("Quarter Level reached. Please Refill.");
      terminal.flush();
  }
  else if(cementTankPercentage > 40) {
    isCTankLowEmailSent = false;
  }
    
  if(isSlowNotify == 0 && isSlow == 1) {
    Blynk.notify("Sintex Tank is Empty!! Please switch On Motor.");
    Blynk.email("Sintex Tank", "Sintex Tank is Empty!! Please switch On Motor.");
    isSlowNotify = 1;

    terminal.println("Sintex Tank is Empty!! Please switch On Motor.");
    terminal.flush();
  }
  else if(isSlow == 0) {
    isSlowNotify = 0;
  }

  if(isShighNotify == 0 && isShigh == 1) {
    Blynk.notify("Sintex Tank is Full!! Please switch Off Motor.");
    Blynk.email("Sintex Tank", "Sintex Tank is Full!! Please switch Off Motor.");
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

  time_t t = DateTime.now(); 
  String dateTimenow = DateFormatter::format("%F %I:%M%p.", t);
  
  thingspeakStatus = thingspeakStatus + dateTimenow;
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
  } else if (String("sys") == param.asStr()) {
    time_t t = DateTime.now(); 
    String dateTimenow = DateFormatter::format("%F %I:%M%p.", t);
  
    terminal.println("System Time.." + dateTimenow);
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
  Blynk.run();

  // Initiates SimpleTimer
  uploadBlynkTimer.run(); 
  uploadThingSpeakTimer.run();
  notifyTimer.run();
  systemTimer.run();

  ExtractSensorData();
}
