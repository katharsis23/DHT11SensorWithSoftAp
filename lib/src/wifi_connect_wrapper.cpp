#include "wifi_connection_wrapper.h"
#include "softap.h"
#include "connect_to_wifi.h"

WiFiConnectionManager::WiFiConnectionManager(const Config& config) 
  : config(config), current_state(WiFiConnectionState::DISCONNECTED), 
    nvs_manager(nullptr), prov_client(nullptr) {
}

WiFiConnectionManager::~WiFiConnectionManager() {
  if (prov_client) {
    delete prov_client;
  }
  if (nvs_manager) {
    delete nvs_manager;
  }
}

bool WiFiConnectionManager::initialize() {
  nvs_manager = new NVS_Manager();
  prov_client = new WIFI_Provisioning_Client(WIFI_Provisioning_Client::Config(
    config.provisioning_ssid,
    config.provisioning_password,
    config.pop,
    3,
    true,
    false
  ));
  
  setState(WiFiConnectionState::DISCONNECTED);
  return true;
}

bool WiFiConnectionManager::connect() {
  if (tryConnectToSavedNetworks()) {
    return true;
  }
  
  return startProvisioning();
}

void WiFiConnectionManager::disconnect() {
  WiFi.disconnect();
  setState(WiFiConnectionState::DISCONNECTED);
}

void WiFiConnectionManager::reset() {
  if (nvs_manager) {
    nvs_manager->clearAllProfiles();
  }
  WiFi.disconnect();
  setState(WiFiConnectionState::DISCONNECTED);
}

bool WiFiConnectionManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiConnectionManager::getConnectedSSID() const {
  return WiFi.SSID();
}

IPAddress WiFiConnectionManager::getIP() const {
  return WiFi.localIP();
}

bool WiFiConnectionManager::connectToNetwork(const char* ssid, const char* password) {
  setState(WiFiConnectionState::CONNECTING);
  
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && 
         millis() - start < config.connection_timeout_ms) {
    delay(500);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    setState(WiFiConnectionState::CONNECTED);
    if (connected_callback) {
      connected_callback(ssid, WiFi.localIP());
    }
    return true;
  }
  
  setState(WiFiConnectionState::FAILED);
  return false;
}

bool WiFiConnectionManager::startProvisioning() {
  setState(WiFiConnectionState::PROVISIONING);
  
  if (prov_callback) {
    prov_callback();
  }
  
  if (prov_client) {
    prov_client->startProvisioning();
  }
  
  return true;
}

void WiFiConnectionManager::stopProvisioning() {
  if (prov_client) {
    prov_client->stopProvisioning();
  }
  setState(WiFiConnectionState::DISCONNECTED);
}

std::vector<WiFiNetworkInfo> WiFiConnectionManager::scanNetworks() {
  std::vector<WiFiNetworkInfo> networks;
  
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    WiFiNetworkInfo info;
    info.ssid = WiFi.SSID(i);
    info.rssi = WiFi.RSSI(i);
    info.encryption = WiFi.encryptionType(i);
    networks.push_back(info);
  }
  
  return networks;
}

bool WiFiConnectionManager::saveProfile(const char* ssid, const char* password, int priority) {
  if (!nvs_manager) return false;
  
  return nvs_manager->saveProfile(ssid, password, priority);
}

int WiFiConnectionManager::getProfileCount() {
  if (!nvs_manager) return 0;
  return nvs_manager->getProfileCount();
}

void WiFiConnectionManager::clearAllProfiles() {
  if (nvs_manager) {
    nvs_manager->clearAllProfiles();
  }
}

void WiFiConnectionManager::handle() {
  if (prov_client) {
    prov_client->handleHTTPRequests();
  }
  
  // Auto-reconnect logic
  if (config.auto_reconnect && !isConnected() && current_state == WiFiConnectionState::DISCONNECTED) {
    tryConnectToSavedNetworks();
  }
}

bool WiFiConnectionManager::tryConnectToSavedNetworks() {
  if (!nvs_manager) return false;
  
  std::vector<WiFiProfile> profiles;
  if (!nvs_manager->loadProfiles(profiles) || profiles.empty()) {
    return false;
  }
  
  setState(WiFiConnectionState::SCANNING);
  
  // Try each saved profile
  for (const auto& profile : profiles) {
    if (connectToNetwork(profile.ssid.c_str(), profile.password.c_str())) {
      return true;
    }
  }
  
  setState(WiFiConnectionState::DISCONNECTED);
  return false;
}

void WiFiConnectionManager::setState(WiFiConnectionState new_state) {
  if (current_state != new_state) {
    current_state = new_state;
    if (state_callback) {
      state_callback(new_state);
    }
  }
}

void WiFiConnectionManager::handleProvisioning() {
  if (prov_client) {
    prov_client->handleHTTPRequests();
  }
}

// Static instance for callbacks
WiFiConnectionManager* g_wifiManager = nullptr;
