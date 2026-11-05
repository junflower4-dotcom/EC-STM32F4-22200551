
/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 02-11-2026
Modified         : 02-11-2026
Language/ver     : C in VS Code

Description      : LAB_PWM_RC - RC Servo Motor Control
                   Control SG90 servo motor from 0 to 180 degrees
                   and back to 0 degrees using PWM and TIM3 IRQ.
                   Reset servo position with USER button (EXTI).
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

// Pin Configuration
#define PWM_PIN       PA_1
#define BUTTON_PIN    PC_13

// Servo Configuration
#define SERVO_MIN     0
#define SERVO_MAX     180
#define SERVO_STEP    10
#define PULSE_MIN_US  500
#define PULSE_MAX_US  2500

// Global Variables
volatile int servo_angle = 0;
volatile int direction = 1;

// Function Prototypes
void setup(void);
void Servo_setAngle(int angle);


// Main Function
int main(void){

    setup();

    while(1){
        // Servo control is handled by interrupts
    }
}


// Initialization
void setup(void){

    // 1. System Clock = 84 MHz
    MCU_init();

    // 2. PWM Initialization: TIM2_CH2 (PA1)
    // Period = 20 ms (50 Hz)
    PWM_init(PWM_PIN, 20);

    // Initial Servo Angle = 0 deg
    Servo_setAngle(0);

    // 3. USER Button: PC13 Input, Pull-Up
    GPIO_init(BUTTON_PIN, PU);

    GPIOC->PUPDR &= ~(3UL << (2 * 13));
    GPIOC->PUPDR |=  (1UL << (2 * 13));

    // 4. External Interrupt: Falling Edge
    EXTI_init(BUTTON_PIN, FALL, 0);

    // 5. Timer Update Interrupt: TIM3, 500 ms
    TIM_UI_init(TIM3, 500);
}


// Set Servo Angle (0 ~ 180 deg)
void Servo_setAngle(int angle){

    // Limit Angle Range
    if(angle < SERVO_MIN) angle = SERVO_MIN;
    if(angle > SERVO_MAX) angle = SERVO_MAX;

    // Convert Angle to Pulse Width (us)
    uint32_t pulse_width =
        PULSE_MIN_US +
        (uint32_t)(angle * (PULSE_MAX_US - PULSE_MIN_US)
        / SERVO_MAX);

    // Update PWM Pulse Width
    PWM_pulsewidth_us(PWM_PIN, pulse_width);
}


// TIM3 Interrupt Handler: Every 500 ms
void TIM3_IRQHandler(void){

    if(is_UIF(TIM3)){

        clear_UIF(TIM3);

        // Update Servo Angle
        servo_angle += SERVO_STEP * direction;

        // Change Direction at Limits
        if(servo_angle >= SERVO_MAX){
            servo_angle = SERVO_MAX;
            direction = -1;
        }
        else if(servo_angle <= SERVO_MIN){
            servo_angle = SERVO_MIN;
            direction = 1;
        }

        // Update PWM
        Servo_setAngle(servo_angle);
    }
}


// External Interrupt Handler: USER Button
void EXTI15_10_IRQHandler(void){

    if(is_pending_EXTI(BUTTON_PIN)){

        clear_pending_EXTI(BUTTON_PIN);

        // Reset Servo State
        servo_angle = 0;
        direction = 1;

        // Reset Servo Position
        Servo_setAngle(0);

        // Restart 500 ms Timer Interval
        TIM3->CNT = 0;
        clear_UIF(TIM3);
        NVIC_ClearPendingIRQ(TIM3_IRQn);
    }
}
