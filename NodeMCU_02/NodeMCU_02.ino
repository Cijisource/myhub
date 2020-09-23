#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "ODbXgkyA-fZohqppkwa0qm8QusGnDXCa";

// Your WiFi credentials.
// Set password to "" for open networks.
 char ssid[] = "Cijaiz_Home";
 char pass[] = "M00n5050";
//char ssid[] = "Galaxy A719DBD";
//char pass[] = "mygalaxya71";
//char ssid[] = "Cijaiz complex";
//char pass[] = "9000150001";

bool isSTankLowEmailSent = false;
bool isSTankFullEmailSent = false;
bool isCTankLowEmailSent = false;
bool isCTankFullEmailSent = false;

float compressorTankPercentage = 0;
float cementTankPercentage = 0;
WidgetLED led0(V1);
WidgetLED led1(V2);
WidgetLED led2(V3);
WidgetLED led3(V4);

BlynkTimer timer;

void setup() {
  Blynk.begin(auth, ssid, pass);
  
  Serial.begin(115200);
  // Setup a function to be called every second
  timer.setInterval(1000L, uploadtoBlynk);
}

BLYNK_CONNECTED(){
  Blynk.email("{DEVICE_NAME} Successfully Connected", "{DEVICE_NAME} Connected");
}

BLYNK_WRITE(V0) {
  compressorTankPercentage = param.asFloat();

  if(compressorTankPercentage > 90) {
    if(!isSTankFullEmailSent) {
      Blynk.email("Compressor Tank", "Compressor Tank is Full");  
      isSTankFullEmailSent = true;
    }
  }
  else {
    isSTankFullEmailSent = false;
  }
   
  if (compressorTankPercentage < 40) {
    if (!isSTankLowEmailSent) {
      Blynk.email("Compressor Tank", "Quarter Level reached. Please Refill.");
      Serial.println(isSTankLowEmailSent);
      Serial.println("Compressor mail sent..");
      isSTankLowEmailSent = true; 
    }
  }
  else {
    isSTankLowEmailSent = false; 
  }
}

BLYNK_WRITE(V10) {
  cementTankPercentage = param.asFloat();
  
  if(cementTankPercentage == 100) {
    if (!isCTankFullEmailSent) {
      Blynk.email("Cement Tank", "Cement Tank is Full.");
      Serial.println(isCTankFullEmailSent);
      Serial.println("Cement tank mail sent..");
      isCTankFullEmailSent = true;  
    }
  }
  else {
    isCTankFullEmailSent = false;
  }
  
  if(cementTankPercentage < 40) {
    if (!isCTankLowEmailSent) {
        Blynk.email("Cement Tank", "Quarter Level reached. Please Refill.");
        Serial.println(isCTankLowEmailSent );
        Serial.println("Cement tank quarter mail sent..");
        
        isCTankLowEmailSent = true;  
    }
  }
  else {
    isCTankLowEmailSent = false;
  }
}

void uploadtoBlynk(){
  Blynk.virtualWrite(V0, compressorTankPercentage);
  Serial.println(compressorTankPercentage);

  Blynk.virtualWrite(V10, cementTankPercentage);
  Serial.println(cementTankPercentage);

  if(compressorTankPercentage < 25) {
    led0.on();
    led1.off();
    led2.off();
    led3.off();
  }
  if(compressorTankPercentage > 25 && compressorTankPercentage < 50) {
    led0.on();
    led1.on();
    led2.off();
    led3.off();
  }
  if(compressorTankPercentage > 50 && compressorTankPercentage < 75) {
    led0.on();
    led1.on();
    led2.on();
    led3.off();
  }
  if(compressorTankPercentage > 75 && compressorTankPercentage < 100) {
    led0.on();
    led1.on();
    led2.on();
    led3.on();
 }
}

void loop() {
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
}
