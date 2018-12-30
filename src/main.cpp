#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2 // Pin which is connected to the DHT sensor.

//#define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)

#define DHTPIN 5 // what pin we're connected to
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

void setup()
{
  Serial.begin(115200);
  WiFi.begin("VIRGIN559", "412C7934"); //WiFi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Waiting for connection");
  }
  dht.begin(); // initialize dht
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {

    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    const size_t bufferSize = JSON_OBJECT_SIZE(4);
    DynamicJsonBuffer jsonBuffer(bufferSize);

    JsonObject &root = jsonBuffer.createObject();
    root["temp"] = t;
    root["humid"] = h;
    root["temp_f"] = f;
    root["loc"] = "living room";
    root["key"] = "secretkey";

    char JSONmessageBuffer[300];
    root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);

    HTTPClient http; //Declare object of class HTTPClient

    http.begin("http://irowell-temperature.herokuapp.com/api/temp"); //Specify request destination
    http.addHeader("Content-Type", "application/json");              //Specify content-type header

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload

    Serial.println("Code:");
    Serial.println(httpCode);
    Serial.println("Response:");
    Serial.println(payload); //Print request response payload

    http.end(); //Close connection
  }
  else
  {
    Serial.println("Error in WiFi connection");
  }
  delay(5000); //Send a request every 5 seconds
}