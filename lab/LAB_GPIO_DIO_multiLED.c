/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 05-03-2021
Modified         : 16-09-2026 
Language/ver     : C++ in VS Code

Description      : Controls Four LEDs sequentially using a push-button input on PA_4
/----------------------------------------------------------------*/

#include "ecRCC2.h"
#include "ecGPIO2.h"

#define LED_PIN_0 PB_12
#define LED_PIN_1 PB_13
#define LED_PIN_2 PB_14
#define LED_PIN_3 PB_15

#define BUTTON_PIN PA_4

// Initialiization 
void setup(void) {
	RCC_HSI_init();

    // Push Button
    GPIO_init(BUTTON_PIN, INPUT);
    GPIO_pupd(BUTTON_PIN, PU);

    // LED0
    GPIO_init(LED_PIN_0, OUTPUT);
    GPIO_pupd(LED_PIN_0, PU);
    GPIO_otype(LED_PIN_0, OPEN_DRAIN);
    GPIO_ospeed(LED_PIN_0, MEDIUM_SPEED);
    // LED1
    GPIO_init(LED_PIN_1, OUTPUT);
    GPIO_pupd(LED_PIN_1, PU);
    GPIO_otype(LED_PIN_1, OPEN_DRAIN);
    GPIO_ospeed(LED_PIN_1, MEDIUM_SPEED);
    // LED2
    GPIO_init(LED_PIN_2, OUTPUT);
    GPIO_pupd(LED_PIN_2, PU);
    GPIO_otype(LED_PIN_2, OPEN_DRAIN);
    GPIO_ospeed(LED_PIN_2, MEDIUM_SPEED);
    // LED3
    GPIO_init(LED_PIN_3, OUTPUT);
    GPIO_pupd(LED_PIN_3, PU);
    GPIO_otype(LED_PIN_3, OPEN_DRAIN);
    GPIO_ospeed(LED_PIN_3, MEDIUM_SPEED);
}
	
int main(void) { 

 	setup();
	
    int ledState = 0;
    uint32_t buttonState = HIGH;
	uint32_t prevButtonState = HIGH;

    GPIO_write(LED_PIN_0, HIGH);
    GPIO_write(LED_PIN_1, LOW);
    GPIO_write(LED_PIN_2, LOW);
    GPIO_write(LED_PIN_3, LOW);


	while(1){
		
        buttonState = GPIO_read(BUTTON_PIN);

        if(buttonState == LOW && prevButtonState == HIGH){

            // Software debouncing
            delay_ms_HSI(50);

            if(GPIO_read(BUTTON_PIN) == LOW){
                
                // Turn OFF current LED
                if(ledState == 0)
                    GPIO_write(LED_PIN_0, LOW);
                else if(ledState == 1)
                    GPIO_write(LED_PIN_1, LOW);
                else if(ledState == 2)
                    GPIO_write(LED_PIN_2, LOW);
                else if(ledState == 3)
                    GPIO_write(LED_PIN_3, LOW);

                // Move to next LED
                ledState++;

                if(ledState > 3)
                    ledState = 0;

                // Turn ON next LED
                if(ledState == 0)
                    GPIO_write(LED_PIN_0, HIGH);
                if(ledState == 1)
                    GPIO_write(LED_PIN_1, HIGH);
                if(ledState == 2)
                    GPIO_write(LED_PIN_2, HIGH);
                if(ledState == 3)
                    GPIO_write(LED_PIN_3, HIGH);
            }
        }

        prevButtonState = buttonState;
    }
}