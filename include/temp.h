#ifndef TEMP_H
#define TEMP_H

#include <Wire.h>
#include <SSD1306.h>

extern SSD1306 display;
int displayTempHumid(float temp, float humidity, float temp_f);
int postTemperature(float temp, float humidity, float temp_f);

#endif