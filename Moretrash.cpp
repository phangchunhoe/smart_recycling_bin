#undef __ARM_FP

#include "mbed.h"
#include <cstring>
#include "doorop.h"
#include "lcd.h"
unsigned char key2, outChar2;
char question [] ="More trash to put in?            ";
char addmore [] ="Detecting trash...            ";

DigitalIn pushbutton1(PB_13);
DigitalIn pushbutton2(PB_10);

int Moretrash(){
    int moretrash=0;
    lcd_init();
    lcd_write_cmd(0x80);
    for (int i = 0; i < 20; i++)		//for 20 char LCD module
    {
        outChar2 = question[i];
        lcd_write_data(outChar2); 
    }	// write character data to LCD
   int value=pushbutton1.read();
   printf("Button1: %.2i \n", value); 
   thread_sleep_for(500);
    while(pushbutton1.read()==0 && pushbutton2.read()==0);

    if(pushbutton1.read()==1){//More trash to be added
        lcd_write_cmd(0x80);
        for (int i = 0; i < 20; i++)		//for 20 char LCD module
        {
        outChar2 = addmore[i];
        lcd_write_data(outChar2); 
        }
        moretrash=1;
        
    }
    else if(pushbutton2.read()==1){//Go to rfid to claim points
        

    }
    return moretrash;
    
    

}

