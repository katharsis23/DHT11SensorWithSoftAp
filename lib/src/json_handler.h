#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include <ArduinoJson.h>
#include <string>

struct WiFiCredentials {
    std::string ssid;
    std::string password;
    
    WiFiCredentials() : ssid(""), password("") {}
    WiFiCredentials(const char* s, const char* p) : ssid(s), password(p) {}
};

class JSON_Handler {
private:
    JsonDocument doc;
    static const int BUFFER_SIZE = 512;
    
public:
    JSON_Handler();
    ~JSON_Handler() = default;
    
    // Deserialization
    bool decode(const char* json_string, size_t length);
    bool decode(const uint8_t* data, size_t length);
    
    // Extract WiFi credentials
    WiFiCredentials getWifiCredentials();
    
    // Check if fields exist
    bool hasSSID() const;
    bool hasPassword() const;
    
    // Serialization
    std::string encode(const WiFiCredentials& creds);
    std::string encodeResponse(const char* status, const char* message);
    
    // Debug
    void printJson() const;
    const JsonDocument& getDocument() const;
};
std::string jsonHandlerToString(const JSON_Handler& handler);
#endif
