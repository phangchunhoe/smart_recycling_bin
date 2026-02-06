#undef __ARM_FP

#include "mbed.h"

DigitalOut pump(PB_0,1);
DigitalOut led(PB_15);



void wash(){
     pump=1;
    
    while(true){
    pump=0;
    thread_sleep_for(1000);
    pump=1;
    thread_sleep_for(1000);
    }
    
}
