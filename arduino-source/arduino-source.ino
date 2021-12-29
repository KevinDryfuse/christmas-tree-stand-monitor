#include <ArduinoJson.h>
#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char deviceId[] = DEVICE_ID;

#include <SPI.h>
#include <WiFiNINA.h>
WiFiClient wifi;

#include <ArduinoHttpClient.h>
char serverAddress[] = "christmas-tree-stand-prd.herokuapp.com";  // server address
int port = 80;

HttpClient client = HttpClient(wifi, serverAddress, port);
int wifiStatus = WL_IDLE_STATUS;

// Water Level Sensor
const int IN_PIN_LOW = 7;
const int IN_PIN_HIGH = 8;
int low_val;
int high_val;

// RGB LED
const int OUT_PIN_RED = 3;
const int OUT_PIN_GREEN = 4;
const int OUT_PIN_BLUE = 5;

// Used to determine if a change happened within the last N seconds
unsigned long lastEventTime;
String currentStatus = "";


void setup() {
  Serial.begin(57600);

  startupColors();
  setColor(255, 255, 0);
  
  while ( wifiStatus != WL_CONNECTED) {
    
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);     // print the network name (SSID);
    Serial.println(pass);     // print the network name (SSID);
    delay(2000);
    // Connect to WPA/WPA2 network:
    wifiStatus = WiFi.begin(ssid, pass);
    Serial.print(wifiStatus);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WL_CONNECTED");
    }
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WL_NO_SHIELD");
    }
    if (WiFi.status() == WL_IDLE_STATUS) {
      Serial.println("WL_IDLE_STATUS");
    }
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.println("WL_NO_SSID_AVAIL");
    }
    if (WiFi.status() == WL_SCAN_COMPLETED) {
      Serial.println("WL_SCAN_COMPLETED");
    }
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("WL_CONNECT_FAILED");
    }
    if (WiFi.status() == WL_CONNECTION_LOST) {
      Serial.println("WL_CONNECTION_LOST");
    }
    if (WiFi.status() == WL_DISCONNECTED) {
      Serial.println("WL_DISCONNECTED");
    }
    
  }
  setColor(0, 0, 255);
  delay(2000);

  // print the SSID of the network we're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.println("----------------------");

  // print the arduino's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("----------------------");

  Serial.print("Device ID: ");
  Serial.println(deviceId);

  lastEventTime = millis();

}

void loop() {

  low_val = digitalRead(IN_PIN_LOW);   // read the low water level sensor
  high_val = digitalRead(IN_PIN_HIGH);   // read the high water level sensor

  boolean significantEvent = false;
  String newStatus = "";

  if (low_val == 0) {
    setColor(255, 0, 0);
    newStatus = "low";
  }
  else if (low_val == 1 && high_val == 0) {
    setColor(0, 255, 0);
    newStatus = "acceptable";
  }
  else if (high_val == 1) {
    setColor(255, 255, 255);
    newStatus = "full";
  }
  else {
    Serial.println("There is an issue");
    setColor(0, 0, 255);
  }

  if (isSignificantEvent(currentStatus, newStatus)) {
    Serial.println("SIGNIFICANT");
    Serial.print("Low Value: ");
    Serial.println(low_val);
    Serial.print("High Value: ");
    Serial.println(high_val);

    if (currentStatus == "") {
      Serial.println("System has been reset!");
      newStatus = "reset";
    }
    else if (newStatus == "low") {
      Serial.println("The water level is in low range!");
    }
    else if (newStatus == "acceptable") {
      Serial.println("The water level is in acceptable range!");
    }
    else if (newStatus == "full") {
      Serial.println("The water level is in full range!");
    } 
    Serial.println("---------------------- " + String (millis()) + " - " + String (lastEventTime));
    
    currentStatus = newStatus;

    post_data(currentStatus);
    
  }

  delay(50);
}


boolean isSignificantEvent(String currentStatus, String newStatus) {
  boolean significantEvent = false;
  if ((currentStatus != newStatus & (millis() - lastEventTime > 30000)) | (currentStatus == "")) {
    significantEvent = true;
    lastEventTime = millis();
  }
  return significantEvent;
}


void setColor(int red, int green, int blue) {
  analogWrite(OUT_PIN_RED, red);
  analogWrite(OUT_PIN_GREEN, green);
  analogWrite(OUT_PIN_BLUE, blue);
}


void startupColors() {

  for (int counter = 0; counter <= 255; counter++) {
    setColor(0, 0, counter);
    delay(10);
  }

  for (int counter = 255; counter >= 0; counter--) {
    setColor(0, 0, counter);
    delay(10);
  }
}


int post_data(String currentStatus) {
  client.sendHeader("Cache-Control", "no-cache");

  // send the POST request
  Serial.println("making post request");

  String postData = "{\"status\": \"" + currentStatus + "\"}";
  client.post("/stands/" + String (DEVICE_ID) + "/status", "application/json", postData);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);

  String response = client.responseBody();
  Serial.println(response);

  return statusCode;
}
