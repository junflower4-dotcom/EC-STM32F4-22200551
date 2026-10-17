#include "ecSTM32F4v2.h"  // this should include 
#include "ecUART2_student.h"
 
#define MAX_BUF 	10
#define END_CHAR 	13

static volatile uint8_t buffer[MAX_BUF]={0, };
static volatile uint8_t PC_string[MAX_BUF]={0, };
static volatile uint8_t PC_data = 0;

static volatile int idx = 0;
static volatile int bReceive =0;

void setup(void){
	RCC_PLL_init();
	SysTick_init();
	
	// USART2: USB serial init
	UART2_init();
	UART2_baud(BAUD_9600);

	// USART1: BT serial init 
	UART1_init();
	UART1_baud(BAUD_9600);
}

int main(void){	
	setup();
	printf("MCU Initialized\r\n");	
	
	while(1){
		if (bReceive == 1){
			printf("PC_string: %s\r\n", PC_string);				
			bReceive = 0;
		}
	}
}

void USART2_IRQHandler(){          		// USART2 RX Interrupt : Recommended
	if(is_USART2_RXNE()){
		PC_data = USART2_read();		// RX from UART2 (PC)
		USART2_write(&PC_data,1);		// TX to USART2	 (PC)	 Echo of keyboard typing
		
		// Creates a String from serial character receive				
		if(PC_data != END_CHAR && (idx < MAX_BUF)){	
			buffer[idx] = PC_data;
			idx++;
		}
		else if (PC_data== END_CHAR) {						
			bReceive = 1;
			// reset PC_string;
			memset(PC_string, 0, sizeof(char) * MAX_BUF);
			// copy to PC_string;
			memcpy(PC_string, buffer, sizeof(char) * idx);
			// reset buffer
			memset(buffer, 0, sizeof(char) * MAX_BUF);	
			idx = 0;
		}
		else{				//  if(idx >= MAX_BUF)			
			idx = 0;							
			// reset PC_string;
			memset(PC_string, 0, sizeof(char) * MAX_BUF);
			// reset buffer
			memset(buffer, 0, sizeof(char) * MAX_BUF);	// reset buffer
			printf("ERROR : Too long string\r\n");
		}
	}
}