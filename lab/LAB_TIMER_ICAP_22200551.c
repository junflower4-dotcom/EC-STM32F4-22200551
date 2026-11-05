
/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 05-11-2026
Modified         : 05-11-2026
Language/ver     : C in VS Code / PlatformIO

Description      : LAB_TIMER_ICAP - Ultrasonic Distance Sensor (HC-SR04)
                   TIM3_CH1 (PA6): 50 ms period, 10 us TRIG pulse
                   TIM4_CH1 (PB6): IC1 rising / IC2 falling ECHO capture
                   USART2: output measured pulse width and distance
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"
#include <stdint.h>
#include <stdio.h> 

#define TRIG_PIN          PA_6
#define ECHO_PIN          PB_6
#define LED_PIN           PA_5

#define PWM_PERIOD_MS     50U
#define ECHO_TICK_US      10U
#define MAX_ECHO_US       25000U

volatile uint32_t echo_ticks = 0U;
volatile uint8_t  measurement_ready = 0U;

static volatile uint32_t rising_capture = 0U;
static volatile uint8_t  rising_seen = 0U;
static uint32_t timer_period_ticks = 65536U;

void setup(void)
{
    // 1. System clock, LED and UART
    RCC_PLL_init();
    SysTick_init();
    GPIO_init(LED_PIN, OUTPUT);
    USART_init(USART2, PA_2, PA_3, 9600);

    // 2. TRIG: 50 ms period with 10 us HIGH pulse
    // Duty = 10 / 50000 = 0.0002 (0.02%)
    PWM_init(TRIG_PIN, PWM_PERIOD_MS);
    PWM_duty(TRIG_PIN, 0.0002f);

    // 3. ECHO: Rising and Falling edge capture
    ICAP_init(ECHO_PIN, IC_RISE);
    ICAP_ICn_init(ECHO_PIN, IC_FALL, IC_2);
    ICAP_tick(ECHO_PIN, ECHO_TICK_US);

    timer_period_ticks = TIM4->ARR + 1U;
}

int main(void)
{
    uint32_t ticks_snapshot;
    uint32_t pulse_us;
    uint32_t distance_centi_cm;
    uint8_t has_measurement;

    setup();

    printf("\r\n=== LAB_TIMER_ICAP: HC-SR04 ===\r\n");
    printf("TRIG: PA6 (D12), ECHO: PB6 (D10)\r\n");
    printf("PWM period: 50 ms, TRIG pulse: 10 us\r\n");

    while (1)
    {
        // Read ISR-updated variables together
        __disable_irq();
        has_measurement = measurement_ready;
        ticks_snapshot = echo_ticks;
        measurement_ready = 0U;
        __enable_irq();

        if (has_measurement)
        {
            pulse_us = ticks_snapshot * ECHO_TICK_US;

            // Distance[cm] ~= Echo time[us] / 58
            // Calculate in 0.01 cm units
            distance_centi_cm = (pulse_us * 100U + 29U) / 58U;

            printf("Echo: %lu us | Distance: %lu.%02lu cm\r\n",
                   (unsigned long)pulse_us,
                   (unsigned long)(distance_centi_cm / 100U),
                   (unsigned long)(distance_centi_cm % 100U));
        }
        else
        {
            printf("No valid echo detected\r\n");
        }

        // Toggle built-in LED
        GPIOA->ODR ^= (1U << 5);
        delay_ms(200);
    }
}

void TIM4_IRQHandler(void)
{
    uint32_t falling_capture;
    uint32_t width_ticks;

    // 1. Timer overflow flag
    if (is_UIF(TIM4))
    {
        clear_UIF(TIM4);
    }

    // 2. Rising edge: Start measuring
    if (is_CCIF(TIM4, IC_1))
    {
        rising_capture = ICAP_read(ECHO_PIN, IC_1);
        rising_seen = 1U;

        clear_CCIF(TIM4, IC_1);
    }

    // 3. Falling edge: Finish measuring
    if (is_CCIF(TIM4, IC_2))
    {
        falling_capture = ICAP_read(ECHO_PIN, IC_2);
        clear_CCIF(TIM4, IC_2);

        if (rising_seen)
        {
            // Correct one timer counter wrap
            if (falling_capture >= rising_capture)
            {
                width_ticks = falling_capture - rising_capture;
            }
            else
            {
                width_ticks = timer_period_ticks - rising_capture
                              + falling_capture;
            }

            // Ignore invalid pulse widths
            if ((width_ticks > 0U) &&
                ((width_ticks * ECHO_TICK_US) <= MAX_ECHO_US))
            {
                echo_ticks = width_ticks;
                measurement_ready = 1U;
            }
        }

        rising_seen = 0U;
    }
}
