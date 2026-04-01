#ifndef WORK_OPTIONS_H
#define WORK_OPTIONS_H

#include <Arduino.h>

// Work options and configurations
extern uint8_t flag[10];
extern IPAddress* ptr;
extern size_t ptr_size;

// Network scanning functions
void sendPaket();
void ping_recv_async_print();

#endif // WORK_OPTIONS_H
