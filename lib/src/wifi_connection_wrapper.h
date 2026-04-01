#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <WiFi.h>
#include "connect_to_wifi.h"
#include "softap.h"

// WiFi connection states
enum class WiFiConnectionState {
  DISCONNECTED,
  SCANNING,
  CONNECTING,
  CONNECTED,
  PROVISIONING,
  FAILED
};

// Callback types for async notifications
typedef std::function<void(WiFiConnectionState)> StateChangeCallback;
typedef std::function<void(const char* ssid, IPAddress ip)> ConnectedCallback;
typedef std::function<void()> ProvisioningStartedCallback;

// Main WiFi connection interface
class WiFiConnectionManager {
public:
  struct Config {
    const char* provisioning_ssid;
    const char* provisioning_password;
    const char* pop;
    bool auto_reconnect;
    uint32_t connection_timeout_ms;
    
    Config(const char* prov_ssid = "PROV_ESP32",
           const char* prov_pass = nullptr,
           const char* pop_ = "abcd1234",
           bool auto_reconnect_ = true,
           uint32_t timeout = 20000)
    : provisioning_ssid(prov_ssid),
      provisioning_password(prov_pass),
      pop(pop_),
      auto_reconnect(auto_reconnect_),
      connection_timeout_ms(timeout) {}
  };

  WiFiConnectionManager(const Config& config);
  ~WiFiConnectionManager();

  // Main interface methods
  bool initialize();
  bool connect();  // Smart connection: tries saved networks, then provisioning
  void disconnect();
  void reset();    // Clear all saved credentials
  
  // State management
  WiFiConnectionState getState() const { return current_state; }
  bool isConnected() const;
  String getConnectedSSID() const;
  IPAddress getIP() const;
  
  // Manual operations
  bool connectToNetwork(const char* ssid, const char* password);
  bool startProvisioning();
  void stopProvisioning();
  std::vector<WiFiNetworkInfo> scanNetworks();
  
  // Profile management
  bool saveProfile(const char* ssid, const char* password, int priority = 0);
  int getProfileCount();
  void clearAllProfiles();
  
  // Callbacks
  void onStateChange(StateChangeCallback callback) { state_callback = callback; }
  void onConnected(ConnectedCallback callback) { connected_callback = callback;}
  void onProvisioningStarted(ProvisioningStartedCallback callback) { prov_callback = callback; }
  
  // For loop() or FreeRTOS task
  void handle();  // Call this regularly
  
  // FreeRTOS task wrapper (static)
  static void taskWrapper(void* parameter);
 

private:
  Config config;
  WiFiConnectionState current_state;
  
  NVS_Manager* nvs_manager;
  WIFI_Provisioning_Client* prov_client;
  
  StateChangeCallback state_callback;
  ConnectedCallback connected_callback= nullptr;
  ProvisioningStartedCallback prov_callback;
  
  bool tryConnectToSavedNetworks();
  void setState(WiFiConnectionState new_state);
  void handleProvisioning();
  
  unsigned long last_connection_check;
  static const unsigned long CONNECTION_CHECK_INTERVAL = 5000;
};
// Global instance pointer (for WiFi event handler)
extern WiFiConnectionManager* g_wifiManager;

#endif // WIFI_CONNECTION_H
