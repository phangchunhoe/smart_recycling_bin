/* 
   wifi_utilities.h
   Header file for WiFi and HTTP utilities
*/

#ifndef WIFI_UTILITIES_H
#define WIFI_UTILITIES_H

#undef __ARM_FP

#include <cstdint>
#include "mbed.h"

// ===== BUFFER SIZES =====
#define RX_BUF 512
#define TX_BUF 512
#define ACC_BUF 4096
#define MAX_PENDING 5

// ===== GLOBAL OBJECTS =====
extern BufferedSerial esp;
extern BufferedSerial pc;

// ===== BIN CAPACITY VALUES =====
extern int plastic;
extern int paper;
extern int metal;

// ===== WIFI CREDENTIALS =====
extern const char WIFI_SSID[];
extern const char WIFI_PASS[];

// ===== STRUCTURES =====
struct PendingRequest {
    int conn_id;
    bool is_favicon;
    bool valid;
    uint32_t queue_time;
};

// ===== FUNCTION DECLARATIONS =====

// Utility functions
void flush_esp();
bool wait_for(const char *token, int timeout_ms = 5000);
void send_cmd(const char *cmd, const char *expect = "OK", int delay_ms = 2000);

// Queue management
bool is_already_queued(int conn_id);
void queue_request(int conn_id, bool is_favicon, uint32_t now);
void cleanup_queue(uint32_t now);
bool get_next_request(int *conn_id, bool *is_favicon);

// WiFi setup
void setup_wifi();

// HTTP response functions
void send_favicon(int conn_id);
void send_http(int conn_id);
void process_requests_loop(uint32_t now);

// Connection state management functions
void track_connection_states(const char *data, int data_len, uint32_t now);
void timeout_inactive_connections(uint32_t now);

// IPD message parsing
void process_ipd_messages(char *acc, int *acc_len, int *acc_start, uint32_t now);

// Connection state management
extern bool conn_busy[5];
extern bool conn_closed[5];
extern uint32_t conn_last_activity[5];

// Request queue
extern PendingRequest pending_requests[MAX_PENDING];
extern int pending_count;

#endif // WIFI_UTILITIES_H