/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Student ID       : [22200551]
Created          : 22-10-2026 
Language/ver     : C in VS Code

Description      : EC Test 1 (2026) Programming Test
                   
                   Rename this file to  EC_Test1_yourID.c
/----------------------------------------------------------------*/


#include "stm32f411xe.h"
#include "ecSTM32F4v2.h"

//==================================================================
// User GPIO Pin definitions
//==================================================================

// JKIT button
#define SW1_PIN    PD_2         // 누르면 Motor 속도 빨라짐
#define SW2_PIN    PA_4			// 누르면 Motor 속도 느려짐

// Photo sensor
#define PHOTO_PIN PA_0

// MODE0 LED_PIN
#define LED_0 PB_12

// Stepper ( A, B, AN, BN )
PinName_t pinStepper[4] = {PC_5, PC_6,PC_7,PC_8};

// FND segments
PinName_t pinFND[8] = { PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

// FND digit select : FND_0 ~ FND_3
PinName_t selectFND[4] = {PA_10, PA_11, PC_4, PC_3}; // FND0, FND1



// Global Variables
// use   volatile uint32_t   instead of uint32_t if used in Interrupt ISR
volatile uint32_t motor_mode   = 0;	
volatile uint32_t motor_dir    = 0;	
volatile uint32_t motor_speed  = 0;	
// YOUR CODE GOES HERE
// YOUR CODE GOES HERE
// YOUR CODE GOES HERE



//==================================================================
//  Setup 
//==================================================================
void setup(void){
	RCC_PLL_init();					// System Clock = 84MHz
	SysTick_init();					// SysTick, 1 msec
	
	// USART2, 9600 baud
    USART2_init();
    USART2_baud(BAUD_9600);	

	// GPIO Inputs
    // SW1, SW2
	GPIO_init(SW1_PIN, INPUT);
    GPIO_pupd(SW1_PIN, PU);
    GPIO_init(SW2_PIN, INPUT);
    GPIO_pupd(SW2_PIN, PU);

	// PHOTO
	GPIO_init(PHOTO_PIN, INPUT);
    GPIO_pupd(PHOTO_PIN, PU);

	// LED_0
	GPIO_init(LED_0, OUTPUT);

	// stepper motor
	stepper_init(FULL, pinStepper);

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


	stepper_step(FULL_STEP_PER_REV, DIR_CW);
	stepper_speed(motor_speed);

	FND_display(0);
	GPIO_wirte(LED_0, LOW);

	
}


//==================================================================
//  Main
//==================================================================
int main(void){
	
	setup();
	int buttonState = 0;

	while(1){

		// Check PAUSE/RESUME
		buttonState = GPIO_read(PHOTO_PIN);

		if(buttonState = 1) {
			while(1) {
				stepper_speed(0);
				GPIO_write(PHOTO_PIN, HIGH);
				delay_ms_HSI(2000);
				GPIO_write(PHOTO_PIN, LOW);
				delay_ms_HSI(2000);
			}
		}
		
		stepper_speed(motor_speed);
		FND_display(motor_mode);
		GPIO_write(LED_0, LOW);
		 
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE

		// Related to controlling motor speed/dir etc
		// HINT:  use  stepper_speed(vel);	only when vel>0
		// stepper_step(1, motor_dir);	 // Keep step=1 or a low value to stop blocking other code
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE


		// Print to the serial monitor every 2 sec
		// PAUSE, blink at 2 sec
		//     e.g.  printf("Mode=%lu Speed=%lu DIR=%lu\r\n", ... );
		// HINT:  msTicks in ecSysTick2.h  is increasing at a 1 msec rate accurately.
		// You can use msTicks  to count every 2000 msec 
		// YOUR CODE GOES HERE
		// YOUR CODE GOES HERE

			
	delay_ms(1);  // use this delay
	}
}


//==================================================================
//  EXTI Interrupt Service Routines
//==================================================================
// SW1 : Reset
void EXTI2_IRQHandler(void) {
    if(is_pending_EXTI(SW1_PIN)) {
		motor_mode ++;
		motor_speed += 4;
        clear_pending_EXTI(SW1_PIN);
    }
}

// SW2 : Pause
void EXTI4_IRQHandler(void) {
	if (is_pending_EXTI(SW2_PIN)) {   
		motor_mode --;
		motor_speed -= 4;

		clear_pending_EXTI(SW2_PIN);
	}
}
//==================================================================
//  UART2 Interrupt Service Routines
//==================================================================
void USART2_IRQHandler(void){
	if(is_USART2_RXNE()){
	
	// HINT: 	comparing a character:  
	//  		uint8_t ch = USART2_read();		
	//			if(ch == 'C' || ch == 'c')	 etc..
	//
	// YOUR CODE GOES HERE
	// YOUR CODE GOES HERE
	}
}
