#ifndef DEFINES_H
#define DEFINES_H

#define DHTTYPE DHT22
#define DHTPIN 13
#define POST_PERIOD 300

typedef enum
{
  DHT_UP = 0,
  DHT_ERR = -1
} dht_status_t;

#endif