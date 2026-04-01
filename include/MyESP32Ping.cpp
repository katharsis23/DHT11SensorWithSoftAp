#include "MyESP32Ping.h"

bool Ping2::ping2(IPAddress ip, int count) {
    return Ping.ping(ip, count);
}

void Ping2::ping_recv_async_print() {
    // Implementation for async ping result printing
    // For now, just a placeholder
    Serial.println("[Ping] Async ping results printed");
}
