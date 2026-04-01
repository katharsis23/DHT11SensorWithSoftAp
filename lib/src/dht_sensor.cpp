#include "dht_sensor.h"

DHTSensor::DHTSensor(uint8_t pin) : pin(pin), dht(nullptr), temperature(0), humidity(0), valid(false), timestamp(0) {
    lastReadTime = 0;
}

DHTSensor::~DHTSensor() {
    if (dht) {
        delete dht;
    }
}

bool DHTSensor::begin() {
    dht = new DHT(pin, DHT11);
    dht->begin();
    Serial.printf("[DHT] DHT11 sensor initialized on pin %d\n", pin);
    return true;
}

void DHTSensor::read() {
    if (!dht) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < READ_INTERVAL) {
        return;
    }
    
    lastReadTime = currentTime;
    
    float newTemp = dht->readTemperature();
    float newHum = dht->readHumidity();
    
    if (!isnan(newTemp) && !isnan(newHum)) {
        temperature = newTemp;
        humidity = newHum;
        valid = true;
        timestamp = currentTime;
        Serial.printf("[DHT] Temp: %.1f°C, Hum: %.1f%%\n", temperature, humidity);
    } else {
        valid = false;
        Serial.println("[DHT] Failed to read sensor data");
    }
}

String DHTSensor::getJsonData() const {
    String json = "\"dht\":{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"valid\":" + String(valid ? "true" : "false") + ",";
    json += "\"timestamp\":" + String(timestamp);
    json += "}";
    return json;
}
