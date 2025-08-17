#define strigger 2
#define secho 3
#define ctrigger 4
#define cecho 5
#define mtrigger 6
#define mecho 7

#define DEVICE_SOFTWARE "UNO_MAINTANK_08_03_2025{DD_MM_YYYY}"
#define BLYNK_FIRMWARE_VERSION "3.0.0"

#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Define RX and TX pins for SoftwareSerial on Arduino
// Connect Arduino's SoftwareSerial TX to ESP8266's RX
// Connect Arduino's SoftwareSerial RX to ESP8266's TX (through a voltage divider if necessary, as ESP8266 is 3.3V)
SoftwareSerial espSerial(2, 13); // RX, TX pins

float stankheight = 106; //cms
long scalibrationvalue = 6;//33;
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

int isSlow;
int isShigh;
int isClow;
int isChigh;
int isMlow;
int isMhigh;

void setup() {
  Serial.begin(9600); // Initialize hardware serial for debugging
  espSerial.begin(9600); // Initialize software serial for communication with ESP8266
  Serial.println("Arduino ready to send data.");
}

StaticJsonDocument<200> checkWaterLevelInCompressorTank(StaticJsonDocument<200> root) {
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
  distance = 50;
  
  Serial.println("Compressor duration");
  Serial.println(duration);

  Serial.println("Compressor distance");
  Serial.println(distance);
  
  //Blynk.virtualWrite(V1, distance);
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

  return root;
}

StaticJsonDocument<200> checkWaterLevelInCementTank(StaticJsonDocument<200> root) {
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
  distance = 50;
  
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
  return root;
}

StaticJsonDocument<200> checkWaterLevelInMiniTank(StaticJsonDocument<200> root) {
  long duration = 0, distance = 0;
  int tanklevelpercentage = 0;

  digitalWrite(mtrigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(mtrigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(mtrigger, LOW);
  duration = pulseIn(mecho, HIGH);
  
  distance = (duration/2) / 29.1;

  //simulator.
  distance = 50;
 
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
  return root;
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

void loop() {
  // put your main code here, to run repeatedly:
  StaticJsonDocument<200> doc; // Allocate a static JSON document

  long uptimesec = millis()/1000;
  doc["aurdinouptimesec"] = uptimesec; // Add another key-value pair
  doc = checkWaterLevelInCompressorTank(doc);
  doc = checkWaterLevelInCementTank(doc);
  doc = checkWaterLevelInMiniTank(doc);

  serializeJson(doc, espSerial); // Send JSON over Serial
  espSerial.println(); // Add a newline for easier parsing on ESP8266
  delay(2000);
}
