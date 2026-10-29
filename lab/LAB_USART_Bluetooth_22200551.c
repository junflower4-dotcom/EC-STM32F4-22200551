/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 18-10-2026
Modified         : 20-10-2026 
Language/ver     : C in VS Code

Description      :LAB_USART_Bluetooth - Wireless LED and stepper motor control using HC-06(18) Bluetooth via USART1.
                  H/L: LED ON/OFF
                  J/K: CW/CCW rotation (2 revolutions)
                  P: Emergency STOP  
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

#define LED_PIN         PA_5

#define STEPPER_MODE    FULL
#define MOTOR_RPM       5
#define MOTOR_REV       2
#define TOTAL_STEPS      (FULL_STEP_PER_REV * MOTOR_REV)

// Stepper motor pins: A, B, A', B'
PinName_t pinStepper[4] = {PB_10, PB_4, PB_5, PB_3};

volatile uint8_t bt_command = 0;        // Bluetooth Command
volatile uint8_t stop_requested = 0;


// Send a string to Bluetooth Tera Term
void BT_print(const char *text) {
    uint32_t len = 0;

    while(text[len] != '\0'){
        len++;
    }

    if(len > 0){
        USART1_write((uint8_t *)text, len);
    }
}

// Echo received command to Bluetooth Tera Term
void BT_echo(uint8_t command) {
    BT_print("RX: ");
    USART1_write(&command, 1);
    BT_print("\r\n");
}


void setup(void) {
    
    MCU_init();

    // LED initialization
    GPIO_init(LED_PIN, OUTPUT);
    GPIO_write(LED_PIN, LOW);

    // USART1: MCU - HC-06(18)
    USART1_init();
    USART1_baud(BAUD_9600);

    // Stepper motor: FULL MODE, 5 RPM
    stepper_init(STEPPER_MODE, pinStepper);
    stepper_speed(MOTOR_RPM);
    stepper_stop();

}


int main(void) {

    uint8_t command = 0;
    uint32_t remaining_steps = 0;
    uint32_t direction = DIR_CW;

    setup();

    BT_print("\r\nBluetooth Initialized\r\n");
    BT_print("Mode: FULL, Speed: 5 RPM\r\n");
    BT_print("H/L: LED ON/OFF\r\n");
    BT_print("J/K: CW/CCW (2 REV)\r\n");
    BT_print("P: Emergency STOP\r\n\r\n");

    while (1)
    {   
        // Emergency STOP takes priority over other commands.
        if(stop_requested){
            remaining_steps = 0;
            stepper_stop();

            // Discard commands received before the stop was handled.
            __disable_irq();
            stop_requested = 0;
            bt_command = 0;
            __enable_irq();

            BT_echo('P');
            BT_print("Emergency STOP\r\n");
            continue;
        }

        // Read the latest Bluetooth command atomically.
        __disable_irq();
        command = bt_command;
        bt_command = 0;
        __enable_irq();

        // Process Bluetooth command.
        switch(command){
            case 'H':
                BT_echo('H');
                GPIO_write(LED_PIN, HIGH);
                BT_print("LED ON\r\n");
                break;

            case 'L':
                BT_echo('L');
                GPIO_write(LED_PIN, LOW);
                BT_print("LED OFF\r\n");
                break;

            case 'J':
                BT_echo('J');
                direction = DIR_CW;
                remaining_steps = TOTAL_STEPS;
                BT_print("Motor CW: 2 REV\r\n");
                break;

            case 'K':
                BT_echo('K');
                direction = DIR_CCW;
                remaining_steps = TOTAL_STEPS;
                BT_print("Motor CCW: 2 REV\r\n");
                break;

            default:
                break;
        }

        // Move one step, then check for new commands.
        if(remaining_steps > 0 && !stop_requested){
            stepper_step(1, direction);
            remaining_steps--;

            // Handle P on the next loop iteration.
            if(stop_requested) continue;

            if(remaining_steps == 0){
                stepper_stop();
                BT_print("Motor Completed\r\n");
            }
        }
    }
}

// Bluetooth commands are handled character by character.
void USART1_IRQHandler(void){
    if(is_USART1_RXNE()){
        uint8_t BT_data = USART1_read();

        if(BT_data == 'P' || BT_data == 'p'){
            stop_requested = 1;
            bt_command = 0;
        }
        else if(!stop_requested){
            switch(BT_data){
                case 'H': case 'h': bt_command = 'H'; break;
                case 'L': case 'l': bt_command = 'L'; break;
                case 'J': case 'j': bt_command = 'J'; break;
                case 'K': case 'k': bt_command = 'K'; break;
                default: break; // Ignore CR, LF and other characters.
            }
        }
    }
}


