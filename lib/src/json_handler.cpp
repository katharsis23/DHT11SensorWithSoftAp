#include "json_handler.h"

JSON_Handler hostJson;

JSON_Handler::JSON_Handler() {
    doc.clear();
}

bool JSON_Handler::decode(const char* json_string, size_t length) {
    return deserializeJson(doc, json_string, length) == DeserializationError::Ok;
}

bool JSON_Handler::decode(const uint8_t* data, size_t length) {
    return decode((const char*)data, length);
}

WiFiCredentials JSON_Handler::getWifiCredentials() {
    WiFiCredentials creds;
    
    if (doc["ssid"].is<const char*>()) {
        creds.ssid = doc["ssid"].as<const char*>();
    }
    if (doc["password"].is<const char*>()) {
        creds.password = doc["password"].as<const char*>();
    }
    
    return creds;
}

bool JSON_Handler::hasSSID() const {
    return doc["ssid"].is<const char*>() && !doc["ssid"].isNull();
}

bool JSON_Handler::hasPassword() const {
    return doc["password"].is<const char*>() && !doc["password"].isNull();
}

std::string JSON_Handler::encode(const WiFiCredentials& creds) {
    JsonDocument response_doc;
    response_doc["ssid"] = creds.ssid;
    response_doc["password"] = creds.password;
    
    std::string result;
    serializeJson(response_doc, result);
    return result;
}

std::string JSON_Handler::encodeResponse(const char* status, const char* message) {
    JsonDocument response_doc;
    response_doc["status"] = status;
    response_doc["message"] = message;
    
    std::string result;
    serializeJson(response_doc, result);
    return result;
}

void JSON_Handler::printJson() const {
    serializeJsonPretty(doc, Serial);
}

const JsonDocument& JSON_Handler::getDocument() const {
    return doc;
}

std::string jsonHandlerToString(const JSON_Handler& handler) {
    JsonDocument doc_copy = handler.getDocument();
    std::string result;
    serializeJson(doc_copy, result);
    return result;
}