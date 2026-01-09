#undef __ARM_FP
#include "mbed.h"
#include "trashlevel.h"
#define WAIT_TIME_US_0 10
#define WAIT_TIME_US_1 20
#define WAIT_TIME_US_2 58
#define WAIT_TIME_MS_0 25
#define WAIT_TIME_MS_1 30
#define WAIT_TIME_MS_2 100
#define WAIT_TIME_MS_3 300
#define WAIT_TIME_MS_4 500
#define WAIT_TIME_MS_5 1000
#define WAIT_TIME_MS_6 2000
#define WAIT_TIME_MS_7 3000
#define WAIT_TIME_MS_8 4000

#define PERIOD_WIDTH_MS     20      // 20 ms period
#define STOP_PULSE_US       1500    // Stop
#define CW_PULSE_US         2400    // Clockwise
#define CCW_PULSE_US        500   
//input of IR sensor data
//DigitalOut Trig(PC_1);
//DigitalIn Echo(PC_2);
//turn on/off the on-board LED: PB14
DigitalOut LED_PB14(PB_14);
unsigned char reggin = 0;
float objDistance = 0;
// ===== Hardware =====
PwmOut Servomotor(PA_6);
DigitalIn button_PC12(PC_12);
DigitalIn button_PA15(PA_15);
int main()
{
    Servomotor.period_ms(PERIOD_WIDTH_MS);

    // Start stopped
    Servomotor.pulsewidth_us(STOP_PULSE_US);
    int level=trashlevel();
    
    if (!level)
    {
           
         Servomotor.resume();
        printf("Throw your trash");
        Servomotor.pulsewidth_us(CW_PULSE_US );
        thread_sleep_for(1500);
        Servomotor.suspend();
        thread_sleep_for(3000);
        Servomotor.resume();
        Servomotor.pulsewidth_us(CCW_PULSE_US);
        printf("Door closing");
        thread_sleep_for(1500);
        Servomotor.suspend();

    }

   if(detectedtype==1){
        Servomotor.pulsewidth_us(CCW_PULSE_US);
    }else if(detectedtype==3){
        Servomotor.pulsewidth_us(CW_PULSE_US);
            
    }

}