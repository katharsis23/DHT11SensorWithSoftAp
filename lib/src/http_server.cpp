#include "http_server.h"
#include "wifi_connection_wrapper.h"
#include "sensor_manager.h"

WebServer apiServer(80);
extern JSON_Handler hostJson;
extern uint8_t flag[10];
extern WiFiConnectionManager* wifiManager;
extern SensorManager* sensorManager;
bool mainServerActive = false;

void startMainServer() {
    if (mainServerActive) return;
    mainServerActive = true;
    Serial.println("[Server] Starting main server...");
    
    apiServer.on("/hello", HTTP_GET, []() {
        Serial.println("[Server] Get request '/hello'");
        apiServer.send(200, "application/json",
                       "{\"message\":\"Hello from ESP32\"}");
    });

    apiServer.on("/delete-credentials", HTTP_GET, []() {
        Serial.println("[Server] Get request '/delete-credentials'");
        wifiManager->clearAllProfiles();
        apiServer.send(200, "application/json",
                       "{\"message\":\"Credentials deleted\"}");
    });

    apiServer.on("/get-mDNS", HTTP_GET, []() {
        Serial.println("[Server] Get request '/get-mDNS'");
        String mDNSName = "ESP32_Device";
        apiServer.send(200, "text/plain", mDNSName);
    });


    apiServer.on("/", HTTP_GET, []() {
        Serial.println("[Server] Get request '/'");
        apiServer.send(200, "text/plain", WiFi.localIP().toString());
    });
    
    apiServer.on("/device", HTTP_GET, []() {
        Serial.println("[Server] Get request '/device'");
        flag[1]=1;
        std::string jsonStr = jsonHandlerToString(hostJson);
        apiServer.send(200, "application/json", jsonStr.c_str());
    });
    
    apiServer.on("/proto-ver", HTTP_GET, []() {
        Serial.println("[Server] Get request '/proto-ver'");
        apiServer.send(200, "text/plain", "OK");
    });

    // Sensor data endpoints
    apiServer.on("/sensors", HTTP_GET, []() {
        Serial.println("[Server] Get request '/sensors'");
        if (sensorManager && sensorManager->isValid()) {
            String jsonData = sensorManager->getJsonData();
            apiServer.send(200, "application/json", jsonData);
        } else {
            apiServer.send(503, "application/json", 
                          "{\"error\":\"Sensors not initialized\"}");
        }
    });

    apiServer.on("/sensors/dht", HTTP_GET, []() {
        Serial.println("[Server] Get request '/sensors/dht'");
        if (sensorManager && sensorManager->isValid()) {
            String jsonData = sensorManager->getDHTJson();
            apiServer.send(200, "application/json", jsonData);
        } else {
            apiServer.send(503, "application/json", 
                          "{\"error\":\"DHT sensor not initialized\"}");
        }
    });

    apiServer.begin();
    Serial.println("[API] Main server started on port 80");
}

void handleMainServer() {
    apiServer.handleClient();
}

void stopMainServer() {
    if (!mainServerActive) return;
    mainServerActive = false;
    apiServer.stop();
    Serial.println("[API] Main server stopped");
}
