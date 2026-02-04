#undef __ARM_FP

#include "mbed.h"
#include <cstring>
#include "lcd.h"
#include "trashtype.h"



#define WAIT_TIME_MS_0 500 
#define WAIT_TIME_MS_1 1500
#define PERIOD_WIDTH_MS  20
#define STOP_PULSE_US   1500
#define CW_PULSE_US     2400
#define CCW_PULSE_US    500

#define TRIG_PULSE_US   20
#define TRIG_DELAY_MS   60
PwmOut motor(PA_7);
DigitalOut Trig(PC_6);
DigitalIn Echo(PC_7);
DigitalOut PC0(PC_0);
DigitalOut PC1(PC_1);
DigitalOut PC2(PC_2);
DigitalOut PC3(PC_3);
char binfull [21];
char halffull [21];
char binempty [21];
char error1 [] ="error                   ";

unsigned char key1, outChar1;

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
        strncpy(binfull,"Metal Bin Full         ",21);
        strncpy(halffull,"Metal Bin Half Full         ",21);
        strncpy(binempty,"Metal Bin empty         ,",21);
        
        
    }else if(detectedtype==3){
        motor.pulsewidth_us(CCW_PULSE_US);
        strncpy(binfull,"Plastic Bin Full         ",21);
        strncpy(halffull,"Plastic Bin Half Full         ",21);
        strncpy(binempty,"Plastic Bin empty         ,",21);
        
        
    }else if(detectedtype==2){
        strncpy(binfull,"Paper Bin Full           ",21);
        strncpy(halffull,"Paper Bin Half Full         ",21);
        strncpy(binempty,"Paper Bin empty         ,",21);
        
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

        printf("Distance for bin level: %.2f cm\n", distance);

        float percentage=(distance/30)*100;

        // Control servo
        if (percentage >75 && percentage <=100)
        {
            PC0=1;
            PC1=0;
            PC2=0;
            PC3=0;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = binempty[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }
        }else if(percentage>=25 && percentage<=50)
        {
            //led bar code
            PC0=1;
            PC1=1;
            PC2=0;
            PC3=0;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = halffull[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }
        }
        
        else if(percentage>=50 && percentage<=75)
        {
            //led bar code
            PC0=1;
            PC1=1;
            PC2=1;
            PC3=0;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar1 = halffull[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }
        }
        else if(percentage<25){
            //ledbar code
            PC0=1;
            PC1=1;
            PC2=1;
            PC3=1;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
            
                outChar1 = binfull[i];
                lcd_write_data(outChar1); 	// write character data to LCD
            }

           if(detectedtype==1){
                motor.pulsewidth_us(STOP_PULSE_US);
            }else if(detectedtype==3){
                motor.pulsewidth_us(STOP_PULSE_US);
            
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
        
        return fullness;
    }









