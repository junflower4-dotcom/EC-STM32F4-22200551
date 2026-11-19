//ecSTM32F4v2.h


/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 05-03-2021
Modified         : 10-01-2026 
Language/ver     : C++ in VS Code

Description      : STM32F411 Library for EC
/----------------------------------------------------------------*/


#ifndef __EC_STM_H
#define __EC_STM_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
// STM built-In Library
#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include "math.h"

// EC course Library
#include "ecPinNames.h"
#include "ecRCC2.h"
#include "ecGPIO2.h"
#include "ecEXTI2.h"
#include "ecSysTick2.h"
#include "ecTIM2.h"
#include "ecPWM2.h"
#include "ecStepper.h"
#include "ecICAP2.h"
#include "ecADC2.h"
#include "ecUART2.h"
#include "ecI2C2.h"
#include "ecDHT11.h"

// #define LED_PIN_NUCLEO 

void MCU_init(void){
    // CLOCK PLL 84MHz
    RCC_PLL_init();
    
    // SysTick 1msec
    SysTick_init();    

    // EXTI

    // TIMx Default Initialization
    // ...
    // PWM Default Initialization
    // ...        
    // USART Default Initialization
    // ...
    // Others
}

#endif



 