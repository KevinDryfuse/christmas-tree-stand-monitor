#include <ArduinoJson.h>

#include <WiFiNINA.h>
WiFiClient wifi;

#include <ArduinoHttpClient.h>
char serverAddress[] = "postman-echo.com";  // server address
int port = 80;

HttpClient client = HttpClient(wifi, serverAddress, port);
int wifiStatus = WL_IDLE_STATUS;

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char deviceId[] = DEVICE_ID;

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

  while (!Serial);
  while ( wifiStatus != WL_CONNECTED) {
    setColor(255, 255, 0);
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);     // print the network name (SSID);
    Serial.println(pass);     // print the network name (SSID);
    delay(1000);
    // Connect to WPA/WPA2 network:
    wifiStatus = WiFi.begin(ssid, pass);
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
    newStatus = "LOW";
  }
  else if (low_val == 1 && high_val == 0) {
    
    setColor(0, 255, 0);
    newStatus = "GOOD";
  }
  else if (high_val == 1) {
    setColor(255, 255, 255);
    newStatus = "HIGH";
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
    }
    else if (newStatus == "LOW") {
      Serial.println("The water level is in LOW range!");
    }
    else if (newStatus == "GOOD") {
      Serial.println("The water level is in GOOD range!");
    }
    else if (newStatus == "HIGH") {
      Serial.println("The water level is in HIGH range!");
    } 
    Serial.println("---------------------- " + String (millis()) + " - " + String (lastEventTime));

    // This is where we would POST to our endpoint
    
    currentStatus = newStatus;
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
