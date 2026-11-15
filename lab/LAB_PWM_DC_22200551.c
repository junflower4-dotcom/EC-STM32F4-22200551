
/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 08-11-2026
Modified         : 10-11-2026
Language/ver     : C in VS Code

Description      : LAB_PWM_DC - Smart Mini-Fan
                   Control DC motor speed using PWM and FSM.
                   Pause and resume using an IR sensor.
                   Display motor status using USART2.
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

// Pin Configuration
#define PWM_PIN         PA_0     // TIM2_CH1
#define DIR_PIN         PC_2     // Motor Direction
#define BUTTON_PIN      PC_13    // USER Button B1
#define IR_PIN          PB_0     // IR Proximity Sensor
#define LED_PIN         PA_5     // Onboard LED LD2

// Configuration
#define PWM_PERIOD_US   1000     // 1 ms = 1 kHz
#define MOTOR_DIR       LOW      // Fixed Motor Direction
#define IR_NEAR_LEVEL   HIGH      // Light blocked: NEAR
#define DEBOUNCE_MS     150

// FSM States
typedef enum {
    FAN_OFF = 0,
    FAN_MID,
    FAN_HIGH,
    FAN_PAUSE
} FanState;

// Global Variables
volatile uint8_t button_event = 0;
volatile uint8_t print_event = 0;
volatile uint8_t led_blink = 1;
volatile uint32_t timer_count = 0;

FanState selected_mode = FAN_OFF;
FanState current_state = FAN_OFF;

// Function Prototypes
void setup(void);
uint8_t IR_isNear(void);
void Fan_update(void);
void LED_update(void);
void Print_status(void);
const char* Mode_name(FanState state);


// Main Function
int main(void){

    setup();

    printf("Smart Mini-Fan Initialized\r\n");
    Print_status();

    while(1){

        // 1. Read IR Sensor
        uint8_t near = IR_isNear();

        // 2. Read and Clear Button Event
        uint8_t pressed;

        __disable_irq();
        pressed = button_event;
        button_event = 0;
        __enable_irq();

        // 3. FSM: Change Mode
        // Button inputs are ignored during PAUSE
        if(!near && pressed){

            if(selected_mode == FAN_OFF){
                selected_mode = FAN_MID;
            }
            else if(selected_mode == FAN_MID){
                selected_mode = FAN_HIGH;
            }
            else{
                selected_mode = FAN_OFF;
            }
        }

        // 4. FSM: Determine Current State
        if(near){
            current_state = FAN_PAUSE;
        }
        else{
            current_state = selected_mode;
        }

        // 5. Update PWM and LED
        Fan_update();
        LED_update();

        // 6. UART Output Every 2 Seconds
        if(print_event){

            print_event = 0;
            Print_status();
        }
    }
}


// Initialization
void setup(void){

    // 1. MCU Initialization: PLL 84 MHz
    MCU_init();

    // 2. Motor Direction Pin: PC2
    GPIO_init(DIR_PIN, OUTPUT);
    GPIO_otype(DIR_PIN, PUSH_PULL);
    GPIO_write(DIR_PIN, MOTOR_DIR);

    // 3. LED Pin: PA5
    GPIO_init(LED_PIN, OUTPUT);
    GPIO_otype(LED_PIN, PUSH_PULL);
    GPIO_write(LED_PIN, LOW);

    // 4. IR Sensor: PB0, Input Pull-Up
    GPIO_init(IR_PIN, INPUT);
    GPIO_pupd(IR_PIN, PU);

    // 5. USER Button: PC13, Input Pull-Up
    GPIO_init(BUTTON_PIN, INPUT);
    GPIO_pupd(BUTTON_PIN, PU);

    // 6. PWM: TIM2_CH1, Period = 1000 us
    PWM_init_us(PWM_PIN, PWM_PERIOD_US);

    // Initialize PWM Output to Motor OFF
    PWM_duty(PWM_PIN, (float)MOTOR_DIR);

    // 7. USART2: PC Communication
    USART2_init();
    USART2_baud(BAUD_9600);

    // Disable USART2 RX Interrupt
    USART2->CR1 &= ~USART_CR1_RXNEIE;
    NVIC_DisableIRQ(USART2_IRQn);

    // 8. External Interrupt: B1 Button
    EXTI_init(BUTTON_PIN, FALL, 0);
    clear_pending_EXTI(BUTTON_PIN);

    // 9. TIM3 Update Interrupt: 500 ms
    TIM_UI_init(TIM3, 500);
}


// Check IR Sensor
uint8_t IR_isNear(void){

    return (GPIO_read(IR_PIN) == IR_NEAR_LEVEL);
}


// Update Motor PWM Duty Ratio
void Fan_update(void){

    static int last_duty = -1;

    int target_duty = 0;

    // Determine Target Duty Ratio
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

    // Update PWM Only When Duty Changes
    if(target_duty != last_duty){

        // Convert 0~100% to 0.0~1.0
        float targetPWM = target_duty / 100.0f;

        // PWM Compensation According to DIR
        float duty =
            fabsf((float)MOTOR_DIR - targetPWM);

        // Set PWM Duty Ratio
        PWM_duty(PWM_PIN, duty);

        last_duty = target_duty;
    }
}


// Update LED According to FSM
void LED_update(void){

    uint8_t led_state = LOW;

    if(current_state == FAN_PAUSE){

        // Blink Every 500 ms
        led_state = led_blink;
    }
    else if(current_state == FAN_MID ||
            current_state == FAN_HIGH){

        led_state = HIGH;
    }
    else{
        led_state = LOW;
    }

    GPIO_write(LED_PIN, led_state);
}


// Convert State to String
const char* Mode_name(FanState state){

    switch(state){

        case FAN_OFF:
            return "OFF";

        case FAN_MID:
            return "MID";

        case FAN_HIGH:
            return "HIGH";

        case FAN_PAUSE:
            return "PAUSE";

        default:
            return "UNKNOWN";
    }
}


// Print Motor Status Using USART2
void Print_status(void){

    int target_duty = 0;

    if(current_state == FAN_MID){
        target_duty = 50;
    }
    else if(current_state == FAN_HIGH){
        target_duty = 100;
    }

    printf("\r\n===== Smart Mini-Fan =====\r\n");
    printf("Mode       : %s\r\n",
           Mode_name(current_state));
    printf("Saved Mode : %s\r\n",
           Mode_name(selected_mode));
    printf("PWM Duty   : %d%%\r\n", target_duty);
    printf("IR Sensor  : %s\r\n",
           IR_isNear() ? "NEAR" : "AWAY");
}


// TIM3 Interrupt Handler: Every 500 ms
void TIM3_IRQHandler(void){

    if(is_UIF(TIM3)){

        clear_UIF(TIM3);

        // Toggle LED Blink State
        led_blink ^= 1U;

        // Count Timer Interrupts
        timer_count++;

        // Generate UART Event Every 2 Seconds
        if(timer_count >= 4){

            timer_count = 0;
            print_event = 1;
        }
    }
}


// External Interrupt Handler: USER Button
void EXTI15_10_IRQHandler(void){

    static uint32_t last_press_ms = 0;
    static uint8_t first_press = 1;

    if(is_pending_EXTI(BUTTON_PIN)){

        clear_pending_EXTI(BUTTON_PIN);

        uint32_t now = msTicks;

        // Accept Button Only When Sensor is AWAY
        if(GPIO_read(BUTTON_PIN) == LOW &&
           !IR_isNear()){

            // Software Debouncing
            if(first_press ||
               (uint32_t)(now - last_press_ms)
                   >= DEBOUNCE_MS){

                button_event = 1;

                last_press_ms = now;
                first_press = 0;
            }
        }
    }
}
