#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "wifi_connection_wrapper.h"
#include "http_server.h"
#include "sensor_manager.h"

WiFiConnectionManager* wifiManager = nullptr;
SensorManager* sensorManager = nullptr;

// Global flag variable
uint8_t flag[10] = {0,0,0,0,0,0,0,0,0,0};


void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   ESP32 WiFi Connection Manager       ║");
  Serial.println("║   Smart WiFi with Auto-Provisioning   ║");
  Serial.println("║   NVS Cleared - Fresh Start          ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  // Configure WiFi manager
  WiFiConnectionManager::Config config(
    "ESP32_PROV",    // Provisioning SSID (visible to phones)
    nullptr,         // Provisioning password (nullptr = open network)
    "abcd1234",      // Proof of Possession (PoP) for security
    true,            // Auto-reconnect if connection lost
    20000            // Connection timeout (20 seconds)
  );

  // Create WiFi manager instance
  wifiManager = new WiFiConnectionManager(config);

  // Initialize sensor manager
  Serial.println("[Setup] Initializing sensor manager...");
  sensorManager = new SensorManager();
  if (!sensorManager->begin()) {
    Serial.println("[Setup] ✗ Failed to initialize sensor manager!");
  } else {
    Serial.println("[Setup] ✓ Sensor manager initialized successfully");
  }

  // Initialize WiFi manager
  Serial.println("[Setup] Initializing WiFi manager...");
  if (!wifiManager->initialize()) {
    Serial.println("[Setup] ✗ Failed to initialize WiFi manager!");
    return;
  }

  // Clear NVS for fast network resolution
  Serial.println("[Setup] Clearing NVS for fresh start...");
  wifiManager->clearAllProfiles();

  // Register callbacks for events
  wifiManager->onConnected(onWiFiConnected);
  
  // Smart connect: tries saved networks first, then provisioning
  Serial.println("[Setup] Starting smart WiFi connection...");
  wifiManager->connect();
  Serial.println("[Setup] Setup complete\n");

}

void loop() {
  // Handle WiFi manager
  if (wifiManager) {
    wifiManager->handle();
  }
  
  // Handle HTTP server
  handleMainServer();
  
  // Read sensors every 5 seconds
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 5000) {
    if (sensorManager) {
      sensorManager->readAllSensors();
    }
    lastSensorRead = millis();
  }
  
  // Blink LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(2, !digitalRead(2));
    lastBlink = millis();
  }
  
  delay(100);
}
