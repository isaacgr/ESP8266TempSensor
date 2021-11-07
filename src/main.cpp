#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
// #include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "SSD1306.h"
#include <base64.h>
#include "env.h" // environement variables

#define DHTTYPE DHT22
#define DHTPIN 13

SSD1306 display(0x3c, 4, 5);
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

bool POST = false;
int TIMER_COUNTER = 0;
int POST_PERIOD = 600; // frequency to post, in seconds

int id = 1; // jsonrpc starting message id

int displayTempHumid(float temp, float humidity, float temp_f)
{
  if (isnan(humidity) || isnan(temp) || isnan(temp_f))
  {
    display.clear(); // clearing the display
    display.drawString(5, 0, "Failed DHT");
    return -1;
  }
  display.clear();
  display.drawString(0, 0, String(humidity) + "%");
  display.drawString(0, 20, String(temp) + "C / " + String(temp_f) + "F");
  return 0;
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
  params["location"] = "bedroom";

  char JSONmessageBuffer[300];
  serializeJson(doc, JSONmessageBuffer);

  String credentials = CREDENTIALS;
  String encodedCredentials = base64::encode(credentials);

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient http; //Declare object of class HTTPClient

  http.begin(*client, URL);                                       //Specify request destination
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  http.addHeader("Authorization", "Basic " + encodedCredentials); // basic auth
  int httpCode = http.POST(JSONmessageBuffer);                    //Send the request
  String payload = http.getString();                              //Get the response payload
  http.end();                                                     //Close connection

  Serial.println(payload); //Print request response payload
  return httpCode;
}

void IRAM_ATTR timer1_ISR(void)
{
  // Do some work
  TIMER_COUNTER++;
  if (TIMER_COUNTER == 600) // update every 10 mins
  {
    POST = true;
    TIMER_COUNTER = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Waiting for connection");
    delay(500);
  }
  Serial.println("Connected.");
  dht.begin(); // initialize dht
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  POST = true;

  // Setup interrupts
  noInterrupts();
  timer1_attachInterrupt(timer1_ISR);
  timer1_isr_init();
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
  timer1_write(312500); // 80MHz == 1sec
  interrupts();

  //Set up ESP watchdog
  // ESP.wdtDisable();
  // ESP.wdtEnable(WDTO_8S);
}

void loop()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  int dhGood = displayTempHumid(t, h, f);

  if (WiFi.status() == WL_CONNECTED)
  {

    if (POST && dhGood == 0)
    {
      int httpCode = postTemperature(t, h, f);
      display.drawString(0, 40, "res: " + String(httpCode) + "    wifi: ok");
      display.display();
    }
  }
  else
  {
    display.drawString(0, 40, "wifi: no");
    display.display();
  }
  POST = false;
}