/* 

Things to change when you are changing the html:

1. Content-Length is already automatically calculated,
   so you do not need to change anything for it 

2. You many need to change the buffer size, and the 
   current limits may not be enough
   -> char payloadBuf[1024]
   * take note that that payloadBuf >= http headers + html;
   i.e., html max = 900 bytes
   
   if headers > 900 bytes, snprintf() will truncate,
   browser will show 'ERR_CONTENT_LENGTH_MISMATCH'

   THEREFORE, U MUST DO: char payloadBuf[2048]


*/



#undef __ARM_FP


#include "mbed.h"
#include <cstring>

#undef __ARM_FP

// ===== UART Pins =====
#define ESP_TX PC_10
#define ESP_RX PC_11

BufferedSerial esp(ESP_TX, ESP_RX, 115200); // communications with esp01
BufferedSerial pc(USBTX, USBRX, 115200); // usb serial console (for debugging)

FileHandle *mbed_override_console() { return &pc; } // displays 

// ===== BUFFERS =====
#define RX_BUF 1024
#define TX_BUF 1024

char rxBuf[RX_BUF];
char txBuf[TX_BUF];

// ===== WIFI CREDENTIALS =====
const char WIFI_SSID[] = "Hi";
const char WIFI_PASS[] = "PHANG9940h";

// ====== ACCUMULATOR BUFFER ==========
#define ACC_BUF 4096
static char acc[ACC_BUF];
static int acc_len = 0;

// ================= UTILITIES =================

// Clears old junk from the ESP;
// Prevents previous responses from confusing the next command
void flush_esp()
{
    while (esp.readable()) {
        esp.read(rxBuf, sizeof(rxBuf));
    }
}

// wait for ESP confirmantion (sync stm32 to esp)
// for synchronous code; only after timeout it moves on
// waits for esp output and prints to console
bool wait_for(const char *token, int timeout_ms = 5000)
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

// send AT commands, prints to pc and wait for 'OK' response
// send_cmd('command', 'reply to expect', 'timeout');
void send_cmd(const char *cmd, const char *expect = "OK", int delay_ms = 2000)
{
    flush_esp();
    esp.write(cmd, strlen(cmd));
    pc.write(cmd, strlen(cmd));
    wait_for(expect, delay_ms);
}

// ================= WIFI SETUP =================

void setup_wifi()
{
    printf("\r\n--- ESP SETUP START ---\r\n");

    send_cmd("AT+RST\r\n", "ready", 8000); // resets esp
    send_cmd("AT\r\n", "OK"); // test communication
    send_cmd("AT+CWMODE=1\r\n", "OK"); // mode 1 = connect to router
    
    // ------ Connect to Wifi -------
    snprintf(txBuf, sizeof(txBuf),
             "AT+CWJAP=\"%s\",\"%s\"\r\n",
             WIFI_SSID, WIFI_PASS);

    //  WAIT UNTIL ESP IS IDLE
    send_cmd(txBuf, "OK", 15000);

    thread_sleep_for(500);   // VERY IMPORTANT

    // ===== GET IP ADDRESS =====
    printf("\r\nESP IP Address:\r\n");
    send_cmd("AT+CIFSR\r\n", "OK", 3000);

    send_cmd("AT+CIPMUX=1\r\n", "OK");

    // Safe: disable first
    send_cmd("AT+CIPSERVER=0\r\n", "OK");
    send_cmd("AT+CIPSERVER=1,80\r\n", "OK");

    printf("\r\n--- ESP READY ---\r\n");
}

// ================= HTTP RESPONSE =================

// improved send_http: robustly does CIPSEND -> payload -> wait for SEND OK -> close
void send_http(int conn_id)
{
    // edits what the browser displays
    const char html[] =
        "<!DOCTYPE html>"
        "<html><head><title>STM32</title></head>"
        "<body style='font-family:sans-serif;'>"
        "<h1>Hello World from STM32 + ESP01</h1>"
        "</body></html>";

    // build full HTTP response into payloadBuf
    // length must be properly modified
    // to ensure that http has correct headers, length etc
    char payloadBuf[1024];
    int payloadLen = snprintf(payloadBuf, sizeof(payloadBuf),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n"
        "%s",
        (int)strlen(html), html);

    if (payloadLen <= 0 || payloadLen >= (int)sizeof(payloadBuf)) {
        pc.write("Payload build error\r\n", 21);
        return;
    }

    // Build CIPSEND command using the payload length
    char cmdBuf[128];
    int cmdLen = snprintf(cmdBuf, sizeof(cmdBuf), "AT+CIPSEND=%d,%d\r\n", conn_id, payloadLen);

    // send the CIPSEND command
    flush_esp();                  // clear any leftover bytes
    esp.write(cmdBuf, cmdLen);
    pc.write(cmdBuf, cmdLen);

    // wait for the '>' prompt (ESP ready to accept payload)
    if (!wait_for(">", 3000)) {
        pc.write("No '>' prompt from ESP\r\n", 26);
        // attempt graceful close if possible
        int closeLen = snprintf(cmdBuf, sizeof(cmdBuf), "AT+CIPCLOSE=%d\r\n", conn_id);
        esp.write(cmdBuf, closeLen);
        pc.write(cmdBuf, closeLen);
        return;
    }

    // send the actual HTTP payload
    esp.write(payloadBuf, payloadLen);
    pc.write(payloadBuf, payloadLen);

    // wait for "SEND OK" or "ERROR"
    if (!wait_for("SEND OK", 3000)) {
        pc.write("Warning: no SEND OK (or timed out)\r\n", 36);
    }

    // close the connection and wait for OK
    int closeLen = snprintf(cmdBuf, sizeof(cmdBuf), "AT+CIPCLOSE=%d\r\n", conn_id);
    esp.write(cmdBuf, closeLen);
    pc.write(cmdBuf, closeLen);
    wait_for("OK", 2000);
}

// ================= MAIN =================
int main()
{
    printf("\r\nSTM32 ESP Web Server\r\n");

    setup_wifi();

    while (true)
    {
        // Read any available bytes from UART into a temp buffer
        if (esp.readable()) {
            int n = esp.read(rxBuf, sizeof(rxBuf) - 1);
            if (n > 0) {
                // append to accumulator (ensure no overflow)
                if (acc_len + n >= ACC_BUF) {
                    // buffer full -> drop oldest data (simple recovery)
                    // move tail to start to free space
                    int keep = ACC_BUF / 2;
                    memmove(acc, acc + acc_len - keep, keep);
                    acc_len = keep;
                }
                memcpy(acc + acc_len, rxBuf, n);
                acc_len += n;
                acc[acc_len] = '\0';

                // also print to console for debugging
                pc.write(rxBuf, n);
            }
        } else {
            thread_sleep_for(50);
            continue;
        }

        // Process all +IPD occurrences in accumulator
        while (true) {
            char *p = strstr(acc, "+IPD,");
            if (!p) break; // nothing to do

            // Ensure we have the full header (find colon that ends header)
            char *colon = strchr(p, ':');
            if (!colon) {
                // header not complete yet -> wait for more bytes
                break;
            }

            // Parse connection id and length manually from the header.
            // Format: +IPD,<conn_id>,<len>:
            int conn_id = -1;
            int data_len = 0;
            char *q = p + strlen("+IPD,");
            char *endptr = nullptr;

            // parse conn_id
            conn_id = (int)strtol(q, &endptr, 10);
            if (endptr == q || *endptr != ',') {
                // parsing failed: remove this +IPD token to avoid infinite loop
                // (possible corruption) or break to wait for more data
                pc.write("Failed to parse conn_id in +IPD header\r\n", 44);
                // remove up to colon to recover
                int removeBytes = (colon + 1) - acc;
                // note thet the memmove command is very memory intensive
                memmove(acc, acc + removeBytes, acc_len - removeBytes);
                acc_len -= removeBytes;
                acc[acc_len] = '\0';
                continue;
            }
            q = endptr + 1;

            // parse data_len
            data_len = (int)strtol(q, &endptr, 10);
            if (endptr == q || *endptr != ':') {
                pc.write("Failed to parse data_len in +IPD header\r\n", 44);
                // remove up to colon to recover
                int removeBytes = (colon + 1) - acc;
                memmove(acc, acc + removeBytes, acc_len - removeBytes);
                acc_len -= removeBytes;
                acc[acc_len] = '\0';
                continue;
            }

            // At this point we have conn_id and data_len.
            // Check if the full payload (data_len bytes) is present in the accumulator after the colon
            int header_index = p - acc;          // start index of +IPD
            int payload_start = (colon - acc) + 1; // index where payload starts
            int bytes_available_after_header = acc_len - payload_start;

            if (bytes_available_after_header < data_len) {
                // not all payload bytes have arrived yet -> wait for more data
                break;
            }

            // We have the full HTTP request payload available (or at least the number of bytes ESP reported).
            // Option: inspect payload (acc + payload_start) if you need to examine GET path etc.
            // For now we respond immediately on this connection id.

            // Avoid responding to favicon to reduce noise
            if (strstr(acc + payload_start, "favicon.ico")) {
                // close this connection and remove processed bytes
                int cmdLen = snprintf(txBuf, sizeof(txBuf), "AT+CIPCLOSE=%d\r\n", conn_id);
                esp.write(txBuf, cmdLen);
                pc.write(txBuf, cmdLen);
            } else {
                send_http(conn_id);
            }

            // Remove processed bytes (header + data_len bytes) from accumulator
            int removeBytes = payload_start + data_len;
            if (removeBytes < acc_len) {
                memmove(acc, acc + removeBytes, acc_len - removeBytes);
                acc_len -= removeBytes;
                acc[acc_len] = '\0';
            } else {
                // exactly consumed everything
                acc_len = 0;
                acc[0] = '\0';
            }

            // Continue loop to see if there are more +IPD messages buffered
        } // end process +IPD occurrences
    }
}
