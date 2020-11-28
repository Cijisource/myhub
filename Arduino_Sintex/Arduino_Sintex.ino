#define strigger 8
#define secho 9

#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial serialPort(11,10); //Rx and Tx

float stankheight = 111; //4 feet
float scalibrationvalue = 13;
float ssensorrestorecalibration;
float stankwidth = 95; //5 feet
float stanklength = 95; //4.5 feet

int isSlow;
int isShigh;
int isClow;
int isChigh;

void setup() {
  // put your setup code here, to run once:

  serialPort.begin(115200);
  Serial.begin(115200);
  Serial.println("----------------SETUP INITIATED--------------------------");
  
  pinMode(strigger, OUTPUT);
  pinMode(secho, INPUT);
  ssensorrestorecalibration = scalibrationvalue; 
   
  Serial.println("----------------SETUP COMPLETED--------------------------");
}
  
void loop() {
  // put your main code here, to run repeatedly:
  long uptimemls = millis();
  long uptimesec = uptimemls/1000;
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ArduinoUptime"] = uptimesec;
  
  checkWaterLevelInSintexTank(root);
  root.printTo(serialPort);
  
  delay(1000);
}

void checkWaterLevelInSintexTank(JsonObject& root) {
  long duration;
  int distance = 0;
  float tanklevelpercentage = 0;

  digitalWrite(strigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(strigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(strigger, LOW);
  duration = pulseIn(secho, HIGH);
  distance = (duration/2) / 29.1;
  
//  Serial.println("Sintex duration");
//  Serial.println(duration);
//
//  Serial.println("Sintex distance");
//  Serial.println(distance);
//  
  //Blynk.virtualWrite(V1, distance);
  root["SSensorDistance"] = distance;
  
//  Serial.println(distance);
  scalibrationvalue = ssensorrestorecalibration;
  if(distance > (stankheight + scalibrationvalue)) {
      distance = stankheight;
    scalibrationvalue = 0;
  } 
  float availablelitres = measureWater(distance, scalibrationvalue, stankheight, stankwidth, stanklength);
  float consumedlitres = consumedWater(distance, scalibrationvalue, stankheight, stankwidth, stanklength);

//  Serial.println(availablelitres);
//  Serial.println(consumedlitres);
  
  //Blynk.virtualWrite(V13, availablelitres);
  root["SAvailableLitres"] = availablelitres;
  
  //Blynk.virtualWrite(V12, consumedlitres);
  root["SConsumedLitres"] = consumedlitres;

  float waterlevelat = 0;

  if(distance > 0) {
    waterlevelat = stankheight + scalibrationvalue - distance;
//    Serial.println(scalibrationvalue);
//    Serial.println("calibration value printed above");
  }
//  Serial.println("SWaterlevel");
//  Serial.println(waterlevelat);
//  Serial.println("tankheight");
//  Serial.println(stankheight);
//  waterlevelat = 44;
//  stankheight = 44;
  tanklevelpercentage = waterlevelat / stankheight * 100;
//  Serial.println("percents");
//  Serial.println(tanklevelpercentage);
  
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
  
  root["STankLevelPercentage"] = (int)tanklevelpercentage;
  root["SWaterLevelAt"] = waterlevelat;
  root["SWaterLevel"] = waterlevelat/30.48;

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
