#ifndef MYESP32PING_H
#define MYESP32PING_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>

// Ping wrapper functions
class Ping2 {
public:
    static bool ping2(IPAddress ip, int count = 1);
    static void ping_recv_async_print();
};

#endif // MYESP32PING_H
