/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 05-03-2021
Modified         : 07-10-2026
Language/ver     : C++ in VS Code

Description      : 7-Segment Counter with Reset/Pause using EXTI and SysTick
/----------------------------------------------------------------*/


// #include "ecRCC2.h"
// #include "ecGPIO2.h"
// #include "ecSysTick2.h"
// #include "ecEXTI2.h"
#include "ecSTM32F4v2.h"

#define SW1_PIN    PD_2         //EVAL board JKIT SW1 : Resets to zero when the push button is pressed
#define SW2_PIN    PA_4			//EVAL board JKIT SW2 : Pause the counting

PinName_t pinFND[8] = { PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
PinName_t selectFND[4] = {PA_10, PA_11, PC_4, PC_3};

volatile uint8_t count = 0;
volatile uint8_t reset_flag = 0;
volatile uint8_t pause = 0;


// Initialiization 
void setup(void)
{
    // Initialize MCU
    MCU_init();
    
    // Button(SW1) PD2
    GPIO_init(SW1_PIN, INPUT);
    GPIO_pupd(SW1_PIN, PU);

    // Button(SW2) PA4
    GPIO_init(SW2_PIN, INPUT);
    GPIO_pupd(SW2_PIN, PU);
    
    // FND
    FND_init(pinFND);
    FND_select_init(selectFND);

    for(int i = 0; i < 8; i++) {
        GPIO_pupd(pinFND[i], NO_PUPD);
        GPIO_ospeed(pinFND[i], MEDIUM_SPEED);
    }

    FND_select(0);

    // Initialize EXTI PD_2 (SW1)
    EXTI_init(SW1_PIN, FALL, 10);

    // Initialize EXTI PA_4 (SW2)
    EXTI_init(SW2_PIN, FALL, 10);

}


// MAIN  ----------------------------------------
int main(void) {

	setup();

    FND_display(count);

	while (1){
        FND_display(count);

        if(pause == 0)
        {
            delay_ms(1000); // 1sec

            if(reset_flag == 1)
            {
                reset_flag = 0;
                continue;
            }

            if(pause == 0)
            {
                count++;
                if(count > 9)
                 count = 0;
            }
        }
        else
        {
            reset_flag = 0;
            delay_ms(10);
        }
    }
}

// SW1 : Reset
void EXTI2_IRQHandler(void) {
    if(is_pending_EXTI(SW1_PIN)) {
        count = 0;
        reset_flag = 1;

        clear_pending_EXTI(SW1_PIN);
    }
}

// SW2 : Pause
void EXTI4_IRQHandler(void) {
	if (is_pending_EXTI(SW2_PIN)) {   
			pause = !pause;
			clear_pending_EXTI(SW2_PIN);
	}
}
