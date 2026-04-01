#include "connect_to_wifi.h"
#include <algorithm>
#include <nvs.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern uint8_t flag[10];
// Static members initialization
const char* NVS_Manager::NVS_NAMESPACE = "wifi_mgr";
const char* NVS_Manager::NVS_PROFILES_KEY = "profiles";


NVS_Manager::NVS_Manager() {
  
}

// Save single credentials (simple version)
bool NVS_Manager::saveCredentials(const char* ssid, const char* password) {
  return saveProfile(ssid, password, 5); // Default priority
}

// Load single credentials (loads first profile)
bool NVS_Manager::loadCredentials(String& ssid, String& password) {
  std::vector<WiFiProfile> profiles;
  if (!loadProfiles(profiles) || profiles.empty()) {
    return false;
  }
  
  ssid = profiles[0].ssid;
  password = profiles[0].password;
  return true;
}

// Save profile
bool NVS_Manager::saveProfile(const char* ssid, const char* password, int priority) {
  Serial.printf("[NVS] Saving profile: %s (priority: %d)\n", ssid, priority);
  
  std::vector<WiFiProfile> profiles;
   
  loadProfiles(profiles);
  // Update if exists
  bool exists = false;
  for (auto& profile : profiles) {
    if (profile.ssid == ssid) {
      profile.password = password;
      profile.priority = priority;
      exists = true;
      break;
    }
  }
  
  // Add new
  if (!exists) {
    if (profiles.size() >= MAX_PROFILES) {
      Serial.println("[NVS] Max profiles limit reached");
      return false;
    }
    profiles.push_back(WiFiProfile(ssid, password, priority));
  }
  
  // Sort by priority
  std::sort(profiles.begin(), profiles.end(), 
    [](const WiFiProfile& a, const WiFiProfile& b) {
      return a.priority > b.priority;
    });
  
  // Save
  String json = serializeProfiles(profiles);
  
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    return false;
  }
  
  bool saved = preferences.putString(NVS_PROFILES_KEY, json);
  preferences.end();
  
  Serial.printf("[NVS] %s (total: %d)\n", saved ? "✓ Saved" : "✗ Failed", profiles.size());
  return saved;
}

void printNVSStats() {
    nvs_stats_t stats;
    nvs_get_stats(NULL, &stats);

    Serial.println("------- NVS STATS -------");
    Serial.printf("Total entries: %d\n", stats.total_entries);
    Serial.printf("Used entries:  %d\n", stats.used_entries);
    Serial.printf("Free entries:  %d\n", stats.free_entries);
    Serial.printf("Namespace count: %d\n", stats.namespace_count);
    Serial.println("--------------------------");
}
void checkStack() {
    UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL); // для поточного таску
    Serial.print("Free stack (words): ");
    Serial.println(freeStack);
}

// Load profiles
bool NVS_Manager::loadProfiles(std::vector<WiFiProfile>& profiles) {
  profiles.clear();
  if (!preferences.begin(NVS_NAMESPACE, true)) {
    return false;
  }
  //printNVSStats();
  //checkStack();
  String json = preferences.getString(NVS_PROFILES_KEY, "");
  preferences.end();
  if (json.length() == 0) {
    return false;
  }
  return deserializeProfiles(json, profiles);
}

// Delete profile
bool NVS_Manager::deleteProfile(const char* ssid) {
  std::vector<WiFiProfile> profiles;
  if (!loadProfiles(profiles)) {
    return false;
  }
  
  auto it = std::remove_if(profiles.begin(), profiles.end(),
    [ssid](const WiFiProfile& p) { return p.ssid == ssid; });
  
  if (it == profiles.end()) {
    return false;
  }
  
  profiles.erase(it, profiles.end());
  
  String json = serializeProfiles(profiles);
  
  if (!preferences.begin(NVS_NAMESPACE, false)) {
    return false;
  }
  
  bool saved = preferences.putString(NVS_PROFILES_KEY, json);
  preferences.end();
  
  Serial.printf("[NVS] Profile deleted (remaining: %d)\n", profiles.size());
  return saved;
}

// Has profile
bool NVS_Manager::hasProfile(const char* ssid) {
  std::vector<WiFiProfile> profiles;
  if (!loadProfiles(profiles)) {
    return false;
  }
  
  for (const auto& profile : profiles) {
    if (profile.ssid == ssid) {
      return true;
    }
  }
  return false;
}

// Get profile count
int NVS_Manager::getProfileCount() {
  std::vector<WiFiProfile> profiles;
  if (!loadProfiles(profiles)) {
    return 0;
  }
  return profiles.size();
}

// Clear all profiles
void NVS_Manager::clearAllProfiles() {
  if (preferences.begin(NVS_NAMESPACE, false)) {
    String existing = preferences.getString(NVS_PROFILES_KEY, "");
    if (existing.length() > 0) {
      preferences.remove(NVS_PROFILES_KEY);
      Serial.println("[NVS] All profiles cleared");
    } else {
      Serial.println("[NVS] No profiles found, nothing to clear");
    }
    preferences.end();
  }
}

// Connect to any available (РЕАЛІЗАЦІЯ!)
bool NVS_Manager::connectToAnyAvailable() {
  Serial.println("[WiFi] Smart connect: scanning...");
  
  std::vector<WiFiProfile> profiles;
  if (!loadProfiles(profiles)) {
    Serial.println("[WiFi] No saved profiles");
    return false;
  }
  
  auto availableNetworks = scanNetworks();
  
  if (availableNetworks.empty()) {
    Serial.println("[WiFi] No networks found");
    return false;
  }
  
  // Find best available
  WiFiProfile* bestProfile = nullptr;
  int bestRSSI = -100;
  
  for (auto& profile : profiles) {
    for (const auto& network : availableNetworks) {
      if (network.ssid == profile.ssid) {
        if (bestProfile == nullptr || 
            profile.priority > bestProfile->priority ||
            (profile.priority == bestProfile->priority && network.rssi > bestRSSI)) {
          bestProfile = &profile;
          bestRSSI = network.rssi;
        }
      }
    }
  }
  
  if (bestProfile == nullptr) {
    Serial.println("[WiFi] No known networks available");
    return false;
  }
  
  Serial.printf("[WiFi] Connecting to: %s\n", bestProfile->ssid.c_str());
  return connectToWiFi(bestProfile->ssid.c_str(), bestProfile->password.c_str());
}

// Connect to profile
bool NVS_Manager::connectToProfile(const WiFiProfile& profile) {
  return connectToWiFi(profile.ssid.c_str(), profile.password.c_str());
}

// Scan networks
std::vector<WiFiNetworkInfo> NVS_Manager::scanNetworks() {
  Serial.println("[WiFi] Scanning networks...");
  
  std::vector<WiFiNetworkInfo> networks;
  int numNetworks = WiFi.scanNetworks();
  
  if (numNetworks <= 0) {
    Serial.println("[WiFi] No networks found");
    return networks;
  }
  
  
  Serial.printf("[WiFi] Found %d networks\n", numNetworks);
  
  for (int i = 0; i < numNetworks; i++) {
    WiFiNetworkInfo info;
    info.ssid = WiFi.SSID(i);
    info.rssi = WiFi.RSSI(i);
    info.encryption = WiFi.encryptionType(i);
    info.channel = WiFi.channel(i);
    networks.push_back(info);
  }
  
  WiFi.scanDelete();
  return networks;
}

// Connect to WiFi
bool NVS_Manager::connectToWiFi(const char* ssid, const char* password, uint32_t timeout_ms) {
  
  Serial.printf("[WiFi] Connecting to: %s\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
    delay(100);
    if ((millis() - start) % 1000 == 0) {
      Serial.println();
    }
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    flag[1]=1;
    Serial.printf("[WiFi] ✓ Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.println("[WiFi] ✗ Connection failed");
    return false;
  }
}

// Is connected
bool NVS_Manager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

// Disconnect
bool NVS_Manager::disconnect() {

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("[WiFi] Disconnected");
  return true;
}

// Serialize profiles to JSON
String NVS_Manager::serializeProfiles(const std::vector<WiFiProfile>& profiles) {
  StaticJsonDocument<2048> doc;
  JsonArray array = doc.to<JsonArray>();
  
  for (const auto& profile : profiles) {
    JsonObject obj = array.createNestedObject();
    obj["ssid"] = profile.ssid;
    obj["password"] = profile.password;
    obj["priority"] = profile.priority;
  }
  
  String json;
  serializeJson(doc, json);
  return json;
}

// Deserialize profiles from JSON
bool NVS_Manager::deserializeProfiles(const String& json, std::vector<WiFiProfile>& profiles) {
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.printf("[NVS] JSON error: %s\n", error.c_str());
    return false;
  }
  
  JsonArray array = doc.as<JsonArray>();
  
  for (JsonObject obj : array) {
    String ssid = obj["ssid"] | "";
    String password = obj["password"] | "";
    
    int priority = obj["priority"] | 0;
    
    if (ssid.length() > 0) {
      profiles.push_back(WiFiProfile(ssid, password, priority));
    }
  }
  
  Serial.printf("[NVS] Loaded %d profiles\n", profiles.size());
  return true;
}
