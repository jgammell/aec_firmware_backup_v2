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

#define CM_PHI_STEP_TID   (timerA1)
#define CM_PHI_STEP_TOP   (1U)
#define CM_PHI_STEP_PORT  (P2)
#define CM_PHI_STEP_PIN   (0U)
#define CM_PHI_SD_PORT    (P1)
#define CM_PHI_SD_PIN     (IO_PIN5)
#define CM_PHI_RESET_PORT (P1)
#define CM_PHI_RESET_PIN  (IO_PIN6)
#define CM_PHI_DIR_PORT   (P1)
#define CM_PHI_DIR_PIN    (IO_PIN7)
#define CM_PHI_FAULT_PORT (P2)
#define CM_PHI_FAULT_PIN  (IO_PIN1)
#define CM_PHI_ES_PORT    (P2)
#define CM_PHI_ES_PIN     (IO_PIN6)

#define CM_THETA_STEP_TID   (timerA0)
#define CM_THETA_STEP_TOP   (3U)
#define CM_THETA_STEP_PORT  (P1)
#define CM_THETA_STEP_PIN   (IO_PIN4)
#define CM_THETA_SD_PORT    (P8)
#define CM_THETA_SD_PIN     (IO_PIN0)
#define CM_THETA_RESET_PORT (P8)
#define CM_THETA_RESET_PIN  (IO_PIN1)
#define CM_THETA_DIR_PORT   (P8)
#define CM_THETA_DIR_PIN    (IO_PIN2)
#define CM_THETA_FAULT_PORT (P1)
#define CM_THETA_FAULT_PIN  (IO_PIN0)
#define CM_THETA_ES_PORT    (P1)
#define CM_THETA_ES_PIN     (IO_PIN3)

#define CM_STEP_FREQ  (32768U)
#define CM_STEP_ONPCT (50U)
#define CM_PIN_IDLE   (true)

typedef enum
{
    theta,
    phi
} CM_Motor_Enum;

typedef enum
{
    clockwise = false,
    counterclockwise = true
} CM_Dir_Enum;

void CM_init(void);

void CM_turnMotorSteps(CM_Motor_Enum motor, uint32_t num_steps, CM_Dir_Enum dir, void (*handler)(void));

void CM_align(CM_Motor_Enum motor, void (*handler)(void));

#endif /* APP_HEADERS_CONTROL_MOTORS_H_ */
