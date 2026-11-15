
/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 11-05-2026
Modified         : 11-10-2026
Language/ver     : C in VS Code / PlatformIO

Description      : LAB_TIMER_ICAP - Ultrasonic Distance Sensor (HC-SR04)
                   TIM3_CH1 (PA6): 100 ms  period, 10 us TRIG pulse
                   TIM4_CH1 (PB6): IC1 rising / IC2 falling ECHO capture
                   USART2: output measured pulse width and distance
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"

// Pin Definitions
#define TRIG_PIN    PA_6
#define ECHO_PIN    PB_6
#define LED_PIN     PA_5

// Timer Settings
#define ECHO_TICK_US       10U
#define ECHO_TIMEOUT_MS    30U
#define NO_ECHO_TIMEOUT_MS 200U
#define MAX_ECHO_TICKS     3000U

// Global Variables
volatile uint16_t rising_cnt  = 0;
volatile uint16_t falling_cnt = 0;
volatile uint16_t echo_ticks  = 0;
volatile uint8_t echo_ready   = 0;

// Timeout Variables
volatile uint8_t rising_detected = 0;
volatile uint32_t rising_ms = 0;


// Function Declaration
void setup(void);


// =================== SETUP ===================

void setup(void)
{
    // 1. MCU Initialization
    MCU_init();

    // 2. LED Initialization
    GPIO_init(LED_PIN, OUTPUT);

    // 3. USART2 Initialization
    USART2_init();

    // Disable unused RX interrupt
    USART2->CR1 &= ~USART_CR1_RXNEIE;
    NVIC_DisableIRQ(USART2_IRQn);

    // 4. TIM4 Input Capture Initialization
    // CH1: Rising Edge
    ICAP_init(ECHO_PIN, IC_RISE);

    // CH2: Falling Edge
    ICAP_ICn_init(ECHO_PIN, IC_FALL, IC_2);

    // TIM4 Counter Tick = 10 us
    ICAP_tick(ECHO_PIN, ECHO_TICK_US);

    // 5. TIM3 PWM Initialization
    PWM_init(TRIG_PIN, 100);

    // Stop TIM3
    TIM3->CR1 &= ~TIM_CR1_CEN;

    // Configure Timer
    // f_TIM = 84 MHz
    // f_CNT = 100 kHz
    TIM3->PSC  = 839;

    // PWM Period = 100 ms
    TIM3->ARR  = 9999;

    // HIGH Pulse Width = 10 us
    TIM3->CCR1 = 1;

    // Apply updated settings
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR  = ~TIM_SR_UIF;
    TIM3->CNT = 0;

    // Restart TIM3
    TIM3->CR1 |= TIM_CR1_CEN;
}


// =================== MAIN ===================

int main(void)
{
    setup();

    uint32_t last_valid_ms = msTicks;

    while(1)
    {
        uint8_t new_echo = 0;
        uint8_t pulse_timeout = 0;
        uint16_t ticks = 0;

        // Critical Section
        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        uint32_t now = msTicks;

        // 1. Check new ECHO measurement
        if(echo_ready)
        {
            ticks = echo_ticks;
            echo_ready = 0;
            new_echo = 1;
        }

        // 2. Check Falling Edge Timeout
        if(rising_detected &&
           (uint32_t)(now - rising_ms) >= ECHO_TIMEOUT_MS)
        {
            rising_detected = 0;
            pulse_timeout = 1;
        }

        // Restore interrupt state
        __set_PRIMASK(primask);

        // 3. Calculate Distance
        if(new_echo)
        {
            last_valid_ms = now;

            // Tick -> Time (us)
            uint32_t echo_us =
                (uint32_t)ticks * ECHO_TICK_US;

            // Time -> Distance (cm)
            float distance_cm = echo_us / 58.0f;

            // USART2 Output
            printf("Distance: %.2f cm\r\n", distance_cm);
        }

        // 4. Falling Edge Timeout Message
        if(pulse_timeout)
        {
            printf("Timeout: No Falling Edge\r\n");
        }

        // 5. No Valid ECHO Timeout
        if((uint32_t)(now - last_valid_ms)
           >= NO_ECHO_TIMEOUT_MS)
        {
            last_valid_ms = now;

            printf("Timeout: No valid ECHO\r\n");
        }
    }
}


// ============== TIM4 INTERRUPT ==============

void TIM4_IRQHandler(void)
{
    uint32_t flags = TIM4->SR;

    // 1. Timer Update Flag
    if(flags & TIM_SR_UIF)
    {
        TIM4->SR = ~TIM_SR_UIF;
    }

    // 2. Input Capture Overcapture
    if(flags & (TIM_SR_CC1OF | TIM_SR_CC2OF))
    {
        rising_detected = 0;

        // Clear overcapture and capture flags
        TIM4->SR = ~(TIM_SR_CC1OF | TIM_SR_CC2OF |
                     TIM_SR_CC1IF | TIM_SR_CC2IF);

        return;
    }

    // 3. Rising Edge (Channel 1)
    if(flags & TIM_SR_CC1IF)
    {
        rising_cnt = ICAP_read(ECHO_PIN, IC_1);
        clear_CCIF(TIM4, IC_1);

        // Rising Edge detected
        rising_detected = 1;

        // Save Rising Edge time
        rising_ms = msTicks;
    }

    // 4. Falling Edge (Channel 2)
    if(flags & TIM_SR_CC2IF)
    {
        falling_cnt = ICAP_read(ECHO_PIN, IC_2);
        clear_CCIF(TIM4, IC_2);

        if(rising_detected)
        {
            // Calculate HIGH duration
            // 16-bit wraparound is handled
            uint16_t diff =
                (uint16_t)(falling_cnt - rising_cnt);

            // Valid pulse width: 0 ~ 30 ms
            if(diff > 0 && diff <= MAX_ECHO_TICKS)
            {
                echo_ticks = diff;
                echo_ready = 1;
            }

            // Measurement finished
            rising_detected = 0;
        }
    }
}

