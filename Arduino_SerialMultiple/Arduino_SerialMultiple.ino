#define trigger 7
#define echo 6

#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial serialPort(9,10); //Rx and Tx

float tankheight = 121; //4 feet
long calibrationvalue = 3;
long sensorrestorecalibration = 3;
float tankwidth = 152; //5 feet
float tanklength = 137; //4.5 feet

void setup() {
  // put your setup code here, to run once:
  
  serialPort.begin(115200);
  Serial.begin(115200);
  Serial.println("----------------SETUP INITIATED--------------------------");
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  Serial.println("----------------SETUP COMPLETED--------------------------");
}
  
void loop() {
  // put your main code here, to run repeatedly:
  long uptimemls = millis();
  long uptimesec = uptimemls/1000;
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ArduinoUptime"] = uptimesec;
  
  checkWaterLevel(root);
//  if(serialPort.available()>0)
//  {
    root.printTo(serialPort);
//  }
  delay(1000);
}

void checkWaterLevel(JsonObject& root) {
  long duration, distance;
  float tanklevelpercentage = 0;

  digitalWrite(trigger, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH);
  distance = (duration/2) / 29.1;
  
  Serial.println("duration");
  Serial.println(duration);

  Serial.println("distance");
  Serial.println(distance);
  
  //Blynk.virtualWrite(V1, distance);
  root["SensorDistance"] = distance;
  
  calibrationvalue = sensorrestorecalibration;
  if(distance > (tankheight + calibrationvalue)) {
    Serial.println(distance);
    distance = tankheight;
    calibrationvalue = 0;
  } 
  measureWater(distance, root);
  consumedWater(distance, root);

  int waterlevelat=0;
  if(distance > 0) {
    waterlevelat = tankheight - distance + calibrationvalue;
  }
  Serial.println("Waterlevelat");
  Serial.println(waterlevelat);
  tanklevelpercentage = waterlevelat / tankheight * 100;
  if(tanklevelpercentage > 100)  {
    tanklevelpercentage = 100;
  }
  if(tanklevelpercentage < 0)  {
    tanklevelpercentage = 0;
  }
  
  //Blynk.virtualWrite(V0, tanklevelpercentage);
  root["TankLevelPercentage"] = tanklevelpercentage;

  //Blynk.virtualWrite(V5, uptimesec);
}

void measureWater(int distance, JsonObject& root) {
  float availablelitres = 0;

  int waterlevelat=0;
  if(distance > 0) {
    waterlevelat = (tankheight - distance) + calibrationvalue;
  }
  
  float availablevolume = waterlevelat * tanklength * tankwidth;
  availablelitres = availablevolume / 1000;
  
  if(availablelitres < 0){
    availablelitres = 0;
  }
  //Blynk.virtualWrite(V3, availablelitres);
  root["AvailableLitres"] = availablelitres;
}

void consumedWater(int distance, JsonObject& root) {
  float consumedlitres = 0;
  float consumedvolume = (distance - calibrationvalue) * tanklength * tankwidth;
  consumedlitres = consumedvolume / 1000;

  if(consumedlitres < 0){
    consumedlitres = 0;
  }
  //Blynk.virtualWrite(V2, consumedlitres);
  root["ConsumedLitres"] = consumedlitres;
}
