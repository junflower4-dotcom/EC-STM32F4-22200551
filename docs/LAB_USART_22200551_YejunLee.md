# LAB: USART - Bluetooth

**Date:** 2026-10-18

**Author/Partner:** 22200551 / Yejun Lee

**Github:** [junflower4-dotcom](https://github.com/junflower4-dotcom)

**Demo Video:** 
* Problem1 : [LAB_USART_LED](https://youtu.be/w4O4v8ye5gA)
* Problem2 : [LAB_USART_Bluetooth](https://youtu.be/a26jQ5XQ_p4)

**PDF version:** [LAB_USART_22200551_YejunLee.pdf](./LAB_USART_22200551_YejunLee.pdf)


## Introduction

In this lab, we will learn how to configure and use ‘USART(Universal synchronous asynchronous receiver transmitter)’ of MCU. Then, we will learn how to communicate between your PC and MCU and MCU to another MCU with wired serial communication.

* **Mission 1**: Control LED(LD2) of other MCU using wired serial communication
* **Mission 2**: Control a stepper motor with wireless Bluetooth communication

You must submit

* LAB Report (\*.md & \*.pdf)
* Zip source files(main\*.c, ecRCC.h, ecGPIO.h, ecSysTick.c etc...).
  * Only the source files. Do not submit project files

### Requirement

#### Hardware

* MCU
  * NUCLEO-F411RE
* Actuator/Sensor/Others:
  * DC motor, DC motor driver(L9110s),
  * Bluetooth Module(HC-06),

#### Software

* VS code, Tera Term


## Problem 1: Communicate MCU1-MCU2 using RS-232

![P1](./img/P1.jpg)

### Procedure

1\. Create a new project under the directory **`EC\LAB\`**

* The environment name is “**LAB_USART_LED”.**
* Create a new source files named as “**LAB_USART_LED_22200551.c”**


2\. Make sure you updated the library in `\EC\include\` &#x20;

* **ecUART2.h, ecUART2.c**
* Also, update **ecSTM32F4v2.h**

3\. Connect each MCUs to each PC with **USART 2** via USB cable (ST-Link)

* MCU1 to PC1
* MCU2 to PC2

4\. Connect MCU1 to MCU2 with **USART 1**

* Connect RX/TX pins externally with jumper wires as
  * MCU1\_TX to MCU2\_RXD
  * MCU1\_RX to MCU2\_TX

5\. Connect **GND pins** between MCU1-MCU2

 > MUST connect the same GND pin between MCU1 and MCU2


6\. Send a message from PC\_1 by typing keys on Teraterm.&#x20;

> Press "Enter" to end the message.

7\. The received message by MCU\_2 should be displayed on PC\_2.
8\. Turn other MCU's LED(LD2) ON/OFF by :

* Press key "**L**" to Turn OFF LED
* Press key "**H**" to Turn ON LED

### Configuration

| Function                        | Port - Pin                   | Configuration                                                      |
| ------------------------------- | ---------------------------- | ------------------------------------------------------------------ |
| System Clock                    |                              | PLL 84MHz                                                          |
| USART2 : USB cable (ST-Link)    |                              | <p>No Parity, 8-bit Data,<br>1-bit Stop bit,<br>9600 baud-rate</p> |
| USART1 : MCU1,  MCU2            | <p>TXD: PA9<br>RXD: PA10</p> | <p>No Parity, 8-bit Data,<br>1-bit Stop bit,<br>9600 baud-rate</p> |
| <p>Digital Out: LD2<br><br></p> | <p>PA5<br><br></p>           |                                                                    |

### Circuit/Wiring Diagram
![P1_circuit](./img/P1_circuit.jpg)


### Algorithm

#### Overview

*  Flowchart
  
<img src="./img/P1_flowchart.jpg" height="900">





#### Description with Code

**1. Global Variables and Definitions**

> Global variables and macros used for USART communication, message buffering, and LED control.

**Code**

```c
#define LED_PIN  PA_5
#define MAX_BUF  64
#define END_CHAR 13 // '\r'

// PC -> MCU (USART2)
volatile uint8_t pc_buffer[MAX_BUF] = {0};
volatile uint32_t pc_idx = 0;
volatile uint32_t pc_len = 0;
volatile uint8_t pc_ready = 0;
volatile uint8_t pc_overflow = 0;

// Other MCU -> MCU (USART1)
volatile uint8_t mcu_buffer[MAX_BUF] = {0};
volatile uint32_t mcu_idx = 0;
volatile uint32_t mcu_len = 0;
volatile uint8_t mcu_ready = 0;
volatile uint8_t mcu_overflow = 0;
```

**Description**

Macro Definitions

- `LED_PIN`: Defines PA5 as the LED output pin.
- `MAX_BUF`: Defines the maximum message length as 64 bytes.
- `END_CHAR`: Defines the carriage return (`'\r'`, ASCII 13) as the end of a message.

USART2 Variables (PC → MCU)

- `pc_buffer[]`: Stores characters received from the PC.
- `pc_idx`: Tracks the current buffer index.
- `pc_len`: Stores the length of the completed message.
- `pc_ready`: Indicates that a complete message is ready for processing.
- `pc_overflow`: Detects when the received message exceeds the buffer capacity.

USART1 Variables (MCU → MCU)

- `mcu_buffer[]`: Stores characters received from another MCU.
- `mcu_idx`: Tracks the current buffer index.
- `mcu_len`: Stores the length of the completed message.
- `mcu_ready`: Indicates that a complete message is ready for processing.
- `mcu_overflow`: Detects when the received message exceeds the buffer capacity.

The `volatile` keyword is used because these variables are shared between interrupt handlers and the main program.


**2. setup()**

> Initializes the MCU, LED GPIO pin, and USART1/USART2 communication at 9600 baud.

**Code**

```c
void setup(void){

    MCU_init();

    GPIO_init(LED_PIN, OUTPUT);
    GPIO_write(LED_PIN, LOW);

    // USART2: PC-MCU
    USART2_init();
    USART2_baud(BAUD_9600);

    // USART1: MCU-MCU
    USART1_init();
    USART1_baud(BAUD_9600);
}
```

**Description**

- Initializes the MCU with an 84 MHz system clock.
- Configures PA5 as a digital output and initially turns OFF the LED.
- Initializes USART2 for PC-MCU communication at 9600 baud.
- Initializes USART1 for MCU-MCU communication at 9600 baud.
- Enables USART receive interrupts for both communication channels.

**Parameter**

- None.

**Return**

- `void`: No return value.

**Example Code**

```c
int main(void){

    setup();

    while(1){
        // Main program
    }
}
```

Initializes the LED as OFF and configures USART1 and USART2 for serial communication.


**3. main()**

> Processes received messages, forwards data between two MCUs, and controls the LED using received commands.

**Code**

```c
int main(void){

    uint8_t message[MAX_BUF];
    uint8_t end_char = END_CHAR;
    uint8_t newline[] = "\r\n";

    setup();

    printf("MCU Initialized\r\n");

    while(1) {

        if(pc_ready) {

            uint32_t len = pc_len;

            for(uint32_t i = 0; i < len; i++) {
                message[i] = pc_buffer[i];
            }

            pc_ready = 0;

            USART1_write(message, len);
            USART1_write(&end_char, 1);
        }

        if(mcu_ready) {

            uint32_t len = mcu_len;

            for(uint32_t i = 0; i < len; i++) {
                message[i] = mcu_buffer[i];
            }

            mcu_ready = 0;

            USART2_write((uint8_t*)"RX: ", 4);
            USART2_write(message, len);
            USART2_write(newline, 2);

            if(len == 1 && message[0] == 'H') {
                GPIO_write(LED_PIN, HIGH);
                printf("LED ON\r\n");
            }
            else if(len == 1 && message[0] == 'L') {
                GPIO_write(LED_PIN, LOW);
                printf("LED OFF\r\n");
            }
        }
    }
}
```

**Description**

- Initializes the system and displays a startup message on Tera Term.
- Checks `pc_ready` and forwards completed PC messages to another MCU through USART1.
- Checks `mcu_ready` and displays received MCU messages on the PC terminal through USART2.
- Turns the LED ON when a single `H` is received.
- Turns the LED OFF when a single `L` is received.
- Uses interrupt-based reception and polling-based transmission.

**Parameter**

- None.

**Return**

- `int`: Declared return type; the infinite loop prevents normal return.

**Example Code**

```c
// PC1 sends:
H + Enter

// Receiving MCU2:
GPIO_write(LED_PIN, HIGH);

// PC2 Tera Term:
RX: H
LED ON
```

Sending `H` from PC1 turns ON the LED of MCU2, while sending `L` turns it OFF.

The same operation can be performed in the opposite direction.


**4. USART2_IRQHandler()**

> Receives characters from the PC through USART2 and stores them in a buffer until the Enter key is pressed.

**Code**

```c
void USART2_IRQHandler(void)
{
    if (is_USART2_RXNE())
    {
        uint8_t PC_data = USART2_read();

        // Ignore new data until previous message is processed
        if (pc_ready) return;

        USART2_write(&PC_data, 1);

        // Enter key: End of message
        if (PC_data == END_CHAR)
        {
            uint8_t LF = '\n';
            USART2_write(&LF, 1);

            if (pc_idx > 0 && !pc_overflow)
            {
                pc_len = pc_idx;
                pc_ready = 1;
            }

            pc_idx = 0;
            pc_overflow = 0;
        }
        else
        {
            // Store received character
            if (!pc_overflow)
            {
                if (pc_idx < MAX_BUF)
                {
                    pc_buffer[pc_idx++] = PC_data;
                }
                else
                {
                    pc_overflow = 1;
                }
            }
        }
    }
}
```

**Description**

- Checks the USART2 RXNE flag to detect received data.
- Reads and echoes the received character to the PC terminal.
- Stores received characters in `pc_buffer`.
- Detects the Enter key (`'\r'`) as the end of a message.
- Sets `pc_ready` when a valid message is completed.
- Uses `pc_overflow` to prevent buffer overflow.
- Allows the main loop to forward completed messages through USART1.

**Parameter**

- None (Automatically called by the USART2 receive interrupt).

**Return**

- `void`: No return value.

**Example Code**

```c
// PC sends:
H + Enter

// Received characters:
'H', '\r'

// Buffer after receiving 'H':
pc_buffer[0] = 'H';

// After receiving '\r':
pc_len = 1;
pc_ready = 1;
```

The completed message is then forwarded to another MCU through USART1 in `main()`.


**5. USART1_IRQHandler()**

> Receives characters from another MCU through USART1 and stores them in a buffer until the end character is detected.

**Code**

```c
void USART1_IRQHandler(void)
{
    if (is_USART1_RXNE())
    {
        uint8_t MCU_data = USART1_read();

        // Ignore new data until previous message is processed
        if (mcu_ready) return;

        // End of message
        if (MCU_data == END_CHAR)
        {
            if (mcu_idx > 0 && !mcu_overflow)
            {
                mcu_len = mcu_idx;
                mcu_ready = 1;
            }

            mcu_idx = 0;
            mcu_overflow = 0;
        }
        else
        {
            // Store received character
            if (!mcu_overflow)
            {
                if (mcu_idx < MAX_BUF)
                {
                    mcu_buffer[mcu_idx++] = MCU_data;
                }
                else
                {
                    mcu_overflow = 1;
                }
            }
        }
    }
}
```

**Description**

- Checks the USART1 RXNE flag to detect received data.
- Reads characters transmitted from another MCU.
- Stores received characters in `mcu_buffer`.
- Detects the carriage return (`'\r'`) as the end of a message.
- Sets `mcu_ready` when a valid message is completed.
- Uses `mcu_overflow` to prevent buffer overflow.
- Allows the main loop to display received messages and control the LED.

**Parameter**

- None (Automatically called by the USART1 receive interrupt).

**Return**

- `void`: No return value.

**Example Code**

```c
// Other MCU sends:
L + Enter

// Buffer after receiving 'L':
mcu_buffer[0] = 'L';

// After receiving '\r':
mcu_len = 1;
mcu_ready = 1;

// LED control in main():
GPIO_write(LED_PIN, LOW);
```

The received message is displayed on the PC terminal, and the LED turns OFF.



### Result

**1. Message**

<img src="./img/P1_result_message.jpg" width="550">

**2. LED Left to Right ON / OFF**

| LED ON | LED OFF |
|:---:|:---:|
| <img src="./img/P1_result_led_left_to_right_ON.jpg" width="300"> | <img src="./img/P1_result_led_left_to_right_OFF.jpg" width="300"> |

**3. LED Right to Left ON / OFF**

| LED ON | LED OFF |
|:---:|:---:|
| <img src="./img/P1_result_led_right_to_left_ON.jpg" width="300"> | <img src="./img/P1_result_led_right_to_left_OFF.jpg" width="300"> |




> The bidirectional USART communication between the PC and two STM32 MCUs was implemented successfully. USART2 was used for communication between the PC terminal and the first MCU, while USART1 was used for communication between the two MCUs. Both USART peripherals were configured with a baud rate of 9600 bps, 8 data bits, no parity bit, and 1 stop bit.

> When a character was entered in the PC serial terminal, the USART2 receive interrupt stored the received data in `pc_buffer`. After the Enter key was pressed, the carriage return character (`'\r'`) was detected and the message was marked as ready using `pc_ready`. The main loop then forwarded the received message to the other MCU through USART1.

> The receiving MCU collected the USART1 data through `USART1_IRQHandler()`. When the message was completed by receiving `'\r'`, the main loop checked the command. The LED connected to PA5 turned on when the received command was `H` and turned off when the received command was `L`. In addition, the received command and LED status were displayed on the PC terminal through USART2.

| Test case | PC input | Expected result | Observed result |
|---|---|---|---|
| LED ON command | `H` + Enter | LED connected to PA5 turns ON | LED ON |
| LED OFF command | `L` + Enter | LED connected to PA5 turns OFF | LED OFF |
| Message forwarding | Character + Enter | PC message is transmitted from USART2 to USART1 | Message received by the other MCU |
| Message display | Received MCU command | `RX: <message>` is displayed on PC terminal | Message displayed correctly |
|

Add: https://youtu.be/w4O4v8ye5gA


### Discussion

> This program uses interrupt-driven reception and polling-based transmission for USART communication. The USART interrupt handlers store incoming characters in buffers and detect the end of a message when the carriage return character (`'\r'`) is received. The `pc_ready` and `mcu_ready` flags notify the main loop that a complete message is available for forwarding or LED control. These shared variables are declared as `volatile` because they can be updated asynchronously by interrupts.

> The program successfully forwards commands from the PC to another MCU through USART1. When the receiving MCU obtains `H`, it turns on the PA5 LED; when it receives `L`, it turns the LED off. The overflow flags prevent messages longer than 64 bytes from being processed, improving reliability. An acknowledgement message from the receiving MCU could be added in future work to confirm successful command execution.

***

## Problem 2: Control Stepper Motor via Bluetooth

![P2](./img/P2.jpg)

### Procedure

1\. Create a new project&#x20;

* The environment name is “**LAB_USART_Bluetooth”.**
* Create a new source files named as “**LAB_USART_Bluetooth_22200551.c”**


2\. Make sure your updated library in `include\` to your project.

* **ecUART2.h, ecUART2.c**
* Update **ecSTM32F4v2.h**

3\. Connect the MCU-BT module. Use USART 1

* Connect RX/TX pins as
  * **MCU\_TX to  BT\_RX**
  * **MCU\_ RX to BT\_TX**

4\.  Connect the BT module (HC-06) to the PC

1. Check the Bluetooth connection by turning MCU's LED(LD2) ON/OFF by sending text of "**L**" or "**H**" from the PC (serial terminal)
2. Connect a stepper motor to MCU  (**see Lab: Stepper**)&#x20;
3. Control Step motor by the Keyboard inputs

* Rotate the stepper motor 2 revolutions with an appropriate speed&#x20;
* Press 'J' : To run CW direction&#x20;
* Press 'K' : To run CCW direction
* Press 'P' : Emergency STOP


### Configuration
#### Connecting to USART1
![P2_Bluetooth](./img/P2_Bluetooth.jpg)


| Function                                         | Port - Pin                   | Configuration                                                      |
| ------------------------------------------------ | ---------------------------- | ------------------------------------------------------------------ |
| **System Clock**                                 |                              | PLL 84MHz                                                          |
| **USART1 : MCU -Bluetooth**                      | <p>TXD: PA9<br>RXD: PA10</p> | <p>No Parity, 8-bit Data,<br>1-bit Stop bit,<br>9600 baud-rate</p> |
| <p><strong>Digital Out: LD2</strong><br><br></p> | <p>PA5<br><br></p>           |                                                                    |
| **Motor Pin A, B, A', B'**                       | PB10, PB4, PB5, PB3          | DOUT                                                               |
|                                                  |                              |                                                                    |

### Circuit/Wiring Diagram
![P2_configure](./img/P2_configure.jpg)

### Algorithm

#### Overview

*  Flowchart
<img src="./img/P2_flowchart.jpg" height="900">

#### Description with Code

**1. Global Variables and Definitions**

> Global variables and macros used for Bluetooth communication, LED control, and stepper motor operation.

**Code**

```c
#define LED_PIN         PA_5

#define STEPPER_MODE    FULL
#define MOTOR_RPM       5
#define MOTOR_REV       2
#define TOTAL_STEPS     (FULL_STEP_PER_REV * MOTOR_REV)

// Stepper motor pins: A, B, A', B'
PinName_t pinStepper[4] = {PB_10, PB_4, PB_5, PB_3};

volatile uint8_t bt_command = 0;
volatile uint8_t stop_requested = 0;
```

**Description**

Macro Definitions

- `LED_PIN`: Defines PA5 as the LED output pin.
- `STEPPER_MODE`: Sets the motor to FULL STEP mode.
- `MOTOR_RPM`: Sets the requested motor speed to 5 RPM.
- `MOTOR_REV`: Specifies 2 revolutions.
- `TOTAL_STEPS`: Calculates 4096 steps for 2 revolutions.

Global Variables

- `pinStepper[]`: Defines the four GPIO pins for stepper motor control.
- `bt_command`: Stores the latest Bluetooth command.
- `stop_requested`: Indicates whether an emergency stop is requested.

The `volatile` keyword is used because the command variables are shared between the interrupt handler and the main program.


**2. BT_print()**

> Sends a text message from the MCU to the Bluetooth Tera Term through USART1.

**Code**

```c
void BT_print(const char *text) {
    uint32_t len = 0;

    while(text[len] != '\0'){
        len++;
    }

    if(len > 0){
        USART1_write((uint8_t *)text, len);
    }
}
```

**Description**

- Calculates the length of the input string until the null character (`'\0'`).
- Transmits the string through USART1 using `USART1_write()`.
- Displays the transmitted message on the Bluetooth Tera Term.

**Parameter**

- `text`: Pointer to the null-terminated string to be transmitted.

**Return**

- `void`: No return value.

**Example Code**

```c
BT_print("LED ON\r\n");
```

Tera Term Output:

```text
LED ON
```


**3. BT_echo()**

> Sends the received Bluetooth command back to the PC terminal for confirmation.

**Code**

```c
void BT_echo(uint8_t command) {
    BT_print("RX: ");
    USART1_write(&command, 1);
    BT_print("\r\n");
}
```

**Description**

- Prints `"RX: "` to indicate a received command.
- Transmits the received command character through USART1.
- Adds a carriage return and line feed (`"\r\n"`).

**Parameter**

- `command`: Received Bluetooth command character.

**Return**

- `void`: No return value.

**Example Code**

```c
BT_echo('H');
```

Tera Term Output:

```text
RX: H
```


**4. setup()**

> Initializes the MCU, LED, USART1 Bluetooth communication, and stepper motor.

**Code**

```c
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
```

**Description**

- Initializes the MCU with an 84 MHz system clock.
- Configures PA5 as a digital output and initially turns OFF the LED.
- Initializes USART1 for Bluetooth communication at 9600 baud.
- Configures the stepper motor in FULL STEP mode.
- Sets the requested motor speed to 5 RPM and stops the motor initially.

**Parameter**

- None.

**Return**

- `void`: No return value.

**Example Code**

```c
int main(void) {
    setup();

    while(1){
        // Main program
    }
}
```

Initializes the communication peripheral and actuators before executing the main program.


**5. main()**

> Processes Bluetooth commands, controls the LED and stepper motor, and handles emergency stop requests.

**Code**

```c
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
```

**Description**

- Initializes the system and displays Bluetooth instructions on Tera Term.
- Checks `stop_requested` first to prioritize emergency stop commands.
- Reads the latest Bluetooth command shared with the USART1 interrupt handler.
- Controls the LED using `H` (ON) and `L` (OFF).
- Sets the motor direction and 4096 steps using `J` (CW) or `K` (CCW).
- Executes the motor one step at a time to allow new commands to be processed.
- Stops the motor when all steps are completed or an emergency stop is requested.
- Displays the command and motor status through Bluetooth.

**Parameter**

- None.

**Return**

- `int`: Declared return type; the infinite loop prevents normal return.

**Example Code**

```text
// PC sends:
H
J

// Bluetooth Tera Term:
RX: H
LED ON
RX: J
Motor CW: 2 REV
Motor Completed
```

Sending `H` turns ON the LED, while `J` rotates the stepper motor clockwise for 2 revolutions.

Sending `P` during motor rotation cancels the remaining steps and stops the motor.


**6. USART1_IRQHandler()**

> Receives Bluetooth commands through USART1 using an interrupt and updates the command variables.

**Code**

```c
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
```

**Description**

- Checks the USART1 RXNE flag to detect received data.
- Reads the received character using `USART1_read()`.
- Sets `stop_requested` when `P` or `p` is received.
- Stores valid commands (`H`, `L`, `J`, `K`) in `bt_command`.
- Supports both uppercase and lowercase input.
- Ignores Enter and other undefined characters.
- Allows the main loop to process Bluetooth commands independently of the interrupt handler.

**Parameter**

- None (Automatically called by the USART1 receive interrupt).

**Return**

- `void`: No return value.

**Example Code**

```c
// PC sends:
J

// USART1 interrupt:
bt_command = 'J';

// Main loop:
direction = DIR_CW;
remaining_steps = TOTAL_STEPS;
```

The main loop starts clockwise motor rotation after receiving the Bluetooth command.

If `P` is received during rotation, `stop_requested` is set to 1, allowing the main loop to stop the motor.



### Result


#### Experiment Images and Results

**1. Bluetooth LED Control**

<img src="./img/P2_Result_LED_ON_OFF_Comparison.jpg" width="600">

*Figure 1. LED ON and OFF using Bluetooth commands H and L.*

The LED was controlled wirelessly through USART1 using the HC-06 Bluetooth module. The `H` command turned the LED ON, while the `L` command turned it OFF. The received commands and corresponding status messages were displayed on Tera Term.

**2. Stepper Motor Rotation**

| Clockwise (CW) | Counterclockwise (CCW) |
|:---:|:---:|
| <img src="./img/P2_Result_03_Motor_CW.jpg" width="320"> | <img src="./img/P2_Result_04_Motor_CCW.jpg" width="320"> |

*Figure 2. Stepper motor rotation using J (CW) and K (CCW) commands.*

The `J` and `K` commands controlled the stepper motor in clockwise and counterclockwise directions, respectively. The motor was configured in FULL STEP mode with 4096 steps for 2 revolutions and a requested speed of 5 RPM.

**3. Emergency STOP**

<img src="./img/P2_Result_05_Emergency_STOP.jpg" width="450">

*Figure 3. Emergency STOP using the P command during motor rotation.*

When the `P` command was received during rotation, the remaining steps were canceled, and the motor stopped. The message "Emergency STOP" was displayed on Tera Term.

**4. LED Control During Motor Rotation**

<img src="./img/P2_Result_06_LED_During_Rotation.jpg" width="450">

*Figure 4. LED control while the stepper motor is rotating.*

The LED could be turned ON or OFF while the stepper motor was rotating. The motor continued its rotation without restarting, demonstrating independent LED and motor control.


> The wireless control system was implemented using the HC-06 Bluetooth module and USART1 at 9600 baud. Commands were transmitted from the PC through Bluetooth using Tera Term and received by the MCU through the USART1 interrupt. The received commands and corresponding status messages were sent back to the PC terminal.

> The `H` and `L` commands were used to turn the LED ON and OFF. The `J` and `K` commands controlled the stepper motor in clockwise (CW) and counterclockwise (CCW) directions, respectively, with 4096 full steps for 2 revolutions. The motor was controlled one step at a time, allowing LED commands to be processed during rotation. When `P` was received, the remaining steps were canceled, and the motor was stopped.

| Test case | PC input | Expected result |
|---|---|---|
| LED ON | `H` | LED turns ON |
| LED OFF | `L` | LED turns OFF |
| Clockwise rotation | `J` | Motor rotates CW for 2 revolutions |
| Counterclockwise rotation | `K` | Motor rotates CCW for 2 revolutions |
| Emergency STOP | `P` | Motor stops during rotation |
| Simultaneous control | `H` during rotation | LED turns ON while the motor continues rotating |
| Bluetooth feedback | Any valid command | Command and status displayed on Tera Term |

Add: https://youtu.be/a26jQ5XQ_p4

### Discussion

> This program uses interrupt-driven reception and polling-based transmission for wireless Bluetooth communication through USART1. The interrupt handler stores incoming commands in `bt_command`, while the main loop controls the LED and stepper motor. The LED and motor can operate independently because they use different GPIO pins, and the motor is controlled one step at a time.

> The stepper motor is configured in FULL STEP mode with 4096 steps for 2 revolutions. The `stop_requested` flag gives the emergency stop command priority over other commands, allowing the remaining steps to be canceled during rotation. However, the blocking delay in `stepper_step()` may cause a short delay before the motor stops. Additionally, the integer-based step delay causes a difference between the requested 5 RPM and the actual motor speed. Using a non-blocking timer and more precise delay control could improve the system's responsiveness and speed accuracy.

## Reference
-  HGU, Embedded Controller Course GitBook: [LAB: USART - Bluetooth | EC](https://ykkim.gitbook.io/ec/ec-course/lab/lab-usart-led-bluetooth)
-  HGU, Embedded Controller Course GitBook: [Tutorial: USART with Tera Term | EC](https://ykkim.gitbook.io/ec/ec-course/tutorial/tutorial-usart-with-teraterm)
-  HGU, Embedded Controller Course GitBook: [Tutorial: Bluetooth | EC](https://ykkim.gitbook.io/ec/ec-course/tutorial/tutorial-bluetooth)

## Troubleshooting

### Problem 1: MCU-to-MCU USART Communication

1. Communication Failure Between Two MCUs

- Problem: Messages may not be transmitted correctly between MCU1 and MCU2.
- Cause: Incorrect TX/RX wiring, missing common GND, or mismatched baud rates.
- Solution: Connect USART1 TX (PA9) to the other MCU's RX (PA10), and vice versa. Connect the GND pins and configure both MCUs to 9600 baud.

2. Message Not Forwarded or LED Not Responding

- Problem: The receiving MCU may not respond to a command.
- Cause: The program detects the carriage return (`'\r'`) as the end of a message. LED control also requires a single-character command.
- Solution: Configure Tera Term to send CR when Enter is pressed. Send `H` or `L` followed by Enter to control the LED.

### Problem 2: Bluetooth Stepper Motor Control

1. No Output on Bluetooth Tera Term

- Problem: Messages sent using `printf()` were not displayed on the Bluetooth Tera Term.
- Cause: The `printf()` function was configured to transmit through USART2, while the HC-06 Bluetooth module was connected to USART1.
- Solution: Implement `BT_print()` using `USART1_write()` to send messages directly through Bluetooth.

2. Emergency STOP Response Delay

- Problem: The motor may not stop immediately when `P` is received during rotation.
- Cause: The `stepper_step()` function uses a blocking delay, preventing the main loop from handling the stop request until the current step finishes.
- Solution: Execute the motor one step at a time using `stepper_step(1, direction)` and check `stop_requested` between steps.

3. Stepper Motor Speed Error

- Problem: The actual motor speed may differ from the requested 5 RPM.
- Cause: The step delay is calculated using integer arithmetic, truncating the required 5.859 ms delay to 5 ms.
- Solution: Use more precise timer-based control or fractional delay calculations to improve motor speed accuracy.