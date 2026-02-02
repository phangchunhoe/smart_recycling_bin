#undef __ARM_FP
#include "mbed.h"

// ---------------- CONSTANT DEFINITIONS ----------------
#define TRIG_PULSE_US      20
#define ECHO_STEP_US       58        // ~1 cm per count
#define MAX_ECHO_COUNT     200       // ~2 meters
#define NEAR_THRESHOLD     30        // ~30 cm

// ---------------- PIN DEFINITIONS ----------------
DigitalOut trig_ultra(PC_4);
DigitalIn  echo_ultra(PC_5);   // IMPORTANT: PullDown fixes floating issue



unsigned int echoCount = 0;
float distance_cm = 0.0f;

Timer echoTimer2;
int ultrasound1()
{
    trig_ultra = 0;
        wait_us(2);
        trig_ultra = 1;
        wait_us(TRIG_PULSE_US);
        trig_ultra = 0;

        // Wait for echo to go HIGH
        while (echo_ultra == 0);

        // Measure echo HIGH time
        echoTimer2.reset();
        echoTimer2.start();

        while (echo_ultra == 1);

        echoTimer2.stop();

        float echo_us = echoTimer2.elapsed_time().count();

        // Distance in cm
        float distance_cm = (echo_us * 0.0343f) / 2.0f;
        printf("Distance for ultrasound1 is %f: ",distance_cm);

    if (distance_cm < 15 && distance_cm > 2) {
        
        printf("Distance for ultrasound1 is %f: ",distance_cm);
        return 1;
    } else {
        
        return 0;
    }
}