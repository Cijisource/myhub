#include "HTTPSRedirect.h"
#define buzzer D4 

static int error_count = 0;
static int connect_count = 0;
static bool flag = false;
    
const char* host = "script.google.com";
// Replace with your own script id to make server side changes
const char *GScriptId = "AKfycbwtuPilvVt2R1V6JAhAzkI9iuP47DE8TC0cRdiD_Torrj9dMJKphA0yKY1eRWOJYAg1"; // Receiving data from google script address
const char *cellAddress1 = "A2"; //To set moisture value
const char *cellAddress2 = "B2"; // to set temperature value
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";

const int httpsPort = 443;

String url1 = String("/macros/s/") + GScriptId + "/exec?read=" + cellAddress1;
String url2 = String("/macros/s/") + GScriptId + "/exec?read=" + cellAddress2;

String payload = "";
String payload1 = "";
String payload2 = "";
HTTPSRedirect* client = nullptr;
int msensor = A0; // moisture sensor is connected with the analog pin A1 of the Arduino
int msvalue = 0; // moisture sensor value 
int mspercent; // moisture value in percentage
float temp; //to store the temperature value
float hum; // to store the humidity value

int Led = D0;

// Subroutine for sending data to Google Sheets
void sendData(float value0, int value1, int value2) {

String GAS_ID = "AKfycbwtuPilvVt2R1V6JAhAzkI9iuP47DE8TC0cRdiD_Torrj9dMJKphA0yKY1eRWOJYAg1"; //sending data to google script address

String url = String("/macros/s/") + GAS_ID+ "/exec";

  payload = payload_base + "\"" + value0 + "," + value1 + "," + value2 + "\"}";
 
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  digitalWrite(buzzer,LOW);
  delay(40);
  digitalWrite(buzzer,HIGH);
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }
   
  Serial.println("a delay of several seconds is required before publishing again ");
  // a delay of several seconds is required before publishing again    
  delay(5000);
  //----------------------------------------Processing data and sending data
  
} 

void setupHttpsRedirect() {
  const unsigned int MAX_CONNECT = 20;

  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(false);
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
    error_count = 5;
  }
  
  if (connect_count > MAX_CONNECT){
    //error_count = 5;
    connect_count = 0;
    flag = false;
    delete client;
    return;
  }
}
