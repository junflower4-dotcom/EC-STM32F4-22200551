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

#define MOTOR_REV       3
#define TOTAL_STEPS      (FULL_STEP_PER_REV * MOTOR_REV)

// Global Variables
// use   volatile uint32_t   instead of uint32_t if used in Interrupt ISR
volatile uint32_t motor_mode   = 0;	
volatile uint32_t motor_dir	   = DIR_CW;
volatile uint32_t motor_speed  = 2;	

uint8_t  CDS_condition  = 0;
// YOUR CODE GOES HERE
// YOUR CODE GOES HERE
// YOUR CODE GOES HERE

// Send a string to Tera Term
void key_print(const char *text) {
    uint32_t len = 0;

    while(text[len] != '\0'){
        len++;
    }

    if(len > 0){
        USART2_write((uint8_t *)text, len);
    }
}

void key_echo(uint8_t command) {
    key_print("RX: ");
    USART2_write(&command, 1);
    key_print("\r\n");
}


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
	GPIO_write(LED_0, HIGH);

	// stepper motor
	stepper_init(FULL, pinStepper);

	// FND
	FND_init(pinFND);
	FND_select_init(selectFND);

	for(int i = 0; i < 8; i++) {
        GPIO_pupd(pinFND[i], NO_PUPD);
        GPIO_ospeed(pinFND[i], MEDIUM_SPEED);
    }

	// Initialize EXTI PD_2 (SW1)
    EXTI_init(SW1_PIN, FALL, 10);

    // Initialize EXTI PA_4 (SW2)
    EXTI_init(SW2_PIN, FALL, 10);

	// stepper motor
	stepper_speed(motor_speed);
	stepper_stop();

	FND_select(0);
	FND_display(0);


}


//==================================================================
//  Main
//==================================================================
int main(void){

	CDS_condition = GPIO_read(PHOTO_PIN);
	uint8_t command	= 0;
	uint32_t remaining_steps = 0;

	setup();

	key_print("\r\nInitially, Mode 0\r\n");
	key_print("CW, STOP, FND=0, LED ON\r\n");

	while(1){

		if(CDS_condition) {
			while(1) {
				stepper_speed(0);
				GPIO_write(PHOTO_PIN, HIGH);
				delay_ms_HSI(2000);
				GPIO_write(PHOTO_PIN, LOW);
				delay_ms_HSI(2000);

				remaining_steps = 0;
				stepper_stop();

				__disable_irq();
           	 	CDS_condition = 0;
            	__enable_irq();

				key_print("Energency STOP\r\n");
			}
		}

		if(!CDS_condition){

			if(motor_mode == 0){
				FND_select(0);
				FND_display(0);
				motor_speed = 0;
				stepper_stop();

			}

			if(motor_mode == 1){
				FND_select(0);
				FND_display(1);
				motor_speed = 2;
				remaining_steps = TOTAL_STEPS;
				switch(command){
            	case 'C':
               		key_echo('C');
               		motor_dir = DIR_CW;
					FND_select(0);
					
                	key_print("Motor CW\r\n");
                	break;

           		case 'B':
                	key_echo('B');
                	motor_dir = DIR_CCW;
					FND_select(1);
					
                	key_print("Motor CCW\r\n");
                	break;

            	default:
                break;
			}

			if(remaining_steps > 0){
            	stepper_step(1, motor_dir);
            	remaining_steps--;

				stepper_speed(motor_speed);

            	if(remaining_steps == 0){
                stepper_stop();
                key_print("Motor Completed\r\n");
           	 	}
        	}
			}
			if(motor_mode == 2){
				FND_select(0);
				FND_display(2);
				motor_speed = 6;
				
				remaining_steps = TOTAL_STEPS;
				switch(command){
            	case 'C':
               		key_echo('C');
               		motor_dir = DIR_CW;
					
					
                	key_print("Motor CW\r\n");
                	break;

           		case 'B':
                	key_echo('B');
                	motor_dir = DIR_CCW;
					
                	key_print("Motor CCW\r\n");
                	break;

            	default:
                break;
			}

			if(remaining_steps > 0){
            	stepper_step(1, motor_dir);
            	remaining_steps--;

				stepper_speed(motor_speed);

            	if(remaining_steps == 0){
                stepper_stop();
                key_print("Motor Completed\r\n");
           	 	}
        	}
			}
			if(motor_mode == 3){
				FND_select(0);
				FND_display(3);
				motor_speed = 10;
				remaining_steps = TOTAL_STEPS;
				switch(command){
            	case 'C':
               		key_echo('C');
               		motor_dir = DIR_CW;

					
                	key_print("Motor CW\r\n");
                	break;

           		case 'B':
                	key_echo('B');

					
                	motor_dir = DIR_CCW;

                	key_print("Motor CCW\r\n");
                	break;

            	default:
                break;
			}

			if(remaining_steps > 0){
            	stepper_step(1, motor_dir);
            	remaining_steps--;

				stepper_speed(motor_speed);

            	if(remaining_steps == 0){
                stepper_stop();
                key_print("Motor Completed\r\n");
           	 	}
        	}
			}

			__disable_irq();
        	command = CDS_condition;
        	CDS_condition = 0;
        	__enable_irq();

			
        }
		

	}
		
		// Read the latest Bluetooth command atomically.



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


//==================================================================
//  EXTI Interrupt Service Routines
//==================================================================
// SW1
void EXTI2_IRQHandler(void) {
    if(is_pending_EXTI(SW1_PIN)) {
		motor_mode ++;
		if(motor_mode > 3){
			motor_mode = 3;
		}
		delay_ms_HSI(20);
        clear_pending_EXTI(SW1_PIN);
    }
}

// SW2 
void EXTI4_IRQHandler(void) {
	if (is_pending_EXTI(SW2_PIN)) {   
		motor_mode --;
		delay_ms_HSI(50);
		clear_pending_EXTI(SW2_PIN);
	}
}
//==================================================================
//  UART2 Interrupt Service Routines
//==================================================================
void USART2_IRQHandler(void){
	if(is_USART2_RXNE()){

		uint8_t key_data = USART2_read();

       
            switch(key_data){
                case 'C': case 'c': CDS_condition = 'C'; break;
                case 'B': case 'b': CDS_condition = 'B'; break;
                default: break; // Ignore CR, LF and other characters.
            }
        
	}
}
