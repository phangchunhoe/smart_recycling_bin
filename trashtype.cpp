
#undef __ARM_FP

#include "mbed.h"
#include <cstring>
#include "lcd.h"
#include "UltrasoundDetect.h"

#define WAIT_TIME_MS_0 500 
#define WAIT_TIME_MS_1 1500

DigitalOut led1(LED1);
DigitalIn IRSensor(PD_2);
DigitalIn pinB9(PB_9);

unsigned char key4, outChar4;
char metalprint [ ] = "Metal               ";
char paperprint [ ] = "Paper               ";
char plasticprint [ ] = "Plastic             ";
//METAL DETECTOR INCOMPLETE
int trashtype()
{
    int i=0;
    lcd_init();	
    int type = 0;
    int detect=0;
    int count=0;
   //1:Metal 2:Paper 3:Plastic
    while(detect==0){
        detect=ultrasound1();
        printf("Detect %i:",detect);
        thread_sleep_for(1000);
    }

    while (type==0)
    {
        count=0;
        while(count<5){
            if(pinB9.read()==1){
                type=1;
                lcd_write_cmd(0x80);			// Move cursor to line 1 position 1
                for (i = 0; i < 20; i++)		//for 20 char LCD module
                {
                    outChar4 = metalprint[i];
                    lcd_write_data(outChar4); 	// write character data to LCD
                }     
                break;
            } 
            thread_sleep_for(200);
            count+=1;
        }
        if(IRSensor.read()==0 && type==0){
            type=2;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
		{
			outChar4 = paperprint[i];
			lcd_write_data(outChar4); 	// write character data to LCD
		}
        }else if(IRSensor.read()==1 && type==0){
            type=3;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
		{
			outChar4 = plasticprint[i];
			lcd_write_data(outChar4); 	// write character data to LCD
		}
        }
    }
    return type;
    
}
