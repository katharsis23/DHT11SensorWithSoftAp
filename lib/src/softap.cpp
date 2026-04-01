#include "softap.h"
#include "http_server.h"
#include "wifi_connection_wrapper.h"

WebServer* webServer = nullptr;
extern uint8_t flag[10];
bool provisioningActive = false;
extern WiFiConnectionManager* wifiManager;
// Config constructor
WIFI_Provisioning_Client::Config::Config(
    const char* service_name_,
    const char* service_key_,
    const char* pop_,
    int         max_retries_,
    bool        autoretry_,
    bool        reset_provisioned_)
: service_name(service_name_ ? service_name_ : "PROV_ESP32"),
  service_key(service_key_),
  pop(pop_ ? pop_ : "abcd1234"),
  max_retries(max_retries_ > 0 ? max_retries_ : 1),
  autoretry_provisioning(autoretry_),
  reset_provisioned(reset_provisioned_) {}

// Class constructor
WIFI_Provisioning_Client::WIFI_Provisioning_Client(const Config& cfg_) 
: cfg(cfg_), retry_count(0), webServer(nullptr) {}

// Setup Web Server (НОВЕ!)
void WIFI_Provisioning_Client::setupWebServer() {
  if (webServer == nullptr) {
    webServer = new WebServer(80);
    // Endpoint для конфігурації WiFi
    webServer->on("/prov-config", HTTP_POST, [this]() {
      Serial.println("[Server] Get reques '/prov-config'");
      handleConfigRequest();
    });
    
    // Endpoint для сканування WiFistopProvisioning
    webServer->on("/prov-scan", HTTP_POST, [this]() {
      Serial.println("[Server] Get reques '/prov-scan'");
      handleScanRequest();
    });
    
    // Endpoint для статусу
    webServer->on("/prov-ctrl", HTTP_GET, [this]() {
      Serial.println("[Server] Get reques '/prov-ctrl'");
      handleStatusRequest();
    });

    webServer->on("/get-mDNS", HTTP_GET, [this]() {
        Serial.println("[Server] Get reques '/get-mDNS'");
        String mDNSName = "ESP32_Device"; // або збережене ім'я
        webServer->send(200, "text/plain", mDNSName);
    });


    webServer->on("/", HTTP_GET, [this]() {
        Serial.println("[Server] Get reques '/'");
        webServer->send(200, "text/plain", WiFi.localIP().toString());
    });


    
    // Proto-ver endpoint (версія протоколу)
    webServer->on("/proto-ver", HTTP_GET, [this]() {
      String json = R"({"prov":{"ver":"v1.1"}})";
      webServer->send(200, "application/json", json);
      Serial.println("[HTTP] /proto-ver requested");
    });
        webServer->on("/hello", HTTP_GET, [this]() {
      webServer->send(200, "hello");
      Serial.println("hello");
    });
    
    // 404
    webServer->onNotFound([this]() {
      handleNotFound();
    });
    
    webServer->begin();
    Serial.println("[HTTP] Web server started on port 80");
  }
}

// Обробник /prov-config (ВИПРАВЛЕНО!)
void WIFI_Provisioning_Client::handleConfigRequest() {
  Serial.println("[HTTP] /prov-config request received");
  
  if (webServer->hasArg("plain")) {
    String body = webServer->arg("plain");
    Serial.print("[HTTP] Payload: ");
    Serial.println(body);
    
    // Парсити JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      Serial.print("[JSON] Parse error: ");
      Serial.println(error.c_str());
      
      webServer->send(400, "application/json", 
        R"({"status":"error","message":"Invalid JSON"})");
      return;
    }
    
    // Отримати SSID і пароль
    const char* ssid = doc["ssid"];
    const char* password = doc["password"];
    
    if (!ssid || !password) {
      Serial.println("[JSON] Missing ssid or password");
      
      webServer->send(400, "application/json", 
        R"({"status":"error","message":"Missing ssid or password"})");
      return;
    }
    
    Serial.print("[JSON] SSID: ");
    Serial.println(ssid);
    Serial.print("[JSON] Password: ");
    Serial.println("*********");
    
    // Відправити відповідь ПЕРЕД підключенням
    StaticJsonDocument<128> response;
    response["status"] = "success";
    response["message"] = "Config received, connecting...";
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    
    webServer->send(200, "application/json", jsonResponse);
    Serial.println("[HTTP] Response sent");
    
    // КРИТИЧНО: Почекати щоб відповідь відправилась
    delay(500);
    
    // НОВЕ: Зберегти credentials через глобальний WiFiManager
    
    if (wifiManager != nullptr) {
      Serial.println("[Prov] Saving credentials and connecting...");
      
      // Зберегти в NVS
      wifiManager->saveProfile(ssid, password, 10);
      
      // Зупинити provisioning
      stopProvisioning();
      
      // Підключитися до WiFi
      wifiManager->connectToNetwork(ssid, password);
    } else {
      Serial.println("[Prov] ERROR: wifiManager is null!");
    }
  } else {
    webServer->send(400, "application/json", 
      R"({"status":"error","message":"No payload"})");
  }
}

// Додайте метод для зупинки provisioning
void WIFI_Provisioning_Client::stopProvisioning() {
  provisioningActive = false;
  Serial.println("[Prov] Stopping provisioning mode...");
  
  // Зупинити HTTP сервер
  if (webServer != nullptr) {
    webServer->stop();
    //delete webServer;
    //webServer = nullptr;
    Serial.println("[HTTP] Web server stopped");
  }
  
  // Зупинити SoftAP
  WiFi.softAPdisconnect(true);
  Serial.println("[SoftAP] Access Point stopped");
}


// Обробник /prov-ctrl (НОВЕ!)
void WIFI_Provisioning_Client::handleStatusRequest() {
  Serial.println("[HTTP] /prov-ctrl request received");
  
  StaticJsonDocument<256> doc;
  
  if (WiFi.status() == WL_CONNECTED) {
    doc["status"] = "connected";
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
  } else {
    doc["status"] = "connecting";
  }
  
  String jsonResponse;
  serializeJson(doc, jsonResponse);
  
  webServer->send(200, "application/json", jsonResponse);
}

// Обробник 404 (НОВЕ!)
void WIFI_Provisioning_Client::handleNotFound() {
  Serial.print("[HTTP] 404: ");
  Serial.println(webServer->uri());
  
  webServer->send(404, "application/json", 
    R"({"error":"Endpoint not found"})");
}

// Handle HTTP requests (НОВЕ!)
void WIFI_Provisioning_Client::handleHTTPRequests() {
  if (webServer != nullptr) {
    //Serial.println("]]]]]]]]]");
    webServer->handleClient();
  }
}

// startProvisioning
void WIFI_Provisioning_Client::startProvisioning() {
    provisioningActive = true;
    // --- 1. Зупинити старий HTTP сервер і SoftAP ---
    if (webServer != nullptr) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
        Serial.println("[HTTP] Web server stopped");
    }

    if (WiFi.softAPgetStationNum() >= 0) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(100); // даємо WiFi час на вимкнення
        Serial.println("[SoftAP] Previous AP stopped");
    }

    // --- 2. Запуск нового AP ---
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(cfg.service_name, cfg.service_key)) {
        Serial.println("[Prov] ✗ Failed to start SoftAP");
        return;
    }

    Serial.println("\n=== WiFi Provisioning Started ===");
    Serial.print("[Prov] SoftAP SSID: "); Serial.println(cfg.service_name);
    if (cfg.service_key) {
        Serial.print("[Prov] SoftAP Password: "); Serial.println(cfg.service_key);
    } else {
        Serial.println("[Prov] SoftAP: Open (no password)");
    }

    retry_count = 0;

    // --- 3. Запуск HTTP сервера ---
    setupWebServer();
}

// onWiFiEvent
void WIFI_Provisioning_Client::onWiFiEvent(WiFiEvent_t event) {
  switch(event){

    
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("[SoftAP] ✓ Client connected!");
      Serial.print("[SoftAP] Connected clients: ");
      Serial.println(WiFi.softAPgetStationNum());
      break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("[SoftAP] Client disconnected");
      break;

    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.println("[SoftAP] Client got IP address");
      break;

        case ARDUINO_EVENT_PROV_CRED_RECV:
      Serial.println("[Prov] ✓ WiFi credentials received!");
      break;

    case ARDUINO_EVENT_PROV_CRED_FAIL:
      Serial.println("[Prov] ✗ Provisioning failed");
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    stopMainServer();
    flag[0]=0;
    flag[1]=0;
  if (provisioningActive ==true) {
    Serial.println("[WiFi] STA disconnected (ignored, provisioning)");
    break; // ⛔ НІЯКОГО reconnect
  }

  Serial.println("[WiFi] Disconnected from target AP");
  if (retry_count < cfg.max_retries) {
    retry_count++;
    WiFi.reconnect();
  } else {
    startProvisioning();
  }
  break;

    default:
      break;
  }

  if(last_event!=event)
  { 
        last_event=event;

    switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("[SoftAP] ✓ Access Point started");
      Serial.print("[SoftAP] IP: ");
      Serial.println(WiFi.softAPIP());
      setupWebServer(); // Запустити HTTP сервер при старті SoftAP
      delay(50);
      break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      startMainServer();
      Serial.println("[WiFi] Connected to target AP");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("[WiFi] Got IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("[WiFi] ✓ Provisioning successful!");
      retry_count = 0;
      break;



    case ARDUINO_EVENT_PROV_START:
      Serial.println("[Prov] Provisioning service started");
      break;

    case ARDUINO_EVENT_PROV_END:
      Serial.println("[Prov] Provisioning ended");
      break;

    default:
      break;
  }
  }

  
}

WIFI_Provisioning_Client* provClient = nullptr;

void WiFiEventHandler(WiFiEvent_t event) {
  if (provClient != nullptr) {
    provClient->onWiFiEvent(event);
  }
}

// Обробник /prov-scan (ДОДАЙТЕ ЯКЩО ВІДСУТНІЙ!)
void WIFI_Provisioning_Client::handleScanRequest() {
  Serial.println("[HTTP] /prov-scan request received");
  
  // Почати сканування WiFi
  int scanCount = WiFi.scanNetworks();
  
  Serial.print("[Scan] Found ");
  Serial.print(scanCount);
  Serial.println(" networks");
  
  // Створити JSON array з мережами
  StaticJsonDocument<1024> doc;
  JsonArray networks = doc.createNestedArray("ap_list");
  
  for (int i = 0; i < scanCount && i < 10; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["auth"] = WiFi.encryptionType(i);
    network["ch"] = WiFi.channel(i);
  }
  
  String jsonResponse;
  serializeJson(doc, jsonResponse);
  
  webServer->send(200, "application/json", jsonResponse);
  Serial.print("[HTTP] Sent ");
  Serial.print(scanCount);
  Serial.println(" networks");
  
  WiFi.scanDelete(); // Очистити результати сканування
}





void startMDNS() {
    if (!MDNS.begin("esp")) { // ім'я host: esp32.local
        Serial.println("Error starting mDNS");
        return;
    }

    Serial.println("mDNS responder started!");

    // Додатково можна додати сервіс
    MDNS.addService("http", "tcp", 80);
}

void onWiFiConnected(const char* ssid, IPAddress ip) {
  Serial.println("\n[App] ========================================");
  Serial.println("[App] ✓✓✓ WiFi Connection Successful! ✓✓✓");
  Serial.printf("[App] Network: %s\n", ssid);
  Serial.printf("[App] IP Address: %s\n", ip.toString().c_str());
  Serial.println("[App] Starting HTTP server...");
  
  // Start HTTP server after WiFi is connected
  startMainServer();
  
  Serial.println("[App] ========================================\n");
}