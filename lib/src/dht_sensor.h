#pragma once
#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

class DHTSensor {
public:
    DHTSensor(uint8_t pin);
    ~DHTSensor();
    
    bool begin();
    void read();
    
    float getTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    bool isValid() const { return valid; }
    unsigned long getTimestamp() const { return timestamp; }
    
    String getJsonData() const;
    
private:
    uint8_t pin;
    DHT* dht;
    float temperature;
    float humidity;
    bool valid;
    unsigned long timestamp;
    unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL = 2000; // 2 seconds
};

#endif // DHT_SENSOR_H
