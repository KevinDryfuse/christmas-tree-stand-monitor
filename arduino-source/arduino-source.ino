#include <ArduinoJson.h>

#include <WiFiNINA.h>
WiFiClient wifi;

#include <ArduinoHttpClient.h>
const char serverAddress[] = "postman-echo.com";  // server address
int port = 80;

HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// Water Level Sensor
int in_pin_low = 7;
int in_pin_high = 8;
int low_val;
int high_val;

void setup() {
  Serial.begin(57600);

  while (!Serial);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);     // print the network name (SSID);
    Serial.println(pass);     // print the network name (SSID);
    delay(1000);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network we're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the arduino's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  Serial.println("Loop!");


  low_val = digitalRead(in_pin_low);   // read the input pin
  high_val = digitalRead(in_pin_high);   // read the input pin
  Serial.println("----------------------");
  Serial.println(low_val);
  Serial.println(high_val);
  if (low_val == 0) {
    Serial.println("The water level is LOW!");
  }
  else if (low_val == 1 && high_val == 0) {
    Serial.println("The water level is in GOOD range!");
  }
  else if (high_val == 1) { 
    Serial.println("The water level is HIGH enough!");
  }
  else {
    Serial.println("There is an issue");
  }



  
  Serial.println("----------------------");
  delay(50);
}

int post_data() {
  client.sendHeader("Cache-Control", "no-cache");
  
  // send the POST request
  Serial.println("making post request");

  String postData = "{\"mykey\":\"myvalue\"}";
  
  client.post("/post", "application/json", postData);
  
  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
    
  String response = client.responseBody();
  DynamicJsonDocument doc(2000);
  deserializeJson(doc, response);
  JsonObject obj = doc.as<JsonObject>();

  String keyValue = obj[String("data")][String("mykey")];
  Serial.print("keyValue: ");
  Serial.println(keyValue);
  return statusCode;
}
