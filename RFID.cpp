
/*#undef __ARM_FP
#include <cstdint>
#include <string.h> 
#include "mbed.h"
#include "MFRC522.h"
#define RST_PIN         PB_1          // Configurable, see typical pin layout above
#define SS_PIN          PB_2 
#define PERIOD_WIDTH_MS     20   
#define STOP_PULSE_US       500    // Stop
#define CW_PULSE_US         2400  


        // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN); 
PwmOut Servomotor1(PA_1);
DigitalIn  button_PB11 (PB_11);
//DigitalIn  button_PB11 (PC_12);
DigitalOut LED(PB_15);


void EXTI15_10_IRQHandler(void);


void RFID(){
    mfrc522.PCD_Init();  
    Servomotor1.period_ms(PERIOD_WIDTH_MS);
    Servomotor1.pulsewidth_us(STOP_PULSE_US);
    
    __disable_irq();                // //disable IRQs globally

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //enable AFIO clock

    AFIO->EXTICR[2] &= ~0xF000;     //clear port selection for EXTI11
    AFIO->EXTICR[2] |= 0x1000;      //select port B

    EXTI->IMR |= 0x0800;            //unmask EXIT12
    EXTI->RTSR |= 0x0800;           //set rising edge trigger. 

    NVIC_EnableIRQ(EXTI15_10_IRQn); //enable IRQ14, bit 8 of ISER[1]

    __enable_irq();                 //enable IRQs globally

    NVIC_SetVector(EXTI15_10_IRQn, (uint32_t)&EXTI15_10_IRQHandler); //load the ISR
    
    //while
}


void EXTI15_10_IRQHandler()
{
    Servomotor1.pulsewidth_us(2400);
    int count=0;
    int cardread=0;
    while(count<400 && cardread==0){
        LED=1;
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
            count+=1;
                continue;
            }
            // Select one of the cards
        cardread=1;
        Servomotor1.pulsewidth_us(1500); 
    }
    LED=0;
    EXTI->PR = 0x0800; //clear interrupt pending flag
}

*/