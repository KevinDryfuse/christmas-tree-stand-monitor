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

// LED Panel
int out_pin_blue = 5;
int out_pin_green = 4;
int out_pin_red = 3;

void setup() {
  Serial.begin(57600);

  pinMode(out_pin_red, OUTPUT);
  pinMode(out_pin_green, OUTPUT);
  pinMode(out_pin_blue, OUTPUT);
  
  analogWrite(out_pin_red, 255);
  analogWrite(out_pin_green, 0);
  analogWrite(out_pin_blue, 0);
  delay(500);
  analogWrite(out_pin_red, 0);
  analogWrite(out_pin_green, 255);
  analogWrite(out_pin_blue, 0);
  delay(500);
  analogWrite(out_pin_red, 0);
  analogWrite(out_pin_green, 0);
  analogWrite(out_pin_blue, 255);
  delay(500);
  analogWrite(out_pin_red, 0);
  analogWrite(out_pin_green, 0);
  analogWrite(out_pin_blue, 0);
  
  pinMode(in_pin_low, INPUT);
  pinMode(in_pin_high, INPUT);

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
    analogWrite(out_pin_red, 255);
    analogWrite(out_pin_green, 0);
    analogWrite(out_pin_blue, 0);
  }
  else if (low_val == 1 && high_val == 0) {
    Serial.println("The water level is in GOOD range!");
    analogWrite(out_pin_red, 0);
    analogWrite(out_pin_green, 255);
    analogWrite(out_pin_blue, 0);
  }
  else if (high_val == 1) { 
    Serial.println("The water level is HIGH enough!");
    analogWrite(out_pin_red, 255);
    analogWrite(out_pin_green, 255);
    analogWrite(out_pin_blue, 255);
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
