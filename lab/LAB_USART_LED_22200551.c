/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 18-10-2026
Modified         : 18-10-2026 
Language/ver     : C in VS Code

Description      :LAB_USART_LED - Bidirectional communication between two MCUs using USART1 and USART2.
                  Remote LED control using H/L commands.
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

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

void setup(void){

	MCU_init();
		
    GPIO_init(LED_PIN, OUTPUT);
    GPIO_write(LED_PIN, LOW);

	// USART2: USB serial init
	// PC-MCU
	USART2_init();  // default 9600
	USART2_baud(BAUD_9600);
	

	// USART1: MCU2 serial init 
	// MCU - MCU
	USART1_init(); // default 9600
	USART1_baud(BAUD_9600);
}

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
			else if(len == 1 && message[0] == 'L')
			{
				GPIO_write(LED_PIN, LOW);
				printf("LED OFF\r\n");
			}

		}
		// USART Receive: Use Interrupt only
		// USART Transmit: Polling
		//USART2_write(PC_string, 7);	       
	}
}

/*================ USART2 RX Interrupt ================*/

// Receive characters from PC (Tera Term)
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

/*================ USART1 RX Interrupt ================*/

// Receive characters from other MCU
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