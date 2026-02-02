#undef __ARM_FP

#include "mbed.h"
#include <cstring>
#include "doorop.h"
#include "lcd.h"
unsigned char key2, outChar2;
char question [] ="More trash to put in?            ";
char addmore [] ="Detecting trash...            ";
char getprize []= "RFID...                     ";
DigitalIn pushbutton1(PC_12);
DigitalIn pushbutton2(PA_15);

int Moretrash(){
    int moretrash=0;
    lcd_init();
    lcd_write_cmd(0x80);
    for (int i = 0; i < 20; i++)		//for 20 char LCD module
    {
        outChar2 = question[i];
        lcd_write_data(outChar2); 
    }	// write character data to LCD
    while(pushbutton1.read()==1 && pushbutton2.read()==1);

    if(pushbutton1.read()==0){//More trash to be added
        lcd_write_cmd(0x80);
        for (int i = 0; i < 20; i++)		//for 20 char LCD module
        {
        outChar2 = addmore[i];
        lcd_write_data(outChar2); 
        }
        moretrash=1;
        
    }
    else if(pushbutton2.read()==0){//Go to rfid to claim points
        lcd_write_cmd(0x80);
        for (int i = 0; i < 20; i++)		//for 20 char LCD module
        {
        outChar2 = getprize[i];
        lcd_write_data(outChar2); 
        }
        //RFID()
    }
    return moretrash;
    
    

}

