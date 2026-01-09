
#undef __ARM_FP

#include "mbed.h"
#include <cstring>
#include "lcd.h"

#define WAIT_TIME_MS_0 500 
#define WAIT_TIME_MS_1 1500

DigitalOut led1(LED1);
DigitalIn button(PC_12);
DigitalIn IRSensor(PD_2);
DigitalIn button2(PA_15);

unsigned char key, outChar;
char metal [ ] = "Metal               ";
char paper [ ] = "Paper               ";
char plastic [ ] = "Plastic             ";
//METAL DETECTOR INCOMPLETE
int trashtype()
{
    int i=0;
    lcd_init();	
    int type = 0;
   //1:Metal 2:Paper 3:Plastic
    while(button2.read()==1);
    printf("nigger");
    while (type==0)
    {
        if(button.read()==0){
            type=1;
            lcd_write_cmd(0x80);			// Move cursor to line 1 position 1
            for (i = 0; i < 20; i++)		//for 20 char LCD module
            {
                outChar = metal[i];
                lcd_write_data(outChar); 	// write character data to LCD
            }
            
        }else if(IRSensor.read()==0){
            type=2;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
		{
			outChar = paper[i];
			lcd_write_data(outChar); 	// write character data to LCD
		}
        }else{
            type=3;
            lcd_write_cmd(0x80);
            for (i = 0; i < 20; i++)		//for 20 char LCD module
		{
			outChar = plastic[i];
			lcd_write_data(outChar); 	// write character data to LCD
		}
        }
    }
    return type;
    
}
