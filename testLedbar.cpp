#undef __ARM_FP

#include "mbed.h"

#define PERIOD_WIDTH_MS  20
#define STOP_PULSE_US   1800
#define CW_PULSE_US     2500
#define CCW_PULSE_US    500

#include <cstdint>
#include <string.h> 
#include "mbed.h"
#include "MFRC522.h"

#define WAIT_TIME_MS_0 500 
#define WAIT_TIME_MS_1 1500
DigitalOut led2(PB_15);
DigitalIn pinIn(PB_13);
//DigitalIn pushbutton11(PB_13);
PwmOut testmotor(PA_7);
DigitalIn pushbutton22(PC_12);
DigitalIn pushbutton21(PA_15);
DigitalOut trig_ultra1(PC_4);
DigitalIn  echo_ultra1(PC_5);   // IMPORTANT: PullDown fixes floating issue
// ---------------- CONSTANT DEFINITIONS ----------------
#define TRIG_PULSE_US      20
#define ECHO_STEP_US       58        // ~1 cm per count
#define MAX_ECHO_COUNT     200       // ~2 meters
#define NEAR_THRESHOLD     30     



int main()
{
    testmotor.period_ms(PERIOD_WIDTH_MS);
    testmotor.pulsewidth_us(STOP_PULSE_US);
    while(true){
        if(pushbutton22.read()==0){
            testmotor.pulsewidth_us(2400);
        }else if(pushbutton21.read()==0){
            testmotor.pulsewidth_us(500);
        }
    }
}

/*while(true){
        led2=0;
        if(pinIn.read()==0){
            led2=1;
            printf("Metal Detected");
        }
    }*/



    /*motoor.period_ms(PERIOD_WIDTH_MS);
    motoor.pulsewidth_us(CW_PULSE_US);
    while(true){
        motoor.pulsewidth_us(500);
        thread_sleep_for(1000);
        motoor.pulsewidth_us(1500);
        thread_sleep_for(1000);
        motoor.pulsewidth_us(2400);
        thread_sleep_for(1000);
    }*/

    /*while(true){
        if(buttonpush.read()==1){
            led2=1;
            thread_sleep_for(1000);
            led2=0;
        }
    }*/