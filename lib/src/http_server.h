#pragma once
#include <WebServer.h>
#include "json_handler.h"
#include "sensor_manager.h"

#include "wifi_connection_wrapper.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiProv.h>
#include <WebServer.h>
#include <ArduinoJson.h>
void startMainServer();
void handleMainServer();
void stopMainServer();