# ESP32 DHT11 Monitor - API Documentation

## 🌐 Overview
ESP32 DHT11 Monitor provides RESTful HTTP API endpoints for sensor data, device information, and WiFi management. All endpoints return JSON responses unless specified otherwise.

**Base URL:** `http://[ESP32_IP]` or `http://esp32-monitor.local`  
**Port:** 80  
**Content-Type:** `application/json` (unless specified)

---

## 📡 API Endpoints

### 1. Device Information
#### `GET /`
**Description:** Get ESP32 IP address  
**Content-Type:** `text/plain`  
**Response:** IP address as plain text

**Example Response:**
```
192.168.1.100
```

**Curl Command:**
```bash
curl http://192.168.1.100/
```

---

#### `GET /device`
**Description:** Get device information and status  
**Response:** JSON with device details

**Example Response:**
```json
{
  "status": "ok",
  "device": "ESP32-DHT11-Monitor",
  "ip": "192.168.1.100",
  "mac": "d4:e9:f4:65:3a:c8",
  "wifi_connected": true,
  "ssid": "HomeWiFi",
  "uptime_ms": 45000,
  "free_heap": 245760,
  "led_state": 1
}
```

**Curl Command:**
```bash
curl http://192.168.1.100/device
```

---

#### `GET /proto-ver`
**Description:** Protocol version check  
**Content-Type:** `text/plain`  
**Response:** "OK"

**Curl Command:**
```bash
curl http://192.168.1.100/proto-ver
```

---

#### `GET /hello`
**Description:** Test endpoint for connectivity  
**Response:** JSON greeting

**Example Response:**
```json
{
  "message": "Hello from ESP32 DHT11 Monitor!",
  "status": "online",
  "timestamp": 1234567890
}
```

**Curl Command:**
```bash
curl http://191.168.1.100/hello
```

---

### 2. Sensor Data

#### `GET /sensors`
**Description:** Get all sensor data in one response  
**Response:** JSON with all sensor readings

**Example Response:**
```json
{
  "timestamp": 1234567890,
  "valid": true,
  "dht": {
    "temperature": 23.5,
    "humidity": 65.2,
    "valid": true,
    "timestamp": 1234567890
  }
}
```

**Curl Command:**
```bash
curl http://192.168.1.100/sensors
```

---

#### `GET /sensors/dht`
**Description:** Get DHT11 sensor data only  
**Response:** JSON with temperature and humidity

**Example Response:**
```json
{
  "temperature": 23.5,
  "humidity": 65.2,
  "valid": true,
  "timestamp": 1234567890
}
```

**Curl Command:**
```bash
curl http://192.168.1.100/sensors/dht
```

---

### 3. WiFi Management

#### `DELETE /delete-credentials`
**Description:** Delete all saved WiFi profiles from NVS  
**Response:** JSON confirmation

**Example Response:**
```json
{
  "status": "success",
  "message": "All WiFi credentials deleted",
  "profiles_cleared": 3
}
```

**Curl Command:**
```bash
curl -X DELETE http://192.168.1.100/delete-credentials
```

---

#### `GET /get-mDNS`
**Description:** Get mDNS hostname information  
**Response:** JSON with mDNS details

**Example Response:**
```json
{
  "hostname": "esp32-monitor",
  "domain": "local",
  "full_hostname": "esp32-monitor.local",
  "mdns_enabled": true
}
```

**Curl Command:**
```bash
curl http://192.168.1.100/get-mDNS
```

---

## 🔍 Response Status Codes

| Status Code | Description | Example Usage |
|-------------|-------------|---------------|
| **200 OK** | Request successful | All GET endpoints |
| **503 Service Unavailable** | Sensor not initialized or invalid data | `/sensors` when sensor fails |
| **404 Not Found** | Endpoint doesn't exist | Invalid URL |
| **500 Internal Server Error** | Server error | Critical system failure |

---

## 📊 Sensor Data Structure

### DHT11 Sensor Fields
| Field | Type | Description | Range |
|-------|------|-------------|-------|
| `temperature` | float | Temperature in Celsius | 0-50°C |
| `humidity` | float | Relative humidity in % | 20-90% |
| `valid` | boolean | Data validity flag | true/false |
| `timestamp` | integer | Unix timestamp (ms) | - |

### Device Status Fields
| Field | Type | Description |
|-------|------|-------------|
| `status` | string | Device status ("ok", "error") |
| `device` | string | Device name |
| `ip` | string | Current IP address |
| `mac` | string | MAC address |
| `wifi_connected` | boolean | WiFi connection status |
| `ssid` | string | Connected WiFi network name |
| `uptime_ms` | integer | System uptime in milliseconds |
| `free_heap` | integer | Available heap memory in bytes |
| `led_state` | integer | LED status (0=off, 1=on) |

---

## 🧪 Testing Examples

### Basic Connectivity Test
```bash
# Test if device is online
curl http://esp32-monitor.local/hello
```

### Get All Sensor Data
```bash
# Get complete sensor readings
curl http://esp32-monitor.local/sensors | jq
```

### Monitor Temperature Continuously
```bash
# Watch temperature changes every 5 seconds
watch -n 5 'curl -s http://esp32-monitor.local/sensors/dht | jq .temperature'
```

### Check Device Status
```bash
# Get full device information
curl http://esp32-monitor.local/device | jq
```

### Reset WiFi Settings
```bash
# Clear all saved WiFi profiles
curl -X DELETE http://esp32-monitor.local/delete-credentials
```

---

## 🔧 Error Handling

### Sensor Not Available
```json
{
  "error": "Sensors not initialized",
  "status": "error"
}
```

### Invalid Sensor Data
```json
{
  "error": "DHT sensor data not valid",
  "status": "error"
}
```

### WiFi Not Connected
```json
{
  "error": "WiFi not connected",
  "status": "error",
  "wifi_connected": false
}
```

---

## 📱 Quick Testing Script

Save as `test_esp32_api.sh`:
```bash
#!/bin/bash

ESP32_IP="192.168.1.100"  # Change to your ESP32 IP
ESP32_HOST="esp32-monitor.local"

echo "🔍 Testing ESP32 DHT11 Monitor API"
echo "=================================="

echo "📍 Basic connectivity test..."
curl -s http://$ESP32_IP/hello | jq

echo "📡 Device information..."
curl -s http://$ESP32_IP/device | jq

echo "🌡️ Sensor data..."
curl -s http://$ESP32_IP/sensors | jq

echo "💧 DHT11 specific data..."
curl -s http://$ESP32_IP/sensors/dht | jq

echo "🔗 mDNS information..."
curl -s http://$ESP32_IP/get-mDNS | jq

echo "✅ API testing complete!"
```

Make executable and run:
```bash
chmod +x test_esp32_api.sh
./test_esp32_api.sh
```

---

## 🚀 Performance Notes

- **Sensor Read Interval:** 2 seconds (DHT11 limitation)
- **HTTP Response Time:** < 100ms typical
- **Concurrent Connections:** Up to 4 simultaneous clients
- **Memory Usage:** ~45KB RAM, ~850KB Flash
- **WiFi Reconnection:** Automatic with 20s timeout

---

## 📝 Development Tips

### Using jq for Pretty JSON
```bash
# Install jq: sudo apt install jq (Ubuntu/Debian)
curl http://esp32-monitor.local/sensors | jq
```

### Monitoring Real-time Data
```bash
# Continuous monitoring with timestamps
while true; do
  echo "$(date): $(curl -s http://esp32-monitor.local/sensors/dht | jq -r .temperature)°C"
  sleep 5
done
```

### Batch Testing
```bash
# Test all endpoints in sequence
endpoints=("/hello" "/device" "/sensors" "/sensors/dht" "/get-mDNS")
for endpoint in "${endpoints[@]}"; do
  echo "Testing $endpoint:"
  curl -s "http://esp32-monitor.local$endpoint" | jq
  echo "---"
done
```

---

## 🔒 Security Notes

- **No Authentication:** API is open on local network
- **No HTTPS:** Only HTTP (unencrypted)
- **Open WiFi Provisioning:** "ESP32_PROV" network has no password
- **Recommendation:** Use only on trusted local networks

---

**📞 Support:** Check serial monitor (115200 baud) for detailed debug information
