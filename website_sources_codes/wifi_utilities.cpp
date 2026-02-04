/* 
   wifi_utilities.cpp
   Implementation of WiFi and HTTP utilities
*/

#include <cstdint>
#undef __ARM_FP

#include "wifi.h"
#include <cstring>

using namespace std::chrono;

// ===== UART Pin Definitions =====
#define ESP_TX PC_10
#define ESP_RX PC_11

// ===== GLOBAL OBJECTS =====
BufferedSerial esp(ESP_TX, ESP_RX, 115200);
BufferedSerial pc(USBTX, USBRX, 115200);

FileHandle *mbed_override_console() { return &pc; }

// ===== BUFFERS =====
char rxBuf[RX_BUF];
char txBuf[TX_BUF];

// ====== BIN CAPACITY VALUES (0-100) ========
int Plastic;
int Paper;
int Metal;

// ===== WIFI CREDENTIALS =====
const char WIFI_SSID[] = "ChongA34";
const char WIFI_PASS[] = "18273645";

// ====== ACCUMULATOR BUFFER ==========
static char acc[ACC_BUF];
static int acc_len = 0;
static int acc_start = 0;

// ============== CONNECTION MANAGEMENT =================
bool conn_busy[5] = {false};
bool conn_closed[5] = {false};
uint32_t conn_last_activity[5] = {0};

PendingRequest pending_requests[MAX_PENDING];
int pending_count = 0;

// ================= UTILITIES =================

void flush_esp()
{
    while (esp.readable()) {
        esp.read(rxBuf, sizeof(rxBuf));
    }
}

bool wait_for(const char *token, int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        if (esp.readable()) {
            int n = esp.read(rxBuf, sizeof(rxBuf) - 1);
            if (n > 0) {
                rxBuf[n] = '\0';
                pc.write(rxBuf, n);
                if (strstr(rxBuf, token)) return true;
            }
        }
        thread_sleep_for(100);
        elapsed += 100;
    }
    return false;
}

void send_cmd(const char *cmd, const char *expect, int delay_ms)
{
    flush_esp();
    esp.write(cmd, strlen(cmd));
    pc.write(cmd, strlen(cmd));
    wait_for(expect, delay_ms);
}

// Check if request already queued for this connection
bool is_already_queued(int conn_id)
{
    for (int i = 0; i < pending_count; i++) {
        if (pending_requests[i].conn_id == conn_id && pending_requests[i].valid) {
            return true;
        }
    }
    return false;
}

void queue_request(int conn_id, bool is_favicon, uint32_t now)
{
    if (is_already_queued(conn_id)) {
        char msg[64];
        snprintf(msg, sizeof(msg), "*** Skipping duplicate queue for conn=%d ***\r\n", conn_id);
        pc.write(msg, strlen(msg));
        return;
    }
    
    if (pending_count < MAX_PENDING) {
        pending_requests[pending_count].conn_id = conn_id;
        pending_requests[pending_count].is_favicon = is_favicon;
        pending_requests[pending_count].valid = true;
        pending_requests[pending_count].queue_time = now;
        pending_count++;
        
        char msg[64];
        snprintf(msg, sizeof(msg), ">>> Queued: conn=%d, favicon=%d, total=%d\r\n", 
                 conn_id, is_favicon, pending_count);
        pc.write(msg, strlen(msg));
    } else {
        pc.write("!!! Queue full, dropping request\r\n", 34);
    }
}

void cleanup_queue(uint32_t now)
{
    int removed = 0;
    for (int i = 0; i < pending_count; ) {
        bool should_remove = false;
        int conn_id = pending_requests[i].conn_id;
        
        if (conn_closed[conn_id]) {
            char msg[64];
            snprintf(msg, sizeof(msg), "*** Removing stale request for closed conn %d ***\r\n", conn_id);
            pc.write(msg, strlen(msg));
            should_remove = true;
        }
        else if ((now - pending_requests[i].queue_time) > 5000) {
            char msg[64];
            snprintf(msg, sizeof(msg), "*** Removing expired request for conn %d ***\r\n", conn_id);
            pc.write(msg, strlen(msg));
            should_remove = true;
        }
        
        if (should_remove) {
            for (int j = i + 1; j < pending_count; j++) {
                pending_requests[j-1] = pending_requests[j];
            }
            pending_count--;
            removed++;
        } else {
            i++;
        }
    }
    
    if (removed > 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "*** Cleaned %d stale requests, %d remaining ***\r\n", 
                 removed, pending_count);
        pc.write(msg, strlen(msg));
    }
}

bool get_next_request(int *conn_id, bool *is_favicon)
{
    if (pending_count == 0) return false;
    
    *conn_id = pending_requests[0].conn_id;
    *is_favicon = pending_requests[0].is_favicon;
    
    char msg[64];
    snprintf(msg, sizeof(msg), "<<< Processing: conn=%d, favicon=%d, queue_size=%d\r\n", 
             *conn_id, *is_favicon, pending_count - 1);
    pc.write(msg, strlen(msg));
    
    for (int i = 1; i < pending_count; i++) {
        pending_requests[i-1] = pending_requests[i];
    }
    pending_count--;
    
    return true;
}

// ================= WIFI SETUP =================

void setup_wifi()
{
    printf("\r\n--- ESP SETUP START ---\r\n");

    send_cmd("AT+RST\r\n", "ready", 8000);
    send_cmd("AT\r\n", "OK");
    send_cmd("AT+CWMODE=1\r\n", "OK");
    
    snprintf(txBuf, sizeof(txBuf),
             "AT+CWJAP=\"%s\",\"%s\"\r\n",
             WIFI_SSID, WIFI_PASS);

    send_cmd(txBuf, "OK", 15000);
    thread_sleep_for(500);

    printf("\r\nESP IP Address:\r\n");
    send_cmd("AT+CIFSR\r\n", "OK", 3000);

    send_cmd("AT+CIPMUX=1\r\n", "OK");
    send_cmd("AT+CIPSERVER=0\r\n", "OK");
    send_cmd("AT+CIPSERVER=1,80\r\n", "OK");

    printf("\r\n--- ESP READY ---\r\n");
}

// ================= HTTP RESPONSE =================

void send_favicon(int conn_id)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "=== Sending favicon to conn=%d ===\r\n", conn_id);
    pc.write(msg, strlen(msg));
    
    conn_busy[conn_id] = true;
    
    const char *resp =
        "HTTP/1.0 204 No Content\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    int len = strlen(resp);
    char cmd[64];
    
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d,%d\r\n", conn_id, len);
    esp.write(cmd, strlen(cmd));
    pc.write(cmd, strlen(cmd));
    
    if (wait_for(">", 1000)) {
        esp.write(resp, len);
        wait_for("SEND OK", 1000);
    } else {
        pc.write("!!! No '>' for favicon\r\n", 24);
    }
    
    thread_sleep_for(50);
    
    char closeCmd[32];
    snprintf(closeCmd, sizeof(closeCmd), "AT+CIPCLOSE=%d\r\n", conn_id);
    esp.write(closeCmd, strlen(closeCmd));
    pc.write(closeCmd, strlen(closeCmd));
    
    conn_busy[conn_id] = false;
    conn_closed[conn_id] = true;
    
    pc.write("=== Favicon done ===\r\n", 22);
}

void send_http(int conn_id)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "=== Sending HTML to conn=%d ===\r\n", conn_id);
    pc.write(msg, strlen(msg));
    
    conn_busy[conn_id] = true;

    char html[2048];

    Plastic = Plastic < 0 ? 0 : (Plastic > 100 ? 100 : Plastic);
    Paper   = Paper   < 0 ? 0 : (Paper   > 100 ? 100 : Paper);
    Metal   = Metal   < 0 ? 0 : (Metal   > 100 ? 100 : Metal);

    snprintf(html, sizeof(html),
        "<!DOCTYPE html>"
        "<html><head><title>STM32</title>"
        "<link rel='stylesheet' href='https://phangchunhoe.github.io/smart_recycling_bin/binCapacity.css'></head>"
        "<body>"
        "<header><p>Smart Recycling Bin</p>"
        "    <button class='ecocoin'>"
        "        <img src='https://phangchunhoe.github.io/smart_recycling_bin/images/ecocoin_start.png'>"
        "        <p class='tooltip'>collect ecocoins</p></button></header>"
        "<main><section class='subheader'>Bin Capacity</section>"
        "    <section class='binCapacities'><div class='binCapacity'>"
        "            <p class='columnHeading'>Plastic</p>"
        "            <p class='fullness'>%d%% Full</p>"
        "            <div class='progress-circle' style='--percent:%d'><span>%d%%</span></div></div>"
        "        <div class='binCapacity'><p class='columnHeading'>Paper</p>"
        "            <p class='fullness'>%d%% Full</p>"
        "            <div class='progress-circle' style='--percent:%d'><span>%d%%</span></div></div>"
        "        <div class='binCapacity'><p class='columnHeading'>Metal</p>"
        "            <p class='fullness'> %d%% Full</p>"
        "            <div class='progress-circle' style='--percent:%d'><span>%d%%</span></div></div>"
        "    </section></main>"
        "<footer>By Phang Chun Hoe, Dennis, Hong Bing and Piyush</footer>"
        "<script src='https://phangchunhoe.github.io/smart_recycling_bin/binCapacity.js'></script>"
        "</body></html>",
    Plastic, Plastic, Plastic,
    Paper,   Paper,   Paper,
    Metal,   Metal,   Metal
    );

    char payloadBuf[4096];
    int html_len = strlen(html);

    int payloadLen = snprintf(payloadBuf, sizeof(payloadBuf),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n"
        "Pragma: no-cache\r\n"
        "Expires: 0\r\n"
        "\r\n"
        "%s",
        html_len,
        html
    );
        
    if (payloadLen <= 0 || payloadLen >= (int)sizeof(payloadBuf)) {
        pc.write("!!! Payload build error\r\n", 25);
        conn_busy[conn_id] = false;
        return;
    }

    char cmdBuf[128];
    snprintf(cmdBuf, sizeof(cmdBuf), "AT+CIPSEND=%d,%d\r\n", conn_id, payloadLen);

    esp.write(cmdBuf, strlen(cmdBuf));
    pc.write(cmdBuf, strlen(cmdBuf));
    
    if (!wait_for(">", 2000)) {
        pc.write("!!! No '>' prompt for HTML\r\n", 28);
        conn_busy[conn_id] = false;
        return;
    }

    esp.write(payloadBuf, payloadLen);
    wait_for("SEND OK", 2000);
    thread_sleep_for(50);

    char closeCmd[32];
    snprintf(closeCmd, sizeof(closeCmd), "AT+CIPCLOSE=%d\r\n", conn_id);
    esp.write(closeCmd, strlen(closeCmd));
    pc.write(closeCmd, strlen(closeCmd));

    conn_busy[conn_id] = false;
    conn_closed[conn_id] = true;
    
    pc.write("=== HTML done ===\r\n", 19);
}

// Process all pending requests that are ready
void process_requests_loop(uint32_t now)
{
    // Keep processing until queue is empty or we hit a busy connection
    int max_iterations = MAX_PENDING; // Prevent infinite loop
    while (pending_count > 0 && max_iterations > 0) {
        max_iterations--;
        
        int conn_id;
        bool is_favicon;
        
        if (!get_next_request(&conn_id, &is_favicon)) {
            break; // Queue empty
        }
        
        // Check if connection is valid
        if (conn_closed[conn_id]) {
            char msg[64];
            snprintf(msg, sizeof(msg), "!!! Conn %d already closed, skipping to next\r\n", conn_id);
            pc.write(msg, strlen(msg));
            continue; // Try next request immediately
        }
        
        if (conn_busy[conn_id]) {
            char msg[64];
            snprintf(msg, sizeof(msg), "!!! Conn %d busy, skipping to next\r\n", conn_id);
            pc.write(msg, strlen(msg));
            continue; // Try next request immediately
        }
        
        // Connection is valid and available - send response
        if (is_favicon) {
            send_favicon(conn_id);
        } else {
            send_http(conn_id);
        }
        
        // Small delay between requests to avoid overwhelming ESP
        thread_sleep_for(20);
    }
}

// ================= CONNECTION STATE TRACKING =================

void track_connection_states(const char *data, int data_len, uint32_t now)
{
    for (int i = 0; i < 5; i++) {
        char closed_msg[16];
        snprintf(closed_msg, sizeof(closed_msg), "%d,CLOSED", i);
        if (strstr(data, closed_msg)) {
            conn_closed[i] = true;
            conn_busy[i] = false;
            
            char msg[64];
            snprintf(msg, sizeof(msg), "*** Conn %d closed ***\r\n", i);
            pc.write(msg, strlen(msg));
        }
        
        char connect_msg[16];
        snprintf(connect_msg, sizeof(connect_msg), "%d,CONNECT", i);
        if (strstr(data, connect_msg)) {
            conn_closed[i] = false;
            conn_busy[i] = false;
            conn_last_activity[i] = now;
            
            char msg[64];
            snprintf(msg, sizeof(msg), "*** Conn %d opened ***\r\n", i);
            pc.write(msg, strlen(msg));
        }
    }
}

void timeout_inactive_connections(uint32_t now)
{
    for (int i = 0; i < 5; i++) {
        if (!conn_closed[i] && !conn_busy[i] && (now - conn_last_activity[i]) > 5000) {
            char msg[64];
            snprintf(msg, sizeof(msg), "!!! Timeout: closing conn %d\r\n", i);
            pc.write(msg, strlen(msg));
            
            char closeCmd[32];
            snprintf(closeCmd, sizeof(closeCmd), "AT+CIPCLOSE=%d\r\n", i);
            esp.write(closeCmd, strlen(closeCmd));
            conn_closed[i] = true;
        }
    }
}

// ================= IPD MESSAGE PARSING =================

void process_ipd_messages(char *acc, int *acc_len, int *acc_start, uint32_t now)
{
    while (true) {
        char *p = strstr(acc + *acc_start, "+IPD,");
        if (!p) break;

        char *colon = strchr(p, ':');
        if (!colon) break;

        int conn_id = -1;
        int data_len = 0;
        char *q = p + strlen("+IPD,");
        char *endptr = nullptr;

        conn_id = (int)strtol(q, &endptr, 10);

        if (conn_id < 0 || conn_id >= 5) {
            pc.write("!!! Invalid conn_id\r\n", 21);
            int removeBytes = (colon + 1) - (acc + *acc_start);
            *acc_start += removeBytes;
            if (*acc_start > ACC_BUF / 2) {
                memmove(acc, acc + *acc_start, *acc_len - *acc_start);
                *acc_len -= *acc_start;
                *acc_start = 0;
            }
            continue;
        }
        
        conn_closed[conn_id] = false;
        conn_last_activity[conn_id] = now;

        if (endptr == q || *endptr != ',') {
            pc.write("!!! Parse error (conn_id)\r\n", 27);
            int removeBytes = (colon + 1) - (acc + *acc_start);
            *acc_start += removeBytes;
            if (*acc_start > ACC_BUF / 2) {
                memmove(acc, acc + *acc_start, *acc_len - *acc_start);
                *acc_len -= *acc_start;
                *acc_start = 0;
            }
            continue;
        }
        q = endptr + 1;

        data_len = (int)strtol(q, &endptr, 10);
        if (endptr == q || *endptr != ':') {
            pc.write("!!! Parse error (data_len)\r\n", 28);
            int removeBytes = (colon + 1) - (acc + *acc_start);
            *acc_start += removeBytes;
            if (*acc_start > ACC_BUF / 2) {
                memmove(acc, acc + *acc_start, *acc_len - *acc_start);
                *acc_len -= *acc_start;
                *acc_start = 0;
            }
            continue;
        }

        int payload_start = (colon - acc) + 1;
        int bytes_available = *acc_len - payload_start;

        if (bytes_available < data_len) {
            break;
        }

        bool is_favicon = (strstr(acc + payload_start, "favicon.ico") != nullptr);
        
        char debug[128];
        snprintf(debug, sizeof(debug), "*** Received request: conn=%d, len=%d, favicon=%d ***\r\n",
                 conn_id, data_len, is_favicon);
        pc.write(debug, strlen(debug));
        
        queue_request(conn_id, is_favicon, now);

        int removeBytes = payload_start + data_len - *acc_start;
        *acc_start += removeBytes;
        
        if (*acc_start > ACC_BUF / 2) {
            memmove(acc, acc + *acc_start, *acc_len - *acc_start);
            *acc_len -= *acc_start;
            *acc_start = 0;
        }
        
        if (*acc_start >= *acc_len) {
            *acc_len = 0;
            *acc_start = 0;
            acc[0] = '\0';
        }
    }
}