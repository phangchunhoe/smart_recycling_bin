#undef __ARM_FP

#include "mbed.h"
#include <cstring>
#include "lcd.h"
#include "trashtype.h"



#define WAIT_TIME_MS_0 500 
#define WAIT_TIME_MS_1 1500
#define PERIOD_WIDTH_MS  20
#define STOP_PULSE_US   2100
#define CW_PULSE_US     2400
#define CCW_PULSE_US    500

#define TRIG_PULSE_US   20
#define TRIG_DELAY_MS   60

PwmOut motor(PA_7);
DigitalOut Trig(PC_1);//5
DigitalIn Echo(PC_2);//6


unsigned char key1, outChar1;
char binfull [] ="Bin Full                    ";
char halffull [] ="Bin is Half full                ";
char binempty [] ="Bin is empty                   ";
char error1 [] ="error                   ";

int detectedtype;
Timer echoTimer;
int trashlevel(){
    int fullness=0;
    int i=0;
    lcd_init();
    detectedtype=trashtype();
    motor.period_ms(PERIOD_WIDTH_MS);
    motor.pulsewidth_us(STOP_PULSE_US);

    if(detectedtype==1){
        motor.pulsewidth_us(CW_PULSE_US);
        

    }else if(detectedtype==3){
        motor.pulsewidth_us(CCW_PULSE_US);
    }
    thread_sleep_for(WAIT_TIME_MS_1);
        Trig = 0;
        wait_us(2);
        Trig = 1;
        wait_us(TRIG_PULSE_US);
        Trig = 0;

        // Wait for echo to go HIGH
        while (Echo == 0);

        // Measure echo HIGH time
        echoTimer.reset();
        echoTimer.start();

        while (Echo == 1);

        echoTimer.stop();

        float echo_us = echoTimer.elapsed_time().count();

        // Distance in cm
        float distance = (echo_us * 0.0343f) / 2.0f;

        printf("Distance: %.2f cm\n", distance);

        float percentage=(distance/30)*100;

        // Control servo
        if (percentage < 10)
        {
            //ledbar code
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = binempty[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }
        }
        
        else if(percentage>=25 && percentage<=75)
        {
            //led bar code
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = halffull[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }
        }
        else if(percentage >75 && percentage <=100){
            //ledbar code
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = binfull[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }

           if(detectedtype==1){
                motor.pulsewidth_us(CCW_PULSE_US);
            }else if(detectedtype==3){
                motor.pulsewidth_us(CW_PULSE_US);
            
            }
            fullness=1;
        }else{
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = error1[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }
        }
        motor.suspend();
        return fullness;
    }









