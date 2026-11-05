# LAB: Stepper Motor

**Date:** 2026-10-15

**Author/Partner:** 22200551 / Yejun Lee

**Demo Video:** [Youtube link](https://youtu.be/XZot2v1PkEA)

## Introduction

In this lab, we will learn how to drive a stepper motor using the digital GPIO outputs of an MCU. You will use a Finite State Machine (FSM) to design the algorithm for stepper motor control.

### Requirement

**Hardware**

* MCU
  * NUCLEO-F411RE
* Actuator/Sensor/Others:
  * **Lab:**
    * Stepper Motor 28BYJ-48 (5V)
    * Motor Driver ULN2003
  * breadboard

**Software**

* Visual Studio Code, PlatformIO, CMSIS, EC library



## Problem : Stepper Motor Control with FSM

### Problem Description

For the lab, we are going to use another type of stepper motor driver that requires a sequence of 4-input pulses as the input.

* **Driver:** **ULN2003 motor driver.**[ULN2003 spec sheet](https://www.electronicoscaldas.com/datasheet/ULN2003A-PCB.pdf)
* **Motor:** 28BYJ-48, Search for the spec sheet

  ![image1](./img/image1.jpg)

1. Find out the number of steps required to rotate 1 revolution of the stepper motor with Full-steppping.
2. Then, rotate the stepper motor 1 revolutions with 5 rpm.
3. Repeat the above process in the opposite direction.
4. Increase and decrease the speed of the motor as fast as it can rotate to find the maximum and minimum speed of the motor.
5. Apply the half-stepping and repeat the above.

### FSM design for Stepper Motor Sequence

#### State Table for Full-Stepping

**Full-stepping sequence**

![image2](./img/image2.jpg)


<table border="1" style="width: 100%; text-align: center;">
  <thead>
    <tr>
      <th rowspan="2" style="text-align: center;">Phase</th>
      <th rowspan="2" style="text-align: center;">Port_Pin</th>
      <th colspan="4" style="text-align: center;">Sequence</th>
    </tr>
    <tr>
      <th style="text-align: center;">1</th>
      <th style="text-align: center;">2</th>
      <th style="text-align: center;">3</th>
      <th style="text-align: center;">4</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>A</td>
      <td>PB_10</td>
      <td>H</td>
      <td>L</td>
      <td>L</td>
      <td>H</td>
    </tr>
    <tr>
      <td>B</td>
      <td>PB_4</td>
      <td>H</td>
      <td>H</td>
      <td>L</td>
      <td>L</td>
    </tr>
    <tr>
      <td>A'</td>
      <td>PB_5</td>
      <td>L</td>
      <td>H</td>
      <td>H</td>
      <td>L</td>
    </tr>
    <tr>
      <td>B'</td>
      <td>PB_3</td>
      <td>L</td>
      <td>L</td>
      <td>H</td>
      <td>H</td>
    </tr>
  </tbody>
</table>

<br>

**State Table**: Moore FSM

<table border="1" style="width: 100%; text-align: center; border-collapse: collapse;">
  <thead>
    <tr>
      <th style="text-align: center;">State</th>
      <th colspan="2" style="text-align: center;">Next State</th>
      <th colspan="1" style="text-align: center;">Output</th>
    </tr>
  </thead>
  <tbody>
	<tr>
      <td style="text-align: center;"></td>
      <td style="text-align: center;">DIR=0</td>
      <td style="text-align: center;">DIR=1</td>
      <td style="text-align: center;">(A B A' B')</td>
    </tr>
    <tr>
      <td>S0</td>
      <td>S1</td>
      <td>S3</td>
      <td>(H H L L)</td>
    </tr>
    <tr>
      <td>S1</td>
      <td>S2</td>
      <td>S0</td>
      <td>(L H H L)</td>
    </tr>
    <tr>
      <td>S2</td>
      <td>S3</td>
      <td>S1</td>
      <td>(L L H H)</td>
    </tr>
    <tr>
      <td>S3</td>
      <td>S0</td>
      <td>S2</td>
      <td>(H L L H)</td>
    </tr>
  </tbody>
</table>


#### State Table for Half-Stepping

**Half-stepping sequence**

![image3](./img/image3.jpg)

<table border="1" style="width: 100%; text-align: center; border-collapse: collapse;">
  <thead>
    <tr>
      <th rowspan="2" style="text-align: center;">Phase</th>
      <th rowspan="2" style="text-align: center;">Port_Pin</th>
      <th colspan="8" style="text-align: center;">Sequence</th>
    </tr>
    <tr>
      <th style="text-align: center;">1</th>
      <th style="text-align: center;">2</th>
      <th style="text-align: center;">3</th>
      <th style="text-align: center;">4</th>
      <th style="text-align: center;">5</th>
      <th style="text-align: center;">6</th>
      <th style="text-align: center;">7</th>
      <th style="text-align: center;">8</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>A</td>
      <td>PB_10</td>
      <td>H</td>
      <td>H</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>H</td>
    </tr>
    <tr>
      <td>B</td>
      <td>PB_4</td>
      <td>L</td>
      <td>H</td>
      <td>H</td>
      <td>H</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
    </tr>
    <tr>
      <td>A'</td>
      <td>PB_5</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>H</td>
      <td>H</td>
      <td>H</td>
      <td>L</td>
      <td>L</td>
    </tr>
    <tr>
      <td>B'</td>
      <td>PB_3</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>L</td>
      <td>H</td>
      <td>H</td>
      <td>H</td>
    </tr>
  </tbody>
</table>

<br>

* **State Table**: Moore FSM

<table border="1" style="width: 100%; text-align: center; border-collapse: collapse;">
  <thead>
    <tr>
      <th style="text-align: center;">State</th>
      <th colspan="2" style="text-align: center;">Next State</th>
      <th style="text-align: center;">Output</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="text-align: center;"></td>
      <td style="text-align: center;">DIR=0</td>
      <td style="text-align: center;">DIR=1</td>
      <td style="text-align: center;">(A B A' B')</td>
    </tr>
    <tr>
      <td>S0</td>
      <td>S1</td>
      <td>S7</td>
      <td>(H L L L)</td>
    </tr>
    <tr>
      <td>S1</td>
      <td>S2</td>
      <td>S0</td>
      <td>(H H L L)</td>
    </tr>
    <tr>
      <td>S2</td>
      <td>S3</td>
      <td>S1</td>
      <td>(L H L L)</td>
    </tr>
    <tr>
      <td>S3</td>
      <td>S4</td>
      <td>S2</td>
      <td>(L H H L)</td>
    </tr>
    <tr>
      <td>S4</td>
      <td>S5</td>
      <td>S3</td>
      <td>(L L H L)</td>
    </tr>
    <tr>
      <td>S5</td>
      <td>S6</td>
      <td>S4</td>
      <td>(L L H H)</td>
    </tr>
    <tr>
      <td>S6</td>
      <td>S7</td>
      <td>S5</td>
      <td>(L L L H)</td>
    </tr>
    <tr>
      <td>S7</td>
      <td>S0</td>
      <td>S6</td>
      <td>(H L L H)</td>
    </tr>
  </tbody>
</table>

### Programming FSM

**Preparation**

* Download files:
  * [ecStepper\_student.h, ecStepper\_student.c](https://github.com/ykkimhgu/EC-student/blob/main/include/lib-student/)
* Change the library files as `ecStepper.h, ecStepper.c`
* Write your name and modified date in the comment section
* must update your header files located in the directory `include\`.
* Create a new project under the directory `EC\lab\`
  * Environment: “**LAB\_Stepper\_Motor”**
  * Source File: “**LAB\_Stepper\_Motor_22200551.c”**
* modify the `platformio.ini` **,** to add new environment.

### Connection Diagram

Read the specification sheet of the motor and the motor driver for wiring and min/max input voltage/current.

![image4](./img/image4.jpg)

![image5](./img/image5.jpg)

### Configuration

<table border="1" style="width:100%; border-collapse:collapse;">
  <thead>
    <tr>
      <th>Function</th>
      <th>Pins</th>
      <th>Configuration</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>A, B, A', B'</td>
      <td>PB10, PB4, PB5, PB3</td>
      <td>DOUT, FAST</td>
    </tr>
  </tbody>
</table>


### Argorithm

#### Overview
  * FlowChart
![FlowChart](./img/Flowchart.jpg)


#### Description with Code
**1. `ecStepper.c`**
  *  The Full-stepping and Half-stepping sequences are implemented using Moore FSM state tables. Each state defines the next state for CW and CCW rotation and the four GPIO output signals. Full-stepping uses four states, while Half-stepping uses eight states.
```c
// Full-stepping Moore FSM
static const StateStepper_t fsm_Full[4] = {
    {{S1,S3},{HIGH,HIGH,LOW,LOW}},
    {{S2,S0},{LOW,HIGH,HIGH,LOW}},
    {{S3,S1},{LOW,LOW,HIGH,HIGH}},
    {{S0,S2},{HIGH,LOW,LOW,HIGH}}
};

// Half-stepping Moore FSM
static const StateStepper_t fsm_Half[8] = {
    {{S1,S7},{HIGH,LOW,LOW,LOW}},
    {{S2,S0},{HIGH,HIGH,LOW,LOW}},
    {{S3,S1},{LOW,HIGH,LOW,LOW}},
    {{S4,S2},{LOW,HIGH,HIGH,LOW}},
    {{S5,S3},{LOW,LOW,HIGH,LOW}},
    {{S6,S4},{LOW,LOW,HIGH,HIGH}},
    {{S7,S5},{LOW,LOW,LOW,HIGH}},
    {{S0,S6},{HIGH,LOW,LOW,HIGH}}
};
```
<br>

  * The `stepper_init()` function selects the FSM according to the stepping mode, initializes the current state to S0, and configures the four motor control pins as digital outputs with fast output speed. All GPIO outputs are initially set to LOW.
  
```c
// Initialization of Stepper Motor
void stepper_init(uint32_t mode, PinName_t *pinStepper){
	if (mode == HALF) 
		fsm = fsm_Half;
	else
		fsm = fsm_Full;
	

	state = S0;

	//  GPIO Digital Out Initiation
	// For A, B, AN, BN
	for (int i=0; i<4; i++){
		stepperPins[i] = pinStepper[i];

		GPIO_init(stepperPins[i], OUTPUT);
		GPIO_ospeed(stepperPins[i], FAST_SPEED);
		GPIO_write(stepperPins[i], LOW);
	}
}
```
<br>

  * The `stepper_speed()` function calculates the delay between steps based on the desired RPM and the number of steps per revolution. It uses different step counts for Full-stepping and Half-stepping to control the motor speed.

```c
// Converts Motor [rpm] to step delay in [msec]
void stepper_speed (uint32_t speedRPM){      // rpm
		
	if(speedRPM == 0)
		return;
	
	if(fsm == fsm_Half)
		_step_delay = 60*1000/(HALF_STEP_PER_REV*speedRPM); 		//in [msec]	
	
	else
		_step_delay = 60*1000/(FULL_STEP_PER_REV*speedRPM);
}
```
<br>

  * The `stepper_step()` function rotates the motor for the specified number of steps in the selected direction. It updates the FSM state, generates the corresponding GPIO outputs, and applies the calculated delay between steps.

```c
// Run Stepper Motor for given steps and direction
void stepper_step(uint32_t steps, uint32_t direction){
	// Exit if stepper_init() not called 
	if (fsm == 0) return;
	_steps = steps;

	// run for step size: 
	for (uint32_t i = 0; i < _steps; i++){
		// Update Present State
		state = fsm[state].next[direction];
		
		// Output of Present State
		stepper_out(state);
	
		// Delay for steppermotor speed
		delay_ms(_step_delay);
	}
}
```

<br>

  * The `stepper_out()` function applies the four output signals defined by the current FSM state to the corresponding GPIO pins. These signals control the motor coils through the ULN2003 driver.

```c
// Stepper Motor Output for given state
void stepper_out(uint32_t state){
	
	// stepperPins[0]=A, stepperPins[1]=B, ...
	// out[0]=A, out[1]=B, ...

	for(int i = 0; i < 4; i++){
		GPIO_write(stepperPins[i], fsm[state].out[i]);
	}
}
```

<br>

  * The `stepper_stop()` function stops the current stepping operation by setting the remaining step limit to zero. It also sets all four GPIO outputs to LOW to deactivate the motor coils.

```c
// Stop Stepper Motor
void stepper_stop(void) {
    _steps = 0;    
    
	for(int i = 0; i < 4; i++){
		GPIO_write(stepperPins[i], LOW);
	}
}
```
<br>

**2. `LAB_Stepper_Motor_22200551.c`**
  * The stepping mode, rotation direction, and motor speed are defined using macros. The STEPPER_MODE macro selects Full-stepping or Half-stepping, while STEP_PER_REV automatically determines the number of steps per revolution. The STEPPER_DIR and RPM macros specify the motor's rotation direction and speed.
  
```c
// STEPPER_MODE select
#define STEPPER_MODE 	FULL
// #define STEPPER_MODE	 HALF 
#define STEP_PER_REV \
		 ((uint32_t)((STEPPER_MODE == FULL) ? FULL_STEP_PER_REV : \
					 (STEPPER_MODE == HALF) ? HALF_STEP_PER_REV : \
					 0U))

// STEPPER_direction DIR_CCW / DIR CW
#define STEPPER_DIR		DIR_CCW

#define BUTTON_PIN PC_13 
#define RPM	(int)(5)
```

<br>

  * The pinStepper array defines the four GPIO pins connected to the ULN2003 motor driver. These pins correspond to phases A, B, A', and B', respectively.

```c
// ( A, B,  AN,  BN)
PinName_t pinStepper[4]    = {PB_10,PB_4,PB_5,PB_3};
```

<br>

  * The setup() function initializes the MCU and configures the USER button as a digital input with a falling-edge external interrupt. It also initializes the stepper motor according to the selected stepping mode and sets the motor speed to the specified RPM.

```c
// Initialiization 
void setup(void){	

    MCU_init();
	
    GPIO_init(BUTTON_PIN, INPUT);           		// GPIOC pin13 initialization
    EXTI_init(BUTTON_PIN, FALL,0);           		// External Interrupt Setting

	stepper_init(STEPPER_MODE, pinStepper); 				// Stepper GPIO pin initialization
	stepper_speed(RPM);                          		//  set stepper motor speed [RPM]
}
```

<br>

  * The main() function calculates the required number of steps based on the selected stepping mode and the desired number of revolutions. The motor rotates in the specified direction for the calculated number of steps, waits for two seconds, and repeats the operation continuously.

```c
int main(void) { 
	// Initialiization 
	setup();
	uint32_t rev=1;     // Revoultion
	uint32_t steps=STEP_PER_REV*rev;
	

	// Inifinite Loop 
	while(1){
		stepper_step(steps, STEPPER_DIR);  
		delay_ms(2000);
	}
}
```

<br>

  * The EXTI15_10_IRQHandler() function handles the external interrupt generated by the USER button. When the button is pressed, stepper_stop() stops the current stepping operation and sets all motor control outputs to LOW. The interrupt pending flag is then cleared.

```c
void EXTI15_10_IRQHandler(void) {  
	if (is_pending_EXTI(BUTTON_PIN)) {
		stepper_stop();
		clear_pending_EXTI(BUTTON_PIN); // cleared by writing '1'
	}
}
```

### Results


1. Full-stepping Mode
   > In Full-stepping mode, 2048 steps were used for approximately one revolution of the motor's output shaft. The motor was tested in both clockwise (CW) and counterclockwise (CCW) directions at a target speed of 5 RPM. The motor was also tested at 1 RPM as the minimum speed setting and 14 RPM as the maximum speed setting used in the experiment. Both CW and CCW operations were demonstrated.

2. Half-stepping Mode
   > In Half-stepping mode, 4096 steps were used for approximately one revolution of the motor's output shaft, which is twice the step count of Full-stepping. The motor was tested in both CW and CCW directions at a target speed of 5 RPM. The same speed tests were performed at 1 RPM and 14 RPM to observe motor operation in Half-stepping mode. 

Add [[demo video link]](https://youtu.be/XZot2v1PkEA)


### Discussion

1. Find out the trapezoid-shape velocity profile for a stepper motor. When is this profile necessary?
   > A trapezoidal velocity profile consists of three stages: acceleration, constant speed, and deceleration. The motor gradually increases its speed, maintains the target speed, and then gradually slows down before stopping.

   >This profile is necessary when a stepper motor operates at high speeds or moves a load with significant inertia. If the motor starts or stops suddenly, it may lose steps or stall because it cannot immediately follow the commanded speed. A trapezoidal velocity profile reduces these problems by providing smooth acceleration and deceleration.

2. How would you change the code more efficiently for micro-stepping control? You don’t have to code this but need to explain your strategy.
   >To implement micro-stepping more efficiently, I would use a dedicated micro-stepping driver, such as the A4988 or DRV8834, with a compatible bipolar stepper motor instead of the ULN2003.

   >In the current code, the MCU directly controls four GPIO outputs using a Moore FSM. However, a dedicated micro-stepping driver controls the winding currents internally, allowing the MCU to control the motor using only STEP and DIR signals.

   >Therefore, I would replace the existing FSM-based GPIO control with STEP pulse generation and direction control. I would also use a hardware timer to generate accurate pulses and adjust the number of steps per revolution according to the selected micro-stepping resolution. This would simplify the code and allow smoother motor operation.

3. There are other types of Stepper Motor Drivers that are simple to use, such as you only give one pulse signal and direction, instead of giving 4 pulse signals. Such examples are A4988, DRV 8834, and TB6600 drivers. Compare these motor drivers with ULN2003 in terms of operating method.
    > The ULN2003 controls a stepper motor using four GPIO signals from the MCU. The MCU generates the Full-stepping or Half-stepping sequence using a Moore FSM, and the ULN2003 switches the motor winding currents according to these signals.

    >In contrast, the A4988, DRV8834, and TB6600 use a STEP/DIR-type control interface. The MCU generates STEP pulses and sets the rotation direction, while the driver internally manages the winding excitation sequence and current regulation. These drivers also support micro-stepping, allowing finer motor movement.

    > ULN2003: Uses four GPIO signals, and the MCU must directly generate the stepping sequence.

    > A4988: Uses STEP and DIR signals and supports micro-stepping up to 1/16 step.

    > DRV8834: Uses STEP and DIR signals and supports micro-stepping up to 1/32 step.

    > TB6600: Uses pulse and direction signals and supports micro-stepping up to 1/16 step, based on the TB6600HG chip.

    > Therefore, the ULN2003 requires the MCU to directly manage the stepping sequence, whereas the other drivers handle this process internally. Using dedicated stepper motor drivers can simplify MCU programming and enable more precise current control and smoother motor operation.



### Reference

- ULN2003 motor driver.**[ULN2003 spec sheet](https://www.electronicoscaldas.com/datasheet/ULN2003A-PCB.pdf)
- HGU, Embedded Controller Course GitBook: [LAB: Stepper Motor | EC](https://ykkim.gitbook.io/ec/ec-course/lab/lab-stepper-motor#ecstepper.c)

### Troubleshooting

> Initially, the stepping mode and the number of steps per revolution had to be changed separately. To simplify mode selection and prevent inconsistent settings, the STEPPER_MODE macro was introduced. The selected mode is passed to stepper_init(), while STEP_PER_REV automatically determines the required step count for Full-stepping or Half-stepping.