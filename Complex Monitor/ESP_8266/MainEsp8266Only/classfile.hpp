#include <ESP8266HTTPClient.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//
long uptimemls = 0;
float stankheight = 117; //cms
long scalibrationvalue = 33;//33;
long ssensorrestorecalibration;

float stankwidth = 153.0; //5 feet
float stanklength = 153.0; //4.5 feet

float ctankheight = 76.2; //4 feet
long ccalibrationvalue = 5;
long csensorrestorecalibration;
float ctankwidth = 132.08; //5 feet
float ctanklength = 304.8; //4.5 feet

float mtankheight = 74; //4 feet
long mcalibrationvalue = 5;
long msensorrestorecalibration;
float mtankwidth = 59; //5 feet
float mtanklength = 132; //4.5 feet
//

bool isSTankLowEmailSent = true;
bool isSTankFullEmailSent = true;
bool isCTankLowEmailSent = true;
bool isCTankFullEmailSent = true;

bool isThingPart1Complete = false;
bool isThingPart2Complete = false;

bool isBlynkPart1Complete = false;
bool isBlynkPart2Complete = false;

String currentDate;
long distance, cdistance, mdistance, lastDistance;;
int tankPercentage, ctankPercentage, mtankPercentage;
float availableLitres, cavailableLitres, mavailableLitres, waterlevelAt, cwaterlevelAt, mwaterlevelAt; 
float consumedLitres, cconsumedLitres, mconsumedLitres;
int isSlow, isShigh, isClow, isChigh, isMlow, isMhigh;
int isSlowNotify, isShighNotify, isClowNotify, isChighNotify, isMlowNotify, isMhighNotify;

String receivedJson = "RECEIVED JSON";
String lastDataReceivedTime = "";

long systemUptime, uptimesec;
unsigned long previousMillis = 0;

String wifiStatus;
String thingspeakStatus, blynkStatus = "----";
String setupConfiguration = "---";
String serialPortStatus = "----";
String wifiChecklog = "ZeroCHeck";

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

WidgetTerminal terminal(V50);

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
  terminal.println("Simulation Over" + currentDate);
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

void terminalCall(String param) {
  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param) {
    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } else if (String("lss") == param) {
    terminal.println(serialPortStatus);
    terminal.println("---END of MSG--");  
  } else if ( String("lrd") == param) {
    terminal.println(receivedJson); 
    terminal.println("---END of MSG--");  
  } else if (String("lst") == param) {
    terminal.println(lastDataReceivedTime);
    terminal.println("---END of MSG--");  
  } else if (String("crd") == param) {
    receivedJson = "";
    terminal.clear();
  } else if (String("sdata") == param) {
    terminal.println("Setup Configuration.." + setupConfiguration);
    terminal.println("---END of MSG--"); 
  } else if (String("lts") == param) {
    terminal.println(thingspeakStatus);
    terminal.println("---END of MSG--"); 
  } else if (String("lwc") == param) {
    terminal.println("wifi check" + wifiChecklog);
    terminal.println("---END of MSG--"); 
  } else if (String("lws") == param) {
    terminal.println("last wifi status" + wifiStatus);
    terminal.println("---END of MSG--"); 
  } else if (String("ssys") == param) {
    setupDateTime();
    terminal.println("Synced Systemtime" + currentDate);
  } else if (String("swifi") == param) {
    //setupWifi();
  } else if (String("sys") == param) {  
    terminal.println("System Time.." + currentDate);
    terminal.println("---END of MSG--");
  } else if (String("sconfig") == param) {  
    terminal.println("Setup Configuration.." + setupConfiguration);
    terminal.println("---END of MSG--");
  } else if (String("dev") == param) {  
    simulateSensor();
    terminal.println("---END of MSG-- Completed Run.." + currentDate);
  } else if (String("ssheet") == param) { 
    terminal.println("Sending data to Google Sheel..");
    terminal.println("---END of MSG--");
  } else if (String("help") == param) {   
    terminal.println("lws -- last wifi status");
    terminal.println("lts -- last thinkspeak status");
    terminal.println("lrd -- last received sensor data");
    terminal.println("sdata -- system configuration details");
    terminal.println("sys -- Get System Time");
    terminal.println("lst -- last received sensor data time");
    terminal.println("lwc -- wifi check log");
    terminal.println("sconfig -- Setup Configuration");
    terminal.println("ssheet -- Send Data to GoogleSheet");
    terminal.println("dev -- Run silent programs as per developer {Developer Usage Only}s");
    terminal.println("resetwifi -- Erases previously connected wifi and reboots the Wifi module in AP mode.");
    terminal.println("---END of MSG--");
  }
  else {
    // Send it back
    terminal.print("You said:");
    //terminal.write(param.getBuffer(), param.getLength());
    terminal.println(param);
    terminal.println();
    terminal.clear();
  }

  // Ensure everything is sent
  terminal.flush();
}

float measureWater(int distance, long calibrationvalue, float tankheight, float tankwidth, float tanklength) {
  float availablelitres = 0;

  int waterlevelat=0;
  if(distance > 0) {
    waterlevelat = tankheight + calibrationvalue - distance;
  }
  
  float availablevolume = waterlevelat * tanklength * tankwidth;
  availablelitres = availablevolume / 1000;
  
  if(availablelitres < 0){
    availablelitres = 0;
  }
  return availablelitres;
}

float consumedWater(int distance, long calibrationvalue, float tankheight, float tankwidth, float tanklength) {
  float consumedlitres = 0;
  float consumedvolume = (distance - calibrationvalue) * tanklength * tankwidth;
  consumedlitres = consumedvolume / 1000;

  if(consumedlitres < 0){
    consumedlitres = 0;
  }
  return consumedlitres;
}