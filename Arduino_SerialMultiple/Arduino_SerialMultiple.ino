#define strigger 7
#define secho 6
#define ctrigger 5
#define cecho 4

#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial serialPort(9,10); //Rx and Tx

float stankheight = 103.0; //cms
long scalibrationvalue = 34;
long ssensorrestorecalibration;
float stankwidth = 153.0; //5 feet
float stanklength = 153.0; //4.5 feet

float ctankheight = 76.2; //4 feet
long ccalibrationvalue = 5;
long csensorrestorecalibration;
float ctankwidth = 132.08; //5 feet
float ctanklength = 304.8; //4.5 feet
int isSlow;
int isShigh;
int isClow;
int isChigh;

void setup() {
  // put your setup code here, to run once:
  isSlow = false;
  isShigh = false;
  isClow = false;
  isChigh = false;
  
  serialPort.begin(115200);
  Serial.begin(115200);
  Serial.println("----------------SETUP INITIATED--------------------------");
  pinMode(strigger, OUTPUT);
  pinMode(secho, INPUT);
  pinMode(ctrigger, OUTPUT);
  pinMode(cecho, INPUT);
  ssensorrestorecalibration = scalibrationvalue;
  csensorrestorecalibration = ccalibrationvalue;
  
  Serial.println("----------------SETUP COMPLETED--------------------------");
}
  
void loop() {
  // put your main code here, to run repeatedly:
  long uptimemls = millis();
  long uptimesec = uptimemls/1000;
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ArduinoUptime"] = uptimesec;
  
  checkWaterLevelInCompressorTank(root);
  checkWaterLevelInCementTank(root);
  
//  if(serialPort.available()>0)
//  {
    root.printTo(serialPort);
//  }
  delay(1000);
}

void checkWaterLevelInCompressorTank(JsonObject& root) {
  long duration, distance;
  int tanklevelpercentage = 0;

  digitalWrite(strigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(strigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(strigger, LOW);
  duration = pulseIn(secho, HIGH);
  distance = (duration/2) / 29.1;
  
  //Serial.println("Compressor duration");
  //Serial.println(duration);

//  Serial.println("Compressor distance");
//  Serial.println(distance);
  
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
    waterlevelat = stankheight + scalibrationvalue - distance;
//    Serial.println(scalibrationvalue);
//    Serial.println("calibration value printed above");
  }
//  Serial.println("Waterlevelat");
//  Serial.println(waterlevelat);
  
  tanklevelpercentage = waterlevelat / stankheight * 100;

  if(tanklevelpercentage < 0)  {
    tanklevelpercentage = 0;
  }
  
  if(tanklevelpercentage < 20 && tanklevelpercentage > 10) {
    isSlow = 1;
  }
  else if(tanklevelpercentage > 20) {
    isSlow = 0;
  }
  
  if(tanklevelpercentage > 95) {
    isShigh = 1;
  }
  else if(tanklevelpercentage < 95) {
    isShigh = 0;
  }
  
  root["isSlow"] = isSlow;
  root["isShigh"] = isShigh;
  
  //Blynk.virtualWrite(V0, tanklevelpercentage);
  root["TankLevelPercentage"] = tanklevelpercentage;
  
  root["SWaterlevelat"] = waterlevelat/30.48;
  //Blynk.virtualWrite(V5, uptimesec);
}

  void checkWaterLevelInCementTank(JsonObject& root) {
  long duration, distance;
  int tanklevelpercentage = 0;

  digitalWrite(ctrigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(ctrigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(ctrigger, LOW);
  duration = pulseIn(cecho, HIGH);
  distance = (duration/2) / 29.1;
  
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
    
  if(tanklevelpercentage < 20 && tanklevelpercentage > 10) {
    isClow = 1;
  }
  else {
    isClow = 0;
  }
  
  if(tanklevelpercentage > 95) {
    isChigh = 1;
  }
  else {
    isChigh = 0;
  }
  
  //Blynk.virtualWrite(V0, tanklevelpercentage);
  root["isClow"] = isClow;
  root["isChigh"] = isChigh;
    
  root["CTankLevelPercentage"] = tanklevelpercentage;
  root["CWaterlevelat"] = waterlevelat/30.48;
  //Blynk.virtualWrite(V5, uptimesec);
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
