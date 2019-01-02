#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "SSD1306.h"

#define DHTPIN 2

#define DHTTYPE DHT22
#define DHTPIN 13

SSD1306 display(0x3c, 4, 5);
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
bool POST = false;
int counter = 0;

void displayTempHumid(float temp, float humidity, float temp_f)
{
  if (isnan(humidity) || isnan(temp) || isnan(temp_f))
  {
    display.clear(); // clearing the display
    display.drawString(5, 0, "Failed DHT");
    return;
  }
  display.clear();
  display.drawString(0, 0, "Humidity: " + String(humidity) + "%\t");
  display.drawString(0, 20, "Temp: " + String(temp) + "C");
  display.drawString(0, 40, "             " + String(temp_f) + "F");
}

void postTemperature()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  const size_t bufferSize = JSON_OBJECT_SIZE(5);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();
  root["temp"] = t;
  root["humid"] = h;
  root["temp_f"] = f;
  root["loc"] = "living room";
  root["key"] = "secretsauce";

  char JSONmessageBuffer[300];
  root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  // Serial.println(JSONmessageBuffer);

  HTTPClient http; //Declare object of class HTTPClient

  http.begin("http://irowell-temperature.herokuapp.com/api/temp"); //Specify request destination
  http.addHeader("Content-Type", "application/json");              //Specify content-type header
  http.end();                                                      //Close connection
  int httpCode = http.POST(JSONmessageBuffer);                     //Send the request
  String payload = http.getString();                               //Get the response payload

  // Serial.println("Code:");
  // Serial.println(httpCode);
  // Serial.println("Response:");
  // Serial.println(payload); //Print request response payload
}

void ICACHE_RAM_ATTR timer1_ISR(void)
{
  // Do some work
  counter++;
  if (counter == 60)
  {
    POST = true;
    counter = 0;
  }
}

void setup()
{
  // Serial.begin(115200);
  WiFi.begin("VIRGIN559", "412C7934"); //WiFi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.println("Waiting for connection");
  }
  dht.begin(); // initialize dht
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  postTemperature();

  // Setup interrupts
  noInterrupts();
  timer1_attachInterrupt(timer1_ISR);
  timer1_isr_init();
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  timer1_write(6250000); // 80MHz == 1sec
  interrupts();

  //Set up ESP watchdog
  // ESP.wdtDisable();
  // ESP.wdtEnable(WDTO_8S);
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED)
  {

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    displayTempHumid(t, h, f);
    display.display();
    if (POST)
    {
      // Serial.println("POST");
      postTemperature();
      // Set-up the next interrupt cycle
      timer1_write(6250000); //80Mhz -> 80*10^6 = 1 second
    }
  }
  else
  {
    display.clear();
    display.drawString(0, 0, "Error in wifi connection");
    display.display();
  }
  POST = false;
}