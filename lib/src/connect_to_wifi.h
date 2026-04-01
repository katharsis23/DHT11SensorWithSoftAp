#ifndef CONNECT_TO_WIFI_H
#define CONNECT_TO_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <vector>
#include "softap.h"


struct WiFiNetworkInfo{
    String ssid;
    int8_t rssi;
    wifi_auth_mode_t encryption;
    uint8_t channel;
};

// WiFi Profile structure
struct WiFiProfile {
  String ssid;
  String password;
  String mDNS;

  int priority;  // Вищий = більший пріоритет
  
  WiFiProfile(String s = "", String p = "", int pr = 0)
    : ssid(s), password(p), priority(pr) {}
};

class NVS_Manager {
public:
  NVS_Manager();
  
  // Single profile operations
  bool saveCredentials(const char* ssid, const char* password);
  bool loadCredentials(String& ssid, String& password);
  
  // Multiple profiles operations (НОВЕ!)
  bool saveProfile(const char* ssid, const char* password, int priority = 0);
  bool loadProfiles(std::vector<WiFiProfile>& profiles);
  bool deleteProfile(const char* ssid);
  bool hasProfile(const char* ssid);
  int getProfileCount();
  void clearAllProfiles();
  
  // Smart connection (НОВЕ!)
  bool connectToAnyAvailable();
  bool connectToProfile(const WiFiProfile& profile);
  
  
  // WiFi scanning
  std::vector<WiFiNetworkInfo> scanNetworks();
  
  // WiFi connection
  bool connectToWiFi(const char* ssid, const char* password, uint32_t timeout_ms = 20000);
  bool isConnected();
  bool disconnect();

private:
  Preferences preferences;
  static const char* NVS_NAMESPACE;
  static const char* NVS_PROFILES_KEY;
  static const int MAX_PROFILES = 5;
  
  String serializeProfiles(const std::vector<WiFiProfile>& profiles);
  bool deserializeProfiles(const String& json, std::vector<WiFiProfile>& profiles);
};

#endif
