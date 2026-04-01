#include "sensor_manager.h"

SensorManager::SensorManager() : dhtSensor(nullptr), initialized(false) {
}

SensorManager::~SensorManager() {
    if (dhtSensor) {
        delete dhtSensor;
    }
}

bool SensorManager::begin() {
    Serial.println("[SensorManager] Initializing DHT11 sensor...");
    
    dhtSensor = new DHTSensor(DHT_PIN);
    if (!dhtSensor->begin()) {
        Serial.println("[SensorManager] ✗ Failed to initialize DHT sensor");
        return false;
    }
    
    initialized = true;
    Serial.println("[SensorManager] ✓ DHT11 sensor initialized");
    return true;
}

void SensorManager::readAllSensors() {
    if (!initialized || !dhtSensor) return;
    
    dhtSensor->read();
}

String SensorManager::getJsonData() const {
    String json = "{";
    json += "\"timestamp\":" + String(millis()) + ",";
    json += "\"valid\":" + String(isValid() ? "true" : "false") + ",";
    json += getDHTJson();
    json += "}";
    return json;
}

String SensorManager::getDHTJson() const {
    if (!dhtSensor) return "\"dht\":null";
    
    return dhtSensor->getJsonData();
}

bool SensorManager::isValid() const {
    return initialized && dhtSensor && dhtSensor->isValid();
}

unsigned long SensorManager::getTimestamp() const {
    if (dhtSensor) {
        return dhtSensor->getTimestamp();
    }
    return 0;
}
