/*
 * control_motors.h
 *
 *  Created on: Feb 21, 2021
 *      Author: jgamm
 */

#ifndef APP_HEADERS_CONTROL_MOTORS_H_
#define APP_HEADERS_CONTROL_MOTORS_H_

#include <stdint.h>
#include <stdbool.h>

#include "hal.h"

#define CM_STEP_FREQ  (65536U)//(32768U)
#define CM_STEP_ONPCT (50U)
#define CM_PIN_IDLE   (true)

typedef enum
{
    theta,
    phi
} CM_Motor_Enum;

typedef enum
{
    clockwise = true,
    counterclockwise = false
} CM_Dir_Enum;

void CM_init(void);

void CM_turnMotorSteps(CM_Motor_Enum motor, uint32_t num_steps, CM_Dir_Enum dir, void (*handler)(void *), void * handler_args);

void CM_align(CM_Motor_Enum motor, void (*handler)(void *), void * handler_args);

#endif /* APP_HEADERS_CONTROL_MOTORS_H_ */
