#undef __ARM_FP
#include "MFRC522.h"
#include "Moretrash.h"
#include "RFID.h"
#include "lcd.h"
#include "mbed.h"
#include "trashlevel.h"
#include "trashlevelAll.h"
#include "wash.h"
#include <cstdint>
#include <string.h>
#include "web.h"
#define RST_PIN PB_1 // Configurable, see typical pin layout above
#define SS_PIN PB_2
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

#define PERIOD_WIDTH_MS 20 // 20 ms period
#define STOP_PULSE_US 1500 // Stop
#define CW_PULSE_US 2400   // Clockwise
#define CCW_PULSE_US 500
// input of IR sensor data
// DigitalOut Trig(PC_1);
// DigitalIn Echo(PC_2);
// turn on/off the on-board LED: PB14
DigitalOut LED_PB14(PB_14);
unsigned char reggin = 0;
float objDistance = 0;
unsigned char key3, outChar3;
// ===== Hardware =====
PwmOut Servomotor(PA_6);
PwmOut motorbin(PA_7);
MFRC522 mfrc522(SS_PIN, RST_PIN);
PwmOut Servomotor1(PA_1);
DigitalIn button_PB11(PB_11);
char pingen[] = "Pin: 1234                   ";
void EXTI15_10_IRQHandler(void);
void nigger() {
  int binfull = 0;
  int level = 0;
  lcd_init();
  // rfid(test if need WFI)

  while (binfull < 3) {

    // wash();
    mfrc522.PCD_Init();
    Servomotor1.period_ms(PERIOD_WIDTH_MS);
    Servomotor1.pulsewidth_us(STOP_PULSE_US);

    __disable_irq(); // //disable IRQs globally

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // enable AFIO clock

    AFIO->EXTICR[2] &= ~0xF000; // clear port selection for EXTI11
    AFIO->EXTICR[2] |= 0x1000;  // select port B

    EXTI->IMR |= 0x0800;  // unmask EXIT12
    EXTI->RTSR |= 0x0800; // set rising edge trigger.

    NVIC_EnableIRQ(EXTI15_10_IRQn); // enable IRQ14, bit 8 of ISER[1]

    __enable_irq(); // enable IRQs globally

    NVIC_SetVector(EXTI15_10_IRQn, (uint32_t)&EXTI15_10_IRQHandler);
    LED_PB14 = 1;
    Servomotor.period_ms(PERIOD_WIDTH_MS);
    // Start stopped
    Servomotor.pulsewidth_us(STOP_PULSE_US);
    level = trashlevel();

    if (!level) {

      Servomotor.resume();
      printf("Throw your trash");
      Servomotor.pulsewidth_us(CW_PULSE_US);
      thread_sleep_for(1500);
      Servomotor.suspend();
      thread_sleep_for(3000);
      Servomotor.resume();
      Servomotor.pulsewidth_us(CCW_PULSE_US);
      printf("Door closing");
      thread_sleep_for(1500);
      Servomotor.suspend();
    }

    if (detectedtype == 1) {
      motorbin.pulsewidth_us(STOP_PULSE_US);
    } else if (detectedtype == 3) {
      motorbin.pulsewidth_us(STOP_PULSE_US);
    }
    binfull = trashlevelAll();
    printf("TrashlevelAll: %i",binfull);
    if (!(binfull == 3)) {
      if (Moretrash() == 1) {
        continue;
      };
    }

    lcd_write_cmd(0x80);
    for (int i = 0; i < 20; i++) // for 20 char LCD module
    {
      outChar3 = pingen[i];
      lcd_write_data(outChar3);
    }
    thread_sleep_for(1000);
    
    web(paper,plastic,metal);
  }
}

void EXTI15_10_IRQHandler() {
  Servomotor1.pulsewidth_us(2400);
  int count = 0;
  int cardread = 0;
  while (count < 400 && cardread == 0) {

    if (!mfrc522.PICC_IsNewCardPresent()) {
      count += 1;
      continue;
    }
    // Select one of the cards
    cardread = 1;
    Servomotor1.pulsewidth_us(1500);
  }
  EXTI->PR = 0x0800; // clear interrupt pending flag
}