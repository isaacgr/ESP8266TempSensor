#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SSD1306.h>
#include "defines.h"
#include "temp.h"
#include "secrets.h"

SSD1306 display(0x3c, 4, 5);
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

bool POST = false;
int TIMER_COUNTER = 0;

void IRAM_ATTR timer1_ISR(void)
{
  // Do some work
  TIMER_COUNTER++;
  if (TIMER_COUNTER == POST_PERIOD)
  {
    POST = true;
    TIMER_COUNTER = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
}

void loop()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  int dhtStatus = displayTempHumid(t, h, f);

  if (WiFi.status() == WL_CONNECTED)
  {

    if (POST && dhtStatus == DHT_UP)
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