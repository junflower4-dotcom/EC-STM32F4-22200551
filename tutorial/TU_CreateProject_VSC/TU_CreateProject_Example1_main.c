#include "stm32f4xx.h"

#define LED_PIN 5

int main(void) {
    RCC->CR |= RCC_CR_HSION;

    while ((RCC->CR & RCC_CR_HSIRDY) == 0) {
    } 

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;

    while ((RCC->CFGR & RCC_CFGR_SWS) != 0) {
    }

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER &= ~(3UL << (2 * LED_PIN));
    GPIOA->MODER |=  (1UL << (2 * LED_PIN));

    while (1) {
        GPIOA->ODR |= (1UL << LED_PIN);
    }
}