/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 05-03-2021
Modified         : 16-09-2026 
Language/ver     : C++ in VS Code

Description      : Controls LED_0 using a Photodetector connected to PA_0
/----------------------------------------------------------------*/

#include "ecRCC2.h"
#include "ecGPIO2.h"

#define LED_PIN PB_12

#define BUTTON_PIN PA_0

// Initialiization 
void setup(void) {
	RCC_HSI_init(); 

    // Photodetector
    GPIO_init(BUTTON_PIN, INPUT);
    GPIO_pupd(BUTTON_PIN, PU);

    // LED0
    GPIO_init(LED_PIN, OUTPUT);
    GPIO_pupd(LED_PIN, PU);
    GPIO_otype(LED_PIN, OPEN_DRAIN);
    GPIO_ospeed(LED_PIN, MEDIUM_SPEED);
}
	
int main(void) { 
 	setup();
	int buttonState=0;
	
	while(1){
		// check if the pushbutton is pressed. Turn LED on/off accordingly:
		buttonState = GPIO_read(BUTTON_PIN);
		
		// Bright
		if(buttonState)	
			GPIO_write(LED_PIN, LOW);
		
		// Dark
		else 
			GPIO_write(LED_PIN, HIGH);

		delay_ms_HSI(100);             					// delay 100 ms
	}
}