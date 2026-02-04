/* 
   main.cpp
   Main application loop for STM32 ESP Web Server
*/

#include <cstdint>
#undef __ARM_FP

#include "wifi.h"
#include <cstring>
//#include "doorop.h"

using namespace std::chrono;

// Local buffers for main loop
static char rxBuf_main[RX_BUF];
static char acc[ACC_BUF];
static int acc_len = 0;
static int acc_start = 0;

// ================= MAIN =================
void web(double papersh, double plasticsh, double metalsh)
{
    printf("\r\nSTM32 ESP Web Server - Immediate Processing Version\r\n");
    int webwait=0;

    // Setup WiFi and initialize connections
    setup_wifi();
    
    for (int i = 0; i < 5; i++) {
        conn_busy[i] = false;
        conn_closed[i] = true;
        conn_last_activity[i] = 0;
    }

    uint32_t last_cleanup_time = 0;
    printf("\r\n*** Waiting for connections ***\r\n\r\n");

    // Main event loop
    while (webwait<5000)
    {
        thread_sleep_for(10);
        uint32_t now = rtos::Kernel::Clock::now().time_since_epoch().count() / 1000;

        // ---------------
            Paper=papersh;
            Plastic=plasticsh;
            Metal=metalsh;
        // --------------

        // Read incoming data from ESP
        if (esp.readable()) {
            int n = esp.read(rxBuf_main, sizeof(rxBuf_main) - 1);
            if (n > 0) {
                // Manage accumulator buffer
                if (acc_len + n >= ACC_BUF) {
                    int keep = ACC_BUF / 2;
                    memmove(acc, acc + acc_len - keep, keep);
                    acc_len = keep;
                    acc_start = 0;
                }
                memcpy(acc + acc_len, rxBuf_main, n);
                acc_len += n;
                acc[acc_len] = '\0';

                pc.write(rxBuf_main, n);
                
                // Track connection open/close events
                track_connection_states(rxBuf_main, n, now);
            }
        }

        // Parse and queue incoming HTTP requests
        process_ipd_messages(acc, &acc_len, &acc_start, now);

        // Periodic cleanup of stale requests
        if ((now - last_cleanup_time) > 500) {
            cleanup_queue(now);
            last_cleanup_time = now;
        }

        // Process all pending HTTP requests
        if (pending_count > 0) {
            process_requests_loop(now);
        }
        
        // Close inactive connections
        timeout_inactive_connections(now);
        webwait+=1;
    }
}