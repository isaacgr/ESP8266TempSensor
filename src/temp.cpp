#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SSD1306.h>
#include <WiFiClientSecureBearSSL.h>
#include <base64.h>
#include "defines.h"
#include "temp.h"
#include "secrets.h"

int id = 1;

int displayTempHumid(float temp, float humidity, float temp_f)
{
  if (isnan(humidity) || isnan(temp) || isnan(temp_f))
  {
    display.clear(); // clearing the display
    display.drawString(5, 0, "Failed DHT");
    return DHT_ERR;
  }
  display.clear();
  display.drawString(0, 0, String(humidity) + "%");
  display.drawString(0, 20, String(temp) + "C / " + String(temp_f) + "F");
  return DHT_UP;
}

int postTemperature(float temp, float humidity, float temp_f)
{
  StaticJsonDocument<192> doc;

  doc["jsonrpc"] = "2.0";
  doc["method"] = "set.temperature";
  doc["id"] = id;
  id++;

  JsonObject params = doc.createNestedObject("params");
  params["temperature_C"] = temp;
  params["humidity"] = humidity;
  params["temperature_F"] = temp_f;
  params["location"] = "cellar";

  char JSONmessageBuffer[300];
  serializeJson(doc, JSONmessageBuffer);

  String encodedCredentials = base64::encode(CREDENTIALS);

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient http; // Declare object of class HTTPClient

  http.begin(*client, URL);                                       // Specify request destination
  http.addHeader("Content-Type", "application/json");             // Specify content-type header
  http.addHeader("Authorization", "Basic " + encodedCredentials); // basic auth
  int httpCode = http.POST(JSONmessageBuffer);                    // Send the request
  String payload = http.getString();                              // Get the response payload
  http.end();                                                     // Close connection

  Serial.println(payload); // Print request response payload
  return httpCode;
}