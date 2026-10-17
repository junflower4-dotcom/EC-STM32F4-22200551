/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 17-09-2026
Modified         : 17-09-2026 
Language/ver     : C++ in VS Code

Description      : 
/----------------------------------------------------------------*/

#include "stm32f4xx.h"
#include "ecGPIO2.h"
#include "ecRCC2.h"

#define BUTTON_PIN PA_4

PinName_t pinFND[8]    = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
PinName_t selectFND[4] = {PA_10, PA_11, PC_4, PC_3};

void setup(void){
    
    // Intialize System Clock
    RCC_HSI_init();    

    // Button Intialize
    GPIO_init(BUTTON_PIN, INPUT);
    GPIO_pupd(BUTTON_PIN, PU);

    // Intialize segment and display selection pins
    FND_init(pinFND);
    FND_select_init(selectFND);    
    
    // Select FND0 and display the initial number 0
    FND_select(0);
    FND_display(0);

};

int main(void) {

    setup();

    uint8_t num = 0;

    while (1) {

        if(GPIO_read(BUTTON_PIN) == LOW){
            
            delay_ms_HSI(20);

            if(GPIO_read(BUTTON_PIN) == LOW){
                
                num++;
                
                if(num > 9)
                    num = 0;

                FND_display(num);

                
                while (GPIO_read(BUTTON_PIN) == LOW){}

                delay_ms_HSI(20);
                
            }
        }

    }

}

