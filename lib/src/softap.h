#pragma once
#ifndef SOFTAP_H
#define SOFTAP_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiProv.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>
class WIFI_Provisioning_Client {
public:
  struct Config {
    const char* service_name;
    const char* service_key;
    const char* pop;
    int         max_retries;
    bool        autoretry_provisioning;
    bool        reset_provisioned;

    Config(const char* service_name_,
           const char* service_key_,
           const char* pop_,
           int         max_retries_,
           bool        autoretry_,
           bool        reset_provisioned_ = false);
  };

  explicit WIFI_Provisioning_Client(const Config& cfg_);
  
  void startProvisioning();
  void onWiFiEvent(WiFiEvent_t event);
  void stopProvisioning();
  
  // НОВІ методи для HTTP обробки
  void setupWebServer();
  void handleHTTPRequests();

private:
  Config cfg;
  int    retry_count;
  WebServer* webServer; // HTTP сервер
  WiFiEvent_t last_event = WiFiEvent_t();
  // HTTP обробники
  void handleConfigRequest();
  void handleScanRequest();
  void handleStatusRequest();
  void handleNotFound();
};

extern WIFI_Provisioning_Client* provClient;
void WiFiEventHandler(WiFiEvent_t event);
void startMDNS();
void onWiFiConnected(const char* ssid, IPAddress ip);
#endif // SOFTAP_H
