/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Yejun Lee]
Created          : 05-03-2021
Modified         : 12-10-2026
Language/ver     : C++ in VS Code

Description      : Stepper motor control library using GPIO outputs and Moore FSM for full-stepping and half-stepping.
                   Supports motor direction, speed, and stop control.
/----------------------------------------------------------------*/

#include "stm32f411xe.h"
#include "ecPinNames.h"
#include "ecRCC2.h"
#include "ecGPIO2.h"
#include "ecSysTick2.h"
			
#ifndef __EC_STEPPER_H
#define __EC_STEPPER_H

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

// Stepper Mode
#define HALF (int)(0)
#define FULL (int)(1)	 
#define DIR_CW (int)(0)
#define DIR_CCW (int)(1)	  

// Stepper Motor Spec
#define FULL_STEP_PER_REV   (64*32)	           // 2048 step -> 1 revolution
#define HALF_STEP_PER_REV   (FULL_STEP_PER_REV*2)   // 4096 step -> 1 revolution

//State number 
typedef enum {
	S0, S1, S2, S3, S4, S5, S6, S7
} StateNum;

// FULL/HALF stepping sequence  - Moore FSM
typedef struct {	
  uint32_t next[2];
  uint8_t out[4];
} StateStepper_t;

// Initialization of Stepper Motor
void stepper_init(uint32_t mode, PinName_t *pinStepper);

// Converts Motor [rpm] to step delay in [msec]
void stepper_speed (uint32_t speedRPM);

// Run Stepper Motor for given steps and direction
void stepper_step(uint32_t steps, uint32_t direction);

// Stepper Motor Output for given state
void stepper_out(uint32_t state);

// Stop Stepper Motor
void stepper_stop(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __EC_STEPPER_H
