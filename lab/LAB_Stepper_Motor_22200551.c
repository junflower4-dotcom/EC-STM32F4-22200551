/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 14-10-2026
Modified         : 14-10-2026
Language/ver     : C++ in VS Code

Description      : Stepper motor control using Moore FSM.
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

// STEPPER_MODE select
#define STEPPER_MODE 	HALF
// #define STEPPER_MODE	 HALF 
#define STEP_PER_REV \
		 ((uint32_t)((STEPPER_MODE == FULL) ? FULL_STEP_PER_REV : \
					 (STEPPER_MODE == HALF) ? HALF_STEP_PER_REV : \
					 0U))

// STEPPER_direction DIR_CCW / DIR CW
#define STEPPER_DIR		DIR_CCW

#define BUTTON_PIN PC_13 
#define RPM	(int)(5)

// ( A, B,  AN,  BN)
PinName_t pinStepper[4]    = {PB_10,PB_4,PB_5,PB_3};


// Initialiization 
void setup(void){	

    MCU_init();
	
    GPIO_init(BUTTON_PIN, INPUT);           		// GPIOC pin13 initialization
    EXTI_init(BUTTON_PIN, FALL,0);           		// External Interrupt Setting

	stepper_init(STEPPER_MODE, pinStepper); 				// Stepper GPIO pin initialization
	stepper_speed(RPM);                          		//  set stepper motor speed [RPM]
}


int main(void) { 
	// Initialiization 
	setup();
	uint32_t rev=1;     // Revoultion
	uint32_t steps=STEP_PER_REV*rev;
	

	// Inifinite Loop 
	while(1){
		stepper_step(steps, STEPPER_DIR); 
		delay_ms(2000);
	}
}


void EXTI15_10_IRQHandler(void) {  
	if (is_pending_EXTI(BUTTON_PIN)) {
		stepper_stop();
		clear_pending_EXTI(BUTTON_PIN); // cleared by writing '1'
	}
}