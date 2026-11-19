
/*----------------------------------------------------------------\
@ Embedded Controller - Smart Home Management System

Description : Smart Home Control using FSM
              HOME / AWAY / ALARM
              STM32 NUCLEO-F411RE

설명 : FSM 기반 스마트홈 관리 시스템
       HOME / AWAY / ALARM 상태 제어
       STM32 NUCLEO-F411RE 사용
/----------------------------------------------------------------*/

#include "ecSTM32F4v2.h"
#include "ecI2C2.h"
#include "ecDHT11.h"

#include <stdio.h>
#include <stdint.h>


// ================================================================
// 1. PIN DEFINITIONS
//    핀 정의
// ================================================================

// Ventilation Motor PWM
// 환풍기 모터 PWM 출력 핀
#define VENTILATION_PIN PB_7
// TIM4 CH2, Period = 1ms (1kHz)
// TIM4 채널 2, PWM 주기 = 1ms

// Window Servo Motor PWM
// 창문 개폐용 서보모터 PWM 출력 핀
#define SERVO_PIN PC_7
// TIM3 CH2, Period = 20ms (50Hz)
// TIM3 채널 2, PWM 주기 = 20ms

// RGB LED (Digital Output)
// RGB LED의 각 색상을 제어하는 디지털 출력 핀
#define RGB_RED   PB_13   // Red LED   / 빨간색
#define RGB_GREEN PB_14   // Green LED / 초록색
#define RGB_BLUE  PB_15   // Blue LED  / 파란색

// Analog Sensors
// 아날로그 센서: JADC를 통해 값 측정
#define LIGHT_SENSOR PA_0
// JADC1, JDR1: Light sensor
// 조도 센서: 현재 거실의 밝기를 측정

#define POTENTIO PC_1
// JADC1, JDR2: Threshold adjustment
// 가변저항: 어두움 판단 기준값을 조절

// Digital Sensors
// 디지털 센서: GPIO를 통해 신호 수신
#define DHT11_PIN PA_6
// Temperature/Humidity sensor
// 온도 및 습도를 측정하는 센서

#define PIR_PIN PB_3
// PIR Motion Sensor (EXTI3)
// 움직임 감지 센서, Rising Edge 인터럽트 사용

// User Input
// 사용자 입력 버튼
#define PIN_BUTTON PA_10
// HOME/AWAY Button (EXTI10)
// 버튼을 누르면 HOME / AWAY 전환


// ================================================================
// 2. THRESHOLD DEFINITIONS
//    센서 기준값 및 시간 상수 정의
// ================================================================

// Ventilation ON thresholds
// 환기를 시작하는 상한 기준값
#define TEMP_HIGH 28
// Temperature upper threshold (°C)
// 온도가 28°C를 초과하면 환기 시작 조건 만족

#define HUM_HIGH 70
// Humidity upper threshold (%RH)
// 습도가 70%를 초과하면 환기 시작 조건 만족

// Ventilation OFF thresholds (Hysteresis)
// 환기를 종료하는 하한 기준값
#define TEMP_LOW 26
// Temperature lower threshold (°C)
// 온도가 26°C 미만이어야 종료 조건 만족

#define HUM_LOW 60
// Humidity lower threshold (%RH)
// 습도가 60% 미만이어야 종료 조건 만족

// NOTE:
// These thresholds are example values.
// 실제 온습도 기준값은 테스트 후 조정할 수 있음.


// ------------------------------------------------
// Timer: TIM1 Update Interrupt = 250ms
// TIM1 인터럽트 1회 = 250ms = 1 Tick
// ------------------------------------------------

#define AWAY_DELAY_TICKS 20
// 5s / 250ms = 20 ticks
// AWAY 전환 후 5초 동안 퇴실 대기

#define ENTRANCE_LIGHT_TICKS 12
// 3s / 250ms = 12 ticks
// HOME에서 PIR 감지 시 노란색 LED를 3초 유지

#define AWAY_YELLOW_TICKS 12
// 3s / 250ms = 12 ticks
// AWAY 카운트다운 시작 후 3초 동안 노란색 LED 점등

#define ALARM_BLINK_TICKS 2
// 0.5s / 250ms = 2 ticks
// ALARM 상태에서 빨간색 LED를 0.5초마다 ON/OFF 전환

#define VENT_BLINK_TICKS 4
// 1s / 250ms = 4 ticks
// 환기 중 초록색 LED를 1초마다 ON/OFF 전환

#define DHT11_PERIOD_MS 2000
// Read DHT11 every 2 seconds
// DHT11 센서는 2초 간격으로 측정
// TIM1 Tick이 아닌 SysTick의 msTicks를 사용


// ================================================================
// 3. SYSTEM STATE DEFINITIONS
//    FSM 상태 정의
// ================================================================

typedef enum {

    MODE_HOME,
    // Normal home: lighting and ventilation enabled
    // 일반 생활 모드: 자동 조명 및 자동 환기 기능 활성화

    MODE_AWAY_EXIT,
    // 5s exit countdown before intrusion monitoring
    // 외출 대기 모드: 5초 카운트다운, 침입 감시 비활성화

    MODE_AWAY,
    // Away: intrusion monitoring enabled
    // 외출 모드: PIR 움직임을 침입으로 감지

    MODE_ALARM
    // Intrusion alarm active
    // 침입 경보 모드: 빨간색 LED 및 LCD 경고 표시

} SystemState;


// Current system state
// 현재 스마트홈 시스템의 상태
SystemState state = MODE_HOME;

// Only Change_State() should modify this variable.
// 상태 변경은 Change_State() 함수에서만 수행하도록 설계
// 초기 상태는 HOME


// ================================================================
// 4. GLOBAL VARIABLES
//    전역변수 및 FLAG 정의
// ================================================================


// ------------------------------------------------
// A. Interrupt Event Flags
//    인터럽트 이벤트 저장 변수
// ------------------------------------------------

volatile uint8_t buttonEvent = 0;
// 1: Button press detected by EXTI10
// 1이면 EXTI10에서 버튼 입력이 감지된 상태
//
// Used by Event_Process() to request HOME/AWAY switching.
// Event_Process()에서 HOME/AWAY 전환을 처리
//
// Cleared after the event is processed.
// 이벤트 처리 후 0으로 초기화


volatile uint8_t pirEvent = 0;
// 1: Motion detected by PIR (EXTI3)
// 1이면 PIR 센서에서 움직임이 감지된 상태
//
// HOME: Starts entrance lighting.
// HOME에서는 입구 노란색 LED 점등 이벤트로 처리
//
// AWAY: Triggers intrusion alarm.
// AWAY에서는 침입 경보 이벤트로 처리
//
// AWAY_EXIT: Motion is ignored.
// AWAY_EXIT에서는 움직임을 침입으로 판단하지 않음


volatile uint32_t controlTicks = 0;
// Incremented every 250ms by TIM1 Update Interrupt.
// TIM1 Update Interrupt가 발생할 때마다 1씩 증가
//
// 1 Tick  = 250ms
// 2 Ticks = 500ms
// 4 Ticks = 1s
// 8 Ticks = 2s
// 12 Ticks = 3s
// 20 Ticks = 5s
//
// Used for LED blinking, entrance lighting and away countdown.
// LED 점멸, 입구 조명 유지 시간, AWAY 카운트다운에 사용
//
// Updated only in TIM1_UP_TIM10_IRQHandler().
// 실제 값 증가는 TIM1 ISR에서 수행


// ------------------------------------------------
// B. DHT11 Sensor Variables
//    온도 및 습도 측정 관련 변수
// ------------------------------------------------

int temperature = 0;
// Last valid temperature reading from DHT11 (°C).
// 마지막으로 정상 측정한 온도값 저장
//
// Used by Ventilation_Control() and LCD_Update().
// 환기 판단 및 LCD 온도 표시에서 사용


int humidity = 0;
// Last valid relative humidity from DHT11 (%RH).
// 마지막으로 정상 측정한 상대습도값 저장
//
// Used by Ventilation_Control() and LCD_Update().
// 환기 판단 및 LCD 습도 표시에서 사용


uint8_t dhtValid = 0;
// 0: No successful DHT11 measurement yet.
// 0이면 아직 정상적으로 측정한 값이 없음
//
// 1: At least one valid measurement is available.
// 1이면 최소 한 번 정상 측정에 성공함
//
// Prevents ventilation control using uninitialized readings.
// 잘못된 초기 측정값으로 환풍기를 작동시키는 것을 방지


uint32_t dhtErrorCount = 0;
// Counts failed DHT11 transactions.
// DHT11 통신 실패 횟수를 기록
//
// Used for error reporting and project analysis.
// 센서 오류 보고 및 프로젝트 결과 분석에 사용


uint32_t lastDhtMs = 0;
// SysTick timestamp of the previous DHT11 read completion.
// 마지막 DHT11 읽기가 완료된 시점의 msTicks 저장
//
// Used to maintain an interval of at least 2 seconds.
// 다음 측정까지 최소 2초 간격을 유지하기 위해 사용


// ------------------------------------------------
// C. JADC Sensor Variables
//    조도 센서 및 Potentiometer 관련 변수
// ------------------------------------------------

volatile uint16_t lightADC = 0;
// Light sensor conversion result (ADC1->JDR1).
// 조도 센서의 JADC 변환 결과 저장
//
// PA0 -> ADC1 JDR1
// ADC_IRQHandler()에서 값을 갱신


volatile uint16_t potADC = 0;
// Potentiometer conversion result (ADC1->JDR2).
// Potentiometer의 JADC 변환 결과 저장
//
// PC1 -> ADC1 JDR2
// 밝기를 판단하는 기준값으로 사용


volatile uint8_t adcUpdated = 0;
// 1: New JADC sensor readings are available.
// 1이면 새로운 JADC 변환값이 저장되었다는 의미
//
// 0: No new readings since previous processing.
// 0이면 Main Loop에서 아직 새로운 값을 처리하지 않음


uint8_t roomDark = 0;
// 1: Living room is dark.
// 1이면 거실이 어두운 상태
//
// 0: Living room is bright.
// 0이면 거실이 밝은 상태
//
// Updated by JADC_Update() after comparing calibrated values.
// JADC_Update()에서 lightADC와 potADC를 비교하여 결정


// ------------------------------------------------
// D. Ventilation Variables
//    자동 환기 제어 관련 변수
// ------------------------------------------------

uint8_t ventOn = 0;
// 1: Ventilation is required.
// 1이면 환풍기 작동 및 창문 열기 필요
//
// 0: Ventilation is stopped.
// 0이면 환풍기 정지 및 창문 닫기
//
// Determined by Ventilation_Control() using hysteresis.
// 온습도 상한값과 하한값을 이용해 환기 여부 판단
//
// When ventOn = 1:
//   Fan ON, Servo OPEN, Green LED blinking.
// ventOn = 1이면 환풍기 ON, 창문 OPEN, Green LED 점멸
//
// When ventOn = 0:
//   Fan OFF, Servo CLOSED.
// ventOn = 0이면 환풍기 OFF, 창문 CLOSED


uint8_t lastVentOn = 0;
// Last ventilation state applied to PWM outputs.
// 실제 PWM에 마지막으로 적용한 환기 상태 저장
//
// Used to avoid repeatedly rewriting the same motor commands.
// ventOn과 비교해 상태가 바뀐 경우에만 PWM을 갱신


// ------------------------------------------------
// E. Entrance Lighting Variables
//    HOME 입구 조명 관련 변수
// ------------------------------------------------

uint8_t entranceLightOn = 0;
// 1: HOME entrance yellow lighting is active.
// 1이면 HOME 입구 Yellow LED 점등 시간 진행 중
//
// 0: Entrance yellow lighting is inactive.
// 0이면 입구 조명 이벤트가 종료된 상태


uint32_t entranceStartTick = 0;
// TIM1 tick when entrance motion was detected.
// PIR에서 움직임을 감지한 순간의 controlTicks 저장
//
// Used to keep Yellow LED active for 3 seconds.
// 현재 Tick과 비교해서 노란색 LED를 3초 유지


// ------------------------------------------------
// F. AWAY Mode Variables
//    AWAY 모드 카운트다운 관련 변수
// ------------------------------------------------

uint32_t awayStartTick = 0;
// TIM1 tick when MODE_AWAY_EXIT begins.
// AWAY_EXIT 상태에 진입한 순간의 controlTicks 저장
//
// Used to calculate 5-second exit countdown.
// 현재 Tick과의 차이를 이용해 5초 카운트다운 계산
//
// The first 3 seconds also use Yellow LED.
// 처음 3초 동안 Yellow LED 점등
//
// After 5 seconds, state changes to MODE_AWAY.
// 5초가 지나면 MODE_AWAY로 전환


// ------------------------------------------------
// G. LCD Display Variables
//    LCD 화면 관리 변수
// ------------------------------------------------

typedef enum {

    SCREEN_HOME,
    // HOME: Temperature and Humidity
    // HOME 상태에서 온도 및 습도 표시

    SCREEN_VENT,
    // HOME+VENT: Temperature, Humidity, VENT
    // 환기 중 HOME, 온습도, VENT 표시

    SCREEN_AWAY_EXIT,
    // AWAY: 5s countdown
    // AWAY 진입을 위한 5초 카운트다운 표시

    SCREEN_AWAY,
    // AWAY: Monitoring
    // 침입 감시 중인 AWAY 상태 표시

    SCREEN_ALARM,
    // ALARM: INTRUDER blinking
    // 침입 경보 및 INTRUDER 점멸 표시

    SCREEN_NOT_DRAWN
    // LCD needs initial display update
    // LCD 화면이 아직 초기 출력되지 않은 상태

} Screen;


Screen previousScreen = SCREEN_NOT_DRAWN;
// Last screen displayed on the LCD.
// 마지막으로 LCD에 표시한 화면 종류 저장
//
// Used to detect when the LCD layout should change.
// 이전 화면과 현재 화면을 비교하여 화면 변경 여부 판단


int previousTemp = -1;
// Temperature last displayed on the LCD.
// LCD에 마지막으로 출력한 온도값 저장
//
// Updated when temperature display is refreshed.
// 온도가 변경되면 LCD를 갱신하기 위해 비교


int previousHum = -1;
// Humidity last displayed on the LCD.
// LCD에 마지막으로 출력한 습도값 저장


int previousCountdown = -1;
// Countdown value last displayed on the LCD.
// LCD에 마지막으로 표시한 카운트다운 값 저장
//
// Used to update countdown only when the number changes.
// 카운트다운 숫자가 변경될 때만 LCD 갱신


// ------------------------------------------------
// H. Bluetooth Variables
//    Bluetooth 통신 관련 변수
// ------------------------------------------------

volatile uint8_t btRxFlag = 0;
// 1: A new Bluetooth command has been received.
// 1이면 USART6에서 새로운 Bluetooth 명령 수신
//
// Set by USART6_IRQHandler().
// USART6 수신 인터럽트에서 1로 설정


volatile char btRxData = 0;
// Latest Bluetooth command received via USART6.
// USART6에서 마지막으로 수신한 문자 저장
//
// Example: 'C' = Clear intrusion alarm.
// 예시: 'C'를 수신하면 침입 경보 해제
//
// A one-byte buffer is sufficient for an initial test,
// but a ring buffer is safer if commands arrive quickly.
// 우선 문자 하나를 저장하지만, 연속 수신을 고려하면
// 추후 Ring Buffer로 확장하는 것이 안전


uint8_t stateChanged = 0;
// 1: System state changed.
// 1이면 시스템 모드가 변경되었다는 의미
//
// Used to send Bluetooth status notifications.
// Bluetooth로 HOME/AWAY/ALARM 상태 변경 메시지 전송


uint8_t ventChanged = 0;
// 1: Ventilation ON/OFF state changed.
// 1이면 환풍기 작동 상태가 변경되었다는 의미
//
// Used to notify the laptop through Bluetooth.
// 환기 ON/OFF 상태 변경을 Bluetooth로 알리는 데 사용


// ================================================================
// 5. FUNCTION PROTOTYPES
//    함수 원형 선언
// ================================================================

// Initialization
// 초기화 함수
void setup(void);

// Sensor Processing
// 센서 데이터 처리 함수
void DHT11_Update(void);
void JADC_Update(void);

// Event / Communication Processing
// 버튼 이벤트 및 Bluetooth 명령 처리
void Event_Process(void);
void Bluetooth_Process(void);

// FSM
// 시스템 상태 판단 및 변경 함수
void FSM_Update(void);
void Change_State(SystemState next);

// Mode Tasks
// 각 모드에서 수행하는 작업
void HOME_Task(void);
void AWAY_Exit_Task(void);
void AWAY_Task(void);
void ALARM_Task(void);

// Ventilation Logic
// 온습도 조건에 따른 환기 판단
void Ventilation_Control(void);

// Output Control
// 모터, LED, LCD 및 Bluetooth 출력
void Motor_Control(void);
void LED_Control(void);
void LCD_Update(void);
void Bluetooth_Notify(void);


// ================================================================
// 6. MAIN FUNCTION
//    메인 함수
// ================================================================

int main(void)
{
    // Initialize hardware
    // 모든 하드웨어 초기화
    setup();

    // Start DHT11 measurement interval after initialization.
    // 초기화 완료 시점을 DHT11 측정 주기의 기준으로 설정
    lastDhtMs = msTicks;

    while (1) {

        // Step 1. Read and update sensor values
        // 1단계: 센서 측정값 확인 및 갱신
        DHT11_Update();
        JADC_Update();

        // Step 2. Process input events and Bluetooth commands
        // 2단계: 버튼 및 Bluetooth 입력 명령 처리
        Event_Process();
        Bluetooth_Process();

        // Step 3. Run the Finite State Machine
        // 3단계: 현재 상태에 맞는 FSM 함수 실행
        FSM_Update();

        // Step 4. Apply system output
        // 4단계: 실제 모터, LED, LCD 출력 제어
        Motor_Control();
        LED_Control();
        LCD_Update();

        // Step 5. Report state changes through Bluetooth
        // 5단계: 상태가 변경되면 Bluetooth로 메시지 전송
        Bluetooth_Notify();
    }
}


// ================================================================
// 7. INITIALIZATION
//    하드웨어 초기화 함수
// ================================================================

void setup(void)
{
    // System clock: 84MHz
    // 시스템 클록을 84MHz로 설정
    RCC_PLL_init();

    // SysTick: millisecond timing
    // SysTick 초기화: 1ms 단위 시간 관리
    SysTick_init();


    // GPIO Initialization
    // RGB LED, Button, PIR GPIO 초기화
    //
    // TODO:
    // RGB LED -> OUTPUT
    // Button  -> INPUT, Pull-up
    // PIR     -> INPUT


    // EXTI Initialization
    // 버튼 및 PIR 외부 인터럽트 초기화
    //
    // TODO:
    // PA10 -> EXTI10, Falling Edge
    // PB3  -> EXTI3, Rising Edge


    // PWM Initialization
    // 환풍기와 서보모터 PWM 초기화
    //
    // TODO:
    // TIM4 CH2 -> Fan PWM, 1000us
    // TIM3 CH2 -> Servo PWM, 20000us
    //
    // Initial output: Fan OFF, Window CLOSED
    // 초기 상태: 환풍기 정지, 창문 닫힘


    // DHT11 Initialization
    // DHT11 온습도 센서 초기화
    DHT11_init(DHT11_PIN);


    // JADC Initialization
    // 조도 센서 및 가변저항 JADC 초기화
    //
    // TODO:
    // ADC1 injected sequence
    // PA0 -> JDR1 (Light Sensor)
    // PC1 -> JDR2 (Potentiometer)
    //
    // Trigger: TIM5 TRGO, 125ms
    // TIM5의 125ms 주기 트리거로 자동 변환


    // Control Timer Initialization
    // 시스템 제어용 TIM1 초기화
    //
    // TODO:
    // TIM1 Update Interrupt, 250ms
    // 250ms마다 controlTicks 증가


    // I2C LCD Initialization
    // I2C 통신 및 LCD 초기화
    //
    // TODO:
    // I2C1 -> PB8(SCL), PB9(SDA)
    // LCD_init()


    // Serial Communication Initialization
    // USART 통신 초기화

    // USART2: PC Debug
    // USART2: PC Tera Term 디버깅
    USART2_init();

    // USART6: Bluetooth, 9600 baud
    // USART6: Bluetooth 통신, 9600bps
    //
    // TODO:
    // USART_init(USART6, PA_11, PA_12, BAUD_9600);
    // Enable USART6 RX interrupt
}


// ================================================================
// 8. FINITE STATE MACHINE
//    FSM 상태별 작업 실행
// ================================================================

void FSM_Update(void)
{
    switch (state) {

        case MODE_HOME:

            // Execute HOME mode functions
            // HOME 모드에서 필요한 기능 실행
            HOME_Task();
            break;

        case MODE_AWAY_EXIT:

            // Execute away countdown
            // 외출 전 5초 카운트다운 실행
            AWAY_Exit_Task();
            break;

        case MODE_AWAY:

            // Monitor intrusion
            // 침입 감시 수행
            AWAY_Task();
            break;

        case MODE_ALARM:

            // Maintain intrusion alarm
            // 침입 경보 유지
            ALARM_Task();
            break;

        default:

            // Recover to HOME if state is invalid
            // 잘못된 상태라면 HOME으로 복구
            Change_State(MODE_HOME);
            break;
    }
}


// ================================================================
// 9. STATE TRANSITION
//    시스템 상태 전환 함수
// ================================================================

void Change_State(SystemState next)
{
    // Do nothing if the state is unchanged.
    // 현재 상태와 다음 상태가 같다면 변경하지 않음
    if (state == next) {
        return;
    }

    // Update current state
    // 현재 시스템 상태 변경
    state = next;

    // Request Bluetooth notification
    // 상태 변경 메시지 전송 요청
    stateChanged = 1;

    // Disable ventilation and entrance lighting outside HOME.
    // HOME이 아닌 상태에서는 환기와 입구 조명 비활성화
    if (next != MODE_HOME) {

        if (ventOn) {
            ventOn = 0;
            ventChanged = 1;
        }

        entranceLightOn = 0;
    }

    // Record the start time of AWAY exit countdown.
    // AWAY_EXIT 진입 시점 저장
    if (next == MODE_AWAY_EXIT) {

        awayStartTick = controlTicks;

        // Ignore previous PIR events.
        // 이전 PIR 이벤트 제거
        pirEvent = 0;
    }

    // TODO:
    // Handle other state-entry actions.
    // HOME 복귀, ALARM 해제 등 추가 진입 동작 구현
}


// ================================================================
// 10. MODE TASK FUNCTIONS
//     모드별 기능 수행 함수
// ================================================================

void HOME_Task(void)
{
    // TODO:
    //
    // 1. Handle entrance PIR lighting for 3 seconds.
    //    PIR 감지 시 입구 Yellow LED를 3초 유지
    //
    // 2. Check darkness flag for white lighting.
    //    roomDark에 따라 White LED 표시 판단
    //
    // 3. Call Ventilation_Control().
    //    온습도에 따른 환기 ON/OFF 판단
    //
    // NOTE:
    // Actual RGB outputs are handled by LED_Control().
    // RGB LED의 실제 출력은 LED_Control()에서 수행

    Ventilation_Control();
}


void AWAY_Exit_Task(void)
{
    // Ignore PIR events during exit countdown.
    // 외출 대기 중 발생한 PIR 움직임은 무시
    pirEvent = 0;

    // Calculate elapsed ticks since exit countdown began.
    // 카운트다운 시작 이후 경과 Tick 계산
    uint32_t elapsed =
        (uint32_t)(controlTicks - awayStartTick);

    // 5-second exit countdown
    // 250ms × 20 = 5000ms = 5초
    if (elapsed >= AWAY_DELAY_TICKS) {

        // Start intrusion monitoring.
        // 5초 후 실제 AWAY 상태로 전환
        Change_State(MODE_AWAY);
    }

    // NOTE:
    // LCD countdown is handled by LCD_Update().
    // 카운트다운 화면은 LCD_Update()에서 출력
}


void AWAY_Task(void)
{
    // PIR detection raises intrusion alarm.
    // PIR 감지 시 침입 경보 상태로 전환
    if (pirEvent) {

        // Consume motion event.
        // PIR 이벤트 처리 후 초기화
        pirEvent = 0;

        // Enter alarm state.
        // ALARM 상태 진입
        Change_State(MODE_ALARM);
    }
}


void ALARM_Task(void)
{
    // Alarm remains active until Bluetooth acknowledgment.
    // Bluetooth 경보 해제 명령이 들어올 때까지 ALARM 유지

    // LED blinking and LCD updates occur separately.
    // 빨간색 LED 점멸 및 LCD 경고는 출력 함수에서 처리

    // TODO:
    // Additional alarm logic if required.
    // 필요 시 추가 경보 기능 구현
}


// ================================================================
// 11. SENSOR PROCESSING
//     센서 측정 및 데이터 처리
// ================================================================

void DHT11_Update(void)
{
    // TODO:
    //
    // 1. Check 2-second measurement interval.
    //    마지막 측정 이후 2초 경과 여부 확인
    //
    // 2. Call DHT11_read() in main context.
    //    Main Loop에서 DHT11_read() 호출
    //
    // 3. If DHT11_OK:
    //    통신 성공 시:
    //      Update temperature and humidity.
    //      온도 및 습도 갱신
    //
    //      Set dhtValid = 1.
    //      정상 측정 Flag 설정
    //
    // 4. If error:
    //    통신 실패 시:
    //      Increment dhtErrorCount.
    //      오류 횟수 증가
    //
    //      Keep last valid measurements.
    //      이전 정상 측정값 유지
    //
    // NOTE:
    // Do NOT call DHT11_read() inside an ISR.
    // DHT11_read()는 인터럽트 함수에서 호출하면 안 됨
}


void JADC_Update(void)
{
    // TODO:
    //
    // 1. Check adcUpdated.
    //    새로운 JADC 측정값이 있는지 확인
    //
    // 2. Compare lightADC and potADC.
    //    조도 센서값과 가변저항 기준값 비교
    //
    // 3. Update roomDark.
    //    어두움 여부를 0 또는 1로 설정
    //
    // 4. Clear adcUpdated.
    //    처리 후 새로운 측정값 Flag 초기화
    //
    // NOTE:
    // Comparison direction depends on light-sensor wiring.
    // 어두울수록 ADC 값이 커지는지 작아지는지
    // 실제 측정 후 비교 방향을 결정해야 함
}


// ================================================================
// 12. EVENT PROCESSING
//     입력 이벤트 처리
// ================================================================

void Event_Process(void)
{
    // TODO:
    //
    // 1. Process buttonEvent.
    //    버튼 인터럽트 이벤트 확인
    //
    // 2. Apply button debounce.
    //    버튼 채터링 방지 처리
    //
    // 3. Toggle HOME <-> AWAY_EXIT.
    //    버튼 입력으로 HOME과 AWAY 전환
    //
    // 4. Handle button behavior while ALARM is active.
    //    ALARM 도중 버튼 입력 처리 규칙 구현
    //
    // 5. PIR events are handled by the mode tasks.
    //    PIR 이벤트는 HOME/AWAY Task에서 처리
}


void Bluetooth_Process(void)
{
    // TODO:
    //
    // 1. Check btRxFlag.
    //    Bluetooth 수신 Flag 확인
    //
    // 2. Read command btRxData.
    //    수신된 명령 문자 확인
    //
    // 3. 'C' clears ALARM and returns to AWAY.
    //    'C' 명령을 받으면 경보 해제 후 AWAY 복귀
    //    다른 모드가 선택된 경우 복귀 규칙 반영 필요
    //
    // 4. Clear received-command flag.
    //    처리한 명령 Flag 초기화
}


// ================================================================
// 13. VENTILATION CONTROL
//     온습도 기반 자동 환기 판단
// ================================================================

void Ventilation_Control(void)
{
    // TODO:
    //
    // 1. Allow ventilation only in MODE_HOME.
    //    HOME에서만 자동 환기 허용
    //
    // 2. Check dhtValid.
    //    DHT11의 정상 측정값이 있는지 확인
    //
    // 3. If OFF:
    //    환기가 꺼진 상태라면:
    //      Start when TEMP > TEMP_HIGH OR HUM > HUM_HIGH.
    //      온도 또는 습도가 상한값 초과 시 환기 시작
    //
    // 4. If ON:
    //    환기가 켜진 상태라면:
    //      Stop when TEMP < TEMP_LOW AND HUM < HUM_LOW.
    //      온도와 습도가 모두 하한값 미만일 때 종료
    //
    // 5. Set ventChanged when ventOn changes.
    //    환기 상태가 변경되면 ventChanged = 1
    //
    // NOTE:
    // This function decides ventOn only.
    // 이 함수는 환기 여부만 결정
    //
    // Actual PWM control is handled by Motor_Control().
    // 실제 팬과 서보모터 PWM은 Motor_Control()에서 설정
}


// ================================================================
// 14. OUTPUT CONTROL
//     하드웨어 출력 제어
// ================================================================

void Motor_Control(void)
{
    // TODO:
    //
    // ventOn = 1 -> Fan ON, Servo OPEN
    // 환기 ON -> 환풍기 작동, 창문 열기
    //
    // ventOn = 0 -> Fan OFF, Servo CLOSED
    // 환기 OFF -> 환풍기 정지, 창문 닫기
    //
    // Apply PWM only when output changes.
    // 환기 상태가 변경되었을 때만 PWM 설정
    //
    // Compare ventOn with lastVentOn.
    // ventOn과 lastVentOn을 비교해서 출력 변경
    //
    // NOTE:
    // Initialize fan OFF and servo CLOSED in setup().
    // setup()에서 처음부터 환풍기 OFF, 창문 CLOSED 설정
}


void LED_Control(void)
{
    // TODO:
    //
    // Apply RGB LED priority.
    // RGB LED 우선순위를 판단하여 최종 색상 결정
    //
    // 1. ALARM
    //    Red blink every 0.5s
    //    빨간색 LED를 0.5초마다 점멸
    //
    // 2. AWAY_EXIT
    //    Yellow for first 3s
    //    외출 카운트다운 시작 후 3초 동안 노란색
    //
    // 3. HOME PIR
    //    Yellow for 3s
    //    입구 움직임 감지 시 노란색 3초 유지
    //
    // 4. HOME VENT
    //    Green blink every 1s
    //    환기 작동 중 초록색 1초 간격 점멸
    //
    // 5. HOME DARK
    //    White ON
    //    거실이 어두우면 흰색 LED 점등
    //
    // 6. Otherwise
    //    All LEDs OFF
    //    다른 조건이 없으면 모든 LED OFF
    //
    // NOTE:
    // Check RGB common-anode/common-cathode polarity.
    // RGB LED의 공통 애노드/캐소드 여부 확인 필요
}


void LCD_Update(void)
{
    // TODO:
    //
    // HOME
    //   Temperature and humidity
    //   온도 및 습도 표시
    //
    // HOME VENT
    //   Temperature, humidity, VENT
    //   온습도와 환기 작동 표시
    //
    // AWAY_EXIT
    //   5-second countdown
    //   5초 카운트다운 표시
    //
    // AWAY
    //   Away / monitoring status
    //   외출 및 침입 감시 상태 표시
    //
    // ALARM
    //   INTRUDER blink every 1s
    //   INTRUDER 경고 문구를 1초 간격으로 점멸
    //
    // Refresh only when displayed information changes.
    // 화면 상태나 표시 데이터가 바뀔 때만 LCD 갱신
    //
    // NOTE:
    // Updating only on screen changes is insufficient.
    // 화면 종류만 비교하면 온습도나 카운트다운이 갱신되지 않음
}


void Bluetooth_Notify(void)
{
    // TODO:
    //
    // Send messages only when relevant conditions change.
    // 상태나 조건이 변경되었을 때만 Bluetooth 메시지 전송
    //
    // Examples / 메시지 예시:
    //
    //   HOME MODE
    //   AWAY MODE
    //   INTRUSION DETECTED
    //   VENTILATION ON
    //   VENTILATION OFF
    //   DHT11 SENSOR ERROR
    //
    // Use USART6 for Bluetooth communication.
    // Bluetooth 출력에는 USART6 사용
    //
    // Clear notification flags after successful handling.
    // 전송 처리 후 stateChanged, ventChanged 등 초기화
}


// ================================================================
// 15. INTERRUPT HANDLERS
//     인터럽트 서비스 루틴
// ================================================================

// TODO:
// Implement with your EC library.
// EC 라이브러리에 맞춰 각 ISR 구현


// ------------------------------------------------
// EXTI15_10_IRQHandler
// ------------------------------------------------
// Button press -> buttonEvent = 1
// 버튼을 누르면 buttonEvent = 1
//
// Actual mode switching occurs in Event_Process().
// 실제 HOME/AWAY 전환은 Event_Process()에서 수행


// ------------------------------------------------
// EXTI3_IRQHandler
// ------------------------------------------------
// PIR rising edge -> pirEvent = 1
// PIR 신호가 LOW에서 HIGH로 변하면 pirEvent = 1
//
// HOME: Entrance lighting event
// HOME에서는 입구 조명 이벤트
//
// AWAY: Intrusion event
// AWAY에서는 침입 이벤트


// ------------------------------------------------
// TIM1_UP_TIM10_IRQHandler
// ------------------------------------------------
// controlTicks++ every 250ms
// 250ms마다 controlTicks를 1 증가
//
// Used for countdown and LED timing.
// 카운트다운과 LED 점멸 시간 계산에 사용


// ------------------------------------------------
// ADC_IRQHandler
// ------------------------------------------------
// Read ADC1->JDR1 into lightADC.
// JDR1의 조도 센서값을 lightADC에 저장
//
// Read ADC1->JDR2 into potADC.
// JDR2의 가변저항값을 potADC에 저장
//
// adcUpdated = 1
// 새로운 JADC 변환이 완료되었다는 Flag 설정


// ------------------------------------------------
// USART6_IRQHandler
// ------------------------------------------------
// Read received character.
// USART6로 수신된 문자 읽기
//
// Store in btRxData.
// 수신 문자를 btRxData에 저장
//
// Set btRxFlag = 1.
// Bluetooth 명령 수신 Flag 설정
