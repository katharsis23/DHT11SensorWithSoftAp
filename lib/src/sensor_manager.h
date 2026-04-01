#pragma once
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "dht_sensor.h"

class SensorManager {
public:
    SensorManager();
    ~SensorManager();
    
    bool begin();
    void readAllSensors();
    
    String getJsonData() const;
    String getDHTJson() const;
    
    bool isValid() const;
    unsigned long getTimestamp() const;
    
private:
    DHTSensor* dhtSensor;
    bool initialized;
    static const uint8_t DHT_PIN = 4;  // GPIO4 for DHT11
};

#endif // SENSOR_MANAGER_H
