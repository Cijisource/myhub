#define BLYNK_TEMPLATE_ID "TMPL0tRLYzze"
#define BLYNK_TEMPLATE_NAME "Sintex Tank Monitor"
#define DEVICE_NAME "Sintex Tank Monitor"
#define DEVICE_SOFTWARE "ESP_SINTEX_01_12_2023{DD_MM_YYYY}"
#define BLYNK_FIRMWARE_VERSION "0.2.4"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "Config.h"
#include "Utilities.h"

#define DHTPIN D3 //connect DHT data pin to D3
#define DHTTYPE DHT11   // DHT 11

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

void setup() {
  Serial.begin(115200);
  Serial.flush();
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  // flush() is needed to print the above (connecting...) message reliably, 
  // in case the wireless connection doesn't go through
  Serial.flush();

  //pinMode(msensor, INPUT);
  pinMode(Led, OUTPUT);
  //digitalWrite(Led, LOW);
  
  //pinMode(DHTPIN, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(Led,HIGH);
  digitalWrite(buzzer,HIGH);
  delay(2000);
    
  //Begins to receive Temperature and humidity values.  
  //From sensor
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(Led, LOW);
    Serial.print(".");
  }

  digitalWrite(Led, HIGH);
  digitalWrite(buzzer,LOW);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(buzzer,HIGH);

  digitalWrite(Led,LOW);
  digitalWrite(buzzer,LOW);
  delay(40);
  digitalWrite(Led,HIGH);
  digitalWrite(buzzer,HIGH);
}

void loop() {
  setupHttpsRedirect();
 
  temp = 100;
  hum = 1221;
  Serial.print("temperature = ");
  Serial.println(temp);
  Serial.print("humidity = ");
  Serial.println(hum);
  msvalue = 20;
  mspercent = map(msvalue,0,1023,100,0); // To display the soil moisture value in percentage
 
  Serial.print("GET Data from cell 1 soil moisture: ");
  Serial.println(cellAddress1);

  if (client->GET(url1, host)){
    //get the value of the cell
    payload1 = client->getResponseBody();
  digitalWrite(buzzer,LOW);
  delay(40);
    digitalWrite(buzzer,HIGH);
    payload1.trim(); //soil moisture set value
    Serial.println(payload1);
    ++connect_count;
  }

  Serial.print("GET Data from cell 2: ");
  Serial.println(cellAddress2);

  if (client->GET(url2, host)){
    //get the value of the cell
    payload2 = client->getResponseBody();
  digitalWrite(buzzer,LOW);
  delay(40);
    digitalWrite(buzzer,HIGH);
    payload2.trim(); /// temperature value
    Serial.println(payload2);
    ++connect_count;
  }
 
  if (error_count > 3){
    Serial.println("Halting processor..."); 
    delete client;
    client = nullptr;
    Serial.flush();
    ESP.deepSleep(0);
  }
  
  // Add some delay in between checks
  delay(1000);
  int moisture = payload1.toInt();
  int ctemp= payload2.toInt();
   
  if(mspercent<=moisture)
  {
    //digitalWrite(buzzer,HIGH);
  }

  if(mspercent>moisture)
  {
    //digitalWrite(buzzer,LOW);
  }
    
  if(temp>=ctemp)
  {
    //digitalWrite(Led,LOW);
  }


  if(temp<ctemp)
  {
    //digitalWrite(Led,LOW);
  }
  
  sendData(temp, hum, mspercent); //--> Calls the sendData Subroutine
  
}
