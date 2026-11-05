
/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 03-11-2026
Modified         : 03-11-2026
Language/ver     : C in VS Code

Description      : LAB_PWM_DC - Smart Mini-Fan
                   PWM speed control with FSM, EXTI,
                   proximity sensor, LED, and UART.
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"
#include "ecTIM2.h"
#include "ecPWM2.h"
#include "ecEXTI2.h"
#include <stdio.h>

// Pin Configuration
#define PWM_PIN         PA_0
#define BUTTON_PIN      PC_13

// Other pins
#define DIR_BIT         2       // PC2
#define LED_BIT         5       // PA5 (LD2)
#define IR_BIT          0       // PB0

// IR Sensor: LOW when object is near
// Change to 1 if the sensor is active HIGH.
#define IR_NEAR_LEVEL   0U

// Direction Control: PC2 = LOW
#define MOTOR_DIR       0U

// Fan States
typedef enum {
    FAN_OFF = 0,
    FAN_MID,
    FAN_HIGH,
    FAN_PAUSE
} FanState;

// Global Variables
volatile uint8_t button_event = 0;
volatile uint8_t button_locked = 0;
volatile uint8_t led_blink = 1;
volatile uint8_t print_event = 0;
volatile uint32_t timer_count = 0;

// Selected mode is retained while PAUSE
FanState selected_mode = FAN_OFF;
FanState current_state = FAN_OFF;

uint8_t paused = 0;

// Function Prototypes
void setup(void);
void GPIO_setup(void);
void UART2_TX_setup(void);
void UART2_send(const char *str);
void Fan_update(void);
void LED_update(void);
void Print_status(void);
uint8_t IR_isNear(void);
const char* Mode_name(FanState state);


// Main Function
int main(void){

    setup();

    UART2_send("Smart Mini-Fan Initialized\r\n");
    Print_status();

    while(1){

        // 1. Detect proximity sensor
        uint8_t near = IR_isNear();

        if(near && !paused){
            paused = 1;
            led_blink = 1;
        }
        else if(!near && paused){
            paused = 0;
        }

        // 2. Read button event safely
        uint8_t pressed;

        __disable_irq();
        pressed = button_event;
        button_event = 0;
        __enable_irq();

        // 3. Change mode only when not paused
        if(pressed && !paused){

            if(selected_mode == FAN_OFF)
                selected_mode = FAN_MID;

            else if(selected_mode == FAN_MID)
                selected_mode = FAN_HIGH;

            else
                selected_mode = FAN_OFF;
        }

        // 4. FSM output state
        if(paused)
            current_state = FAN_PAUSE;
        else
            current_state = selected_mode;

        // 5. Update PWM and LED
        Fan_update();
        LED_update();

        // 6. Print status every 2 seconds
        if(print_event){

            print_event = 0;
            Print_status();
        }
    }
}


// Initialization
void setup(void){

    // 1. MCU Initialization (PLL included)
    MCU_init();

    // 2. GPIO Initialization
    GPIO_setup();

    // 3. PWM: TIM2_CH1, Period = 1 ms
    PWM_init_us(PWM_PIN, 1000);
    PWM_duty(PWM_PIN, 0.0f);

    // 4. USART2: 9600 baud (TX only)
    UART2_TX_setup();

    // 5. B1 External Interrupt
    EXTI_init(BUTTON_PIN, FALL, 1);
    clear_pending_EXTI(BUTTON_PIN);

    // 6. TIM3 Update Interrupt: 500 ms
    TIM_UI_init(TIM3, 500);
}


// GPIO Initialization
void GPIO_setup(void){

    // Enable GPIOA, GPIOB, GPIOC clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN;

    // PC2: Direction Output (01)
    GPIOC->MODER &= ~(3UL << (2 * DIR_BIT));
    GPIOC->MODER |=  (1UL << (2 * DIR_BIT));
    GPIOC->OTYPER &= ~(1UL << DIR_BIT);

    // Direction = LOW
    GPIOC->BSRR = (1UL << (DIR_BIT + 16));

    // PA5: Onboard LED Output (01)
    GPIOA->MODER &= ~(3UL << (2 * LED_BIT));
    GPIOA->MODER |=  (1UL << (2 * LED_BIT));
    GPIOA->OTYPER &= ~(1UL << LED_BIT);

    // LED OFF
    GPIOA->BSRR = (1UL << (LED_BIT + 16));

    // PB0: IR Sensor Input (00)
    GPIOB->MODER &= ~(3UL << (2 * IR_BIT));

    // PB0: Pull-Up (01)
    GPIOB->PUPDR &= ~(3UL << (2 * IR_BIT));
    GPIOB->PUPDR |=  (1UL << (2 * IR_BIT));

    // PC13: B1 Button Input (00)
    GPIOC->MODER &= ~(3UL << 26);

    // PC13: Pull-Up (01)
    GPIOC->PUPDR &= ~(3UL << 26);
    GPIOC->PUPDR |=  (1UL << 26);
}


// Check IR Sensor
uint8_t IR_isNear(void){

    uint32_t value =
        (GPIOB->IDR >> IR_BIT) & 1UL;

    return (value == IR_NEAR_LEVEL);
}


// Set Motor Duty according to FSM
void Fan_update(void){

    static int last_duty = -1;

    int target_duty = 0;

    switch(current_state){

        case FAN_OFF:
            target_duty = 0;
            break;

        case FAN_MID:
            target_duty = 50;
            break;

        case FAN_HIGH:
            target_duty = 100;
            break;

        case FAN_PAUSE:
        default:
            target_duty = 0;
            break;
    }

    // Only update PWM when duty changes
    if(target_duty != last_duty){

        float targetPWM = target_duty / 100.0f;

        // Compensation for one-PWM / one-DIR wiring
        float outputPWM = (MOTOR_DIR == 0U)
                        ? targetPWM
                        : (1.0f - targetPWM);

        PWM_duty(PWM_PIN, outputPWM);

        last_duty = target_duty;
    }
}


// LED Control
void LED_update(void){

    uint8_t led_on = 0;

    if(current_state == FAN_PAUSE){

        // Toggle every 500ms: 1-second blink cycle
        led_on = led_blink;
    }
    else if(current_state == FAN_MID ||
            current_state == FAN_HIGH){

        led_on = 1;
    }
    else{
        led_on = 0;
    }

    if(led_on)
        GPIOA->BSRR = (1UL << LED_BIT);
    else
        GPIOA->BSRR = (1UL << (LED_BIT + 16));
}


// State name for serial monitoring
const char* Mode_name(FanState state){

    switch(state){

        case FAN_OFF:   return "OFF";
        case FAN_MID:   return "MID";
        case FAN_HIGH:  return "HIGH";
        case FAN_PAUSE: return "PAUSE";
        default:        return "UNKNOWN";
    }
}


// Serial Display
void Print_status(void){

    int duty = 0;

    if(current_state == FAN_MID)
        duty = 50;
    else if(current_state == FAN_HIGH)
        duty = 100;

    char msg[160];

    snprintf(msg, sizeof(msg),
        "\r\n===== Smart Mini-Fan =====\r\n"
        "STATE : %s\r\n"
        "SAVED : %s\r\n"
        "DUTY  : %d%%\r\n"
        "SENSOR: %s\r\n",
        Mode_name(current_state),
        Mode_name(selected_mode),
        duty,
        paused ? "NEAR" : "AWAY");

    UART2_send(msg);
}


// TIM3 IRQ: Every 500 ms
void TIM3_IRQHandler(void){

    if(is_UIF(TIM3)){

        clear_UIF(TIM3);

        timer_count++;

        // LED blink: Toggle every 500 ms
        led_blink ^= 1U;

        // UART output every 2 seconds
        if((timer_count % 4U) == 0U){
            print_event = 1;
        }

        // Re-arm B1 after button release
        if(button_locked &&
           (GPIOC->IDR & (1UL << 13))){

            button_locked = 0;
        }
    }
}


// B1 External Interrupt
void EXTI15_10_IRQHandler(void){

    if(is_pending_EXTI(BUTTON_PIN)){

        clear_pending_EXTI(BUTTON_PIN);

        // Accept one event per button press
        if(!button_locked &&
           !(GPIOC->IDR & (1UL << 13))){

            button_locked = 1;
            button_event = 1;
        }
    }
}


// USART2 TX Initialization
void UART2_TX_setup(void){

    // GPIOA and USART2 peripheral clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2: Alternate Function Mode
    GPIOA->MODER &= ~(3UL << 4);
    GPIOA->MODER |=  (2UL << 4);

    // PA2: AF7 (USART2_TX)
    GPIOA->AFR[0] &= ~(0xFUL << 8);
    GPIOA->AFR[0] |=  (7UL << 8);

    // Push-Pull, Pull-Up
    GPIOA->OTYPER &= ~(1UL << 2);
    GPIOA->PUPDR &= ~(3UL << 4);
    GPIOA->PUPDR |=  (1UL << 4);

    // Update CMSIS system clock variable
    SystemCoreClockUpdate();

    // Calculate APB1 peripheral clock
    uint32_t ppre1 = (RCC->CFGR >> 10) & 7UL;
    uint32_t div = (ppre1 < 4U)
                 ? 1U : (1UL << (ppre1 - 3U));

    uint32_t pclk1 = SystemCoreClock / div;

    // USART2: 9600 baud, 8N1, oversampling 16
    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;

    USART2->BRR = (pclk1 + 4800UL) / 9600UL;

    // Enable TX and USART
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}


// Send string to Tera Term
void UART2_send(const char *str){

    while(*str){

        while(!(USART2->SR & USART_SR_TXE)){
        }

        USART2->DR = (uint8_t)(*str++);
    }
}
