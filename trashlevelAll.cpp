#undef __ARM_FP

#include "mbed.h"
//#include "trashlevel.h"
#define STOP_PULSE_US   2100
#define CW_PULSE_US     2500
#define CW1_PULSE_US    1200
#define CW2_PULSE_US    500
#define PERIOD_WIDTH_MS  20

#define TRIG_PULSE_US   20
#define TRIG_DELAY_MS   60

PwmOut motor1(PA_7);
DigitalOut Trig1(PC_1);//5
DigitalIn Echo1(PC_2);//6

Timer echoTimer1;
int binfullcount=0;
int bincheck=0;

int main(){
    
    
while(bincheck<3){
        if(bincheck==0){
            motor1.period_ms(PERIOD_WIDTH_MS);
            motor1.pulsewidth_us(CW_PULSE_US);
        }
        Trig1 = 0;
        wait_us(2);
        Trig1 = 1;
        wait_us(TRIG_PULSE_US);
        Trig1 = 0;

        // Wait for echo to go HIGH
        while (Echo1 == 0);

        // Measure echo HIGH time
        echoTimer1.reset();
        echoTimer1.start();

        while (Echo1 == 1);

        echoTimer1.stop();

        float echo_us = echoTimer1.elapsed_time().count();

        // Distance in cm
        float distance = (echo_us * 0.0343f) / 2.0f;
        if(distance<10){
            binfullcount+=1;
        }
        printf("Distance is  %.2f cm \n",distance);
        thread_sleep_for(1000);
        if(bincheck==1){
            motor1.pulsewidth_us(1000);
            printf("Bincheck is  %.2i \n",bincheck);
        }else if(bincheck==2){
            motor1.pulsewidth_us(2000);
            printf("Bincheck is  %.2i \n",bincheck);
        }
        bincheck+=1;
        
        thread_sleep_for(1000);
    }
}

