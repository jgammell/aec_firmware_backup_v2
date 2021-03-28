/*
 * control_motors.c
 *
 *  Created on: Feb 21, 2021
 *      Author: jgamm
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pwm.h"
#include "io_hal.h"
#include "error.h"
#include "control_motors.h"
#include "io_interrupts.h"
#include "flashctl.h"
#include "hal.h"

#define ORIENTATION_INFO_BASE ((uint8_t *) 0x1800)

typedef struct
{
    const PWM_Sources_Enum      timer_source;
    const uint8_t               timer_output;
    IO_Registers_Struct * const step_port;
    const uint8_t               step_pin;
    IO_Registers_Struct * const sd_port;
    const uint8_t               sd_pin;
    IO_Registers_Struct * const reset_port;
    const uint8_t               reset_pin;
    IO_Registers_Struct * const dir_port;
    const uint8_t               dir_pin;
    IO_Registers_Struct * const fault_port;
    const uint8_t               fault_pin;
    IO_Registers_Struct * const es_port;
    const uint8_t               es_pin;
    uint32_t                    freq;
} CM_Motor_Struct;
typedef struct
{
    uint32_t    num_steps;
    CM_Dir_Enum dir;
    void      (*handler)(void *);
    void * handler_args;
} CM_TurnStepsCmd_Struct;
typedef struct
{
    CM_Dir_Enum dir;
    void (*handler)(void *);
    void * handler_args;
} CM_AlignCmd_Struct;
typedef struct
{
    const uint8_t mem_valid;
    int32_t theta_current;
    int32_t phi_current;
    int32_t theta_aligned;
    int32_t phi_aligned;
    bool current_valid;
    bool aligned_valid;
} CM_OrientationInfo_Struct;

static CM_OrientationInfo_Struct  orientation_info =
{
 .mem_valid = 0,
 .theta_current = 0,
 .phi_current = 0,
 .theta_aligned = 0,
 .phi_aligned = 0,
 .current_valid = false,
 .aligned_valid = false
};
static CM_Motor_Struct Theta =
{
 .timer_source = CM_THETA_STEP_TID,
 .timer_output = CM_THETA_STEP_TOP,
 .step_port    = CM_THETA_STEP_PORT,
 .step_pin     = CM_THETA_STEP_PIN,
 .sd_port      = CM_THETA_SD_PORT,
 .sd_pin       = CM_THETA_SD_PIN,
 .reset_port   = CM_THETA_RESET_PORT,
 .reset_pin    = CM_THETA_RESET_PIN,
 .dir_port     = CM_THETA_DIR_PORT,
 .dir_pin      = CM_THETA_DIR_PIN,
 .fault_port   = CM_THETA_FAULT_PORT,
 .fault_pin    = CM_THETA_FAULT_PIN,
 .es_port      = CM_THETA_ES_PORT,
 .es_pin       = CM_THETA_ES_PIN,
 .freq         = CM_STEP_FREQ
};
static CM_Motor_Struct Phi =
{
 .timer_source = CM_PHI_STEP_TID,
 .timer_output = CM_PHI_STEP_TOP,
 .step_port    = CM_PHI_STEP_PORT,
 .step_pin     = CM_PHI_STEP_PIN,
 .sd_port      = CM_PHI_SD_PORT,
 .sd_pin       = CM_PHI_SD_PIN,
 .reset_port   = CM_PHI_RESET_PORT,
 .reset_pin    = CM_PHI_RESET_PIN,
 .dir_port     = CM_PHI_DIR_PORT,
 .dir_pin      = CM_PHI_DIR_PIN,
 .fault_port   = CM_PHI_FAULT_PORT,
 .fault_pin    = CM_PHI_FAULT_PIN,
 .es_port      = CM_PHI_ES_PORT,
 .es_pin       = CM_PHI_ES_PIN,
 .freq         = CM_STEP_FREQ
};

static SemaphoreHandle_t theta_ownership = NULL;
static SemaphoreHandle_t phi_ownership   = NULL;

static QueueHandle_t theta_turnsteps_command = NULL;
static QueueHandle_t phi_turnsteps_command   = NULL;
static QueueHandle_t theta_align_command     = NULL;
static QueueHandle_t phi_align_command       = NULL;

static TaskHandle_t theta_turnsteps_task = NULL;
static TaskHandle_t theta_align_task     = NULL;
static TaskHandle_t phi_turnsteps_task   = NULL;
static TaskHandle_t phi_align_task       = NULL;

static void _turnMotorStepsTask(void *);
static void _alignTask(void *);
static void _enableMotor(CM_Motor_Struct * motor, CM_Dir_Enum dir);
static void _disableMotor(CM_Motor_Struct * motor);
static void _startTurnSteps(CM_Motor_Struct * motor, uint32_t steps);
static void _startTurn(CM_Motor_Struct * motor, uint32_t freq);
static void _stopTurn(CM_Motor_Struct * motor);
static void _eventTurnTheta(void);
static void _eventTurnPhi(void);
static void _eventAlignTheta(void);
static void _eventAlignPhi(void);
static void _faultTheta(void);
static void _faultPhi(void);
static void _storeOrientationInfo(void);
static void _retrieveOrientationInfo(void);

void CM_init(void)
{
    static StaticSemaphore_t theta_ownership_buffer;
    ASSERT(theta_ownership == NULL);
    theta_ownership = xSemaphoreCreateBinaryStatic(&theta_ownership_buffer);
    ASSERT(theta_ownership != NULL);
    xSemaphoreGive(theta_ownership);

    static StaticSemaphore_t phi_ownership_buffer;
    ASSERT(phi_ownership == NULL);
    phi_ownership = xSemaphoreCreateBinaryStatic(&phi_ownership_buffer);
    ASSERT(phi_ownership != NULL);
    xSemaphoreGive(phi_ownership);

    static uint8_t theta_turnsteps_command_storage_buffer[1U*sizeof(CM_TurnStepsCmd_Struct)];
    static StaticQueue_t theta_turnsteps_command_buffer;
    ASSERT(theta_turnsteps_command == NULL);
    theta_turnsteps_command = xQueueCreateStatic(1, sizeof(CM_TurnStepsCmd_Struct), theta_turnsteps_command_storage_buffer, &theta_turnsteps_command_buffer);
    ASSERT(theta_turnsteps_command != NULL);

    static uint8_t phi_turnsteps_command_storage_buffer[1U*sizeof(CM_TurnStepsCmd_Struct)];
    static StaticQueue_t phi_turnsteps_command_buffer;
    ASSERT(phi_turnsteps_command == NULL);
    phi_turnsteps_command = xQueueCreateStatic(1, sizeof(CM_TurnStepsCmd_Struct), phi_turnsteps_command_storage_buffer, &phi_turnsteps_command_buffer);
    ASSERT(phi_turnsteps_command != NULL);

    static uint8_t theta_align_command_storage_buffer[1U*sizeof(CM_AlignCmd_Struct)];
    static StaticQueue_t theta_align_command_buffer;
    ASSERT(theta_align_command == NULL);
    theta_align_command = xQueueCreateStatic(1, sizeof(CM_AlignCmd_Struct), theta_align_command_storage_buffer, &theta_align_command_buffer);
    ASSERT(theta_align_command != NULL);

    static uint8_t phi_align_command_storage_buffer[1U*sizeof(CM_AlignCmd_Struct)];
    static StaticQueue_t phi_align_command_buffer;
    ASSERT(phi_align_command == NULL);
    phi_align_command = xQueueCreateStatic(1, sizeof(CM_AlignCmd_Struct), phi_align_command_storage_buffer, &phi_align_command_buffer);
    ASSERT(phi_align_command != NULL);

    static StackType_t theta_turnsteps_task_stack_buffer[configMINIMAL_STACK_SIZE];
    static StaticTask_t theta_turnsteps_task_task_buffer;
    ASSERT(theta_turnsteps_task == NULL);
    theta_turnsteps_task = xTaskCreateStatic(_turnMotorStepsTask, "turn theta steps", configMINIMAL_STACK_SIZE, (void*)&Theta, 2, theta_turnsteps_task_stack_buffer, &theta_turnsteps_task_task_buffer);
    ASSERT(theta_turnsteps_task != NULL);

    static StackType_t phi_turnsteps_task_stack_buffer[configMINIMAL_STACK_SIZE];
    static StaticTask_t phi_turnsteps_task_task_buffer;
    ASSERT(phi_turnsteps_task == NULL);
    phi_turnsteps_task = xTaskCreateStatic(_turnMotorStepsTask, "turn phi steps", configMINIMAL_STACK_SIZE, (void*)&Phi, 2, phi_turnsteps_task_stack_buffer, &phi_turnsteps_task_task_buffer);
    ASSERT(phi_turnsteps_task != NULL);

    static StackType_t theta_align_task_stack_buffer[configMINIMAL_STACK_SIZE];
    static StaticTask_t theta_align_task_task_buffer;
    ASSERT(theta_align_task == NULL);
    theta_align_task = xTaskCreateStatic(_alignTask, "align theta", configMINIMAL_STACK_SIZE, (void*)&Theta, 2, theta_align_task_stack_buffer, &theta_align_task_task_buffer);
    ASSERT(theta_align_task != NULL);

    static StackType_t phi_align_task_stack_buffer[configMINIMAL_STACK_SIZE];
    static StaticTask_t phi_align_task_task_buffer;
    ASSERT(phi_align_task == NULL);
    phi_align_task = xTaskCreateStatic(_alignTask, "align phi", configMINIMAL_STACK_SIZE, (void*)&Phi, 2, phi_align_task_stack_buffer, &phi_align_task_task_buffer);
    ASSERT(phi_align_task != NULL);

    IO_PinConfig_Struct io_config =
    {
     .initial_out = (IO_Out_Enum)CM_PIN_IDLE,
     .dir = ioDirOutput,
     .ren = false,
     .ds = ioDsReduced,
     .sel = ioSelIo,
     .ies = ioIesFalling,
     .ie = false
    };
    IO_configurePin(Theta.sd_port, Theta.sd_pin, &io_config);
    IO_configurePin(Theta.reset_port, Theta.reset_pin, &io_config);
    IO_configurePin(Theta.dir_port, Theta.dir_pin, &io_config);
    IO_configurePin(Phi.sd_port, Phi.sd_pin, &io_config);
    IO_configurePin(Phi.reset_port, Phi.reset_pin, &io_config);
    IO_configurePin(Phi.dir_port, Phi.dir_pin, &io_config);
    io_config.sel = ioSelPeripheral;
    IO_configurePin(Theta.step_port, Theta.step_pin, &io_config);
    IO_configurePin(Phi.step_port, Phi.step_pin, &io_config);
    io_config.sel = ioSelIo;
    io_config.dir = ioDirInput;
    IO_configurePin(Theta.fault_port, Theta.fault_pin, &io_config);
    IO_configurePin(Theta.es_port, Theta.es_pin, &io_config);
    IO_configurePin(Phi.fault_port, Phi.fault_pin, &io_config);
    IO_configurePin(Phi.es_port, Theta.es_pin, &io_config);

    IO_attachInterrupt(Theta.es_port, Theta.es_pin, _eventAlignTheta);
    IO_attachInterrupt(Phi.es_port, Phi.es_pin, _eventAlignPhi);
    IO_attachInterrupt(Theta.fault_port, Theta.fault_pin, _faultTheta);
    IO_attachInterrupt(Phi.fault_port, Phi.fault_pin, _faultPhi);

    if(((CM_OrientationInfo_Struct *) ORIENTATION_INFO_BASE)->mem_valid != 0)
        _storeOrientationInfo();
    else
        _retrieveOrientationInfo();
}

void CM_setFreq(CM_Motor_Enum motor, uint32_t freq)
{
    ASSERT(freq > 0);
    switch(motor)
    {
    case theta:
        Theta.freq = freq;
        break;
    case phi:
        Phi.freq = freq;
        break;
    default:
        ASSERT(false);
    }
}

uint32_t CM_getFreq(CM_Motor_Enum motor)
{
    switch(motor)
    {
    case theta:
        return Theta.freq;
    case phi:
        return Phi.freq;
    default:
        ASSERT(false);
        return 0;
    }
}

void CM_turnMotorSteps(CM_Motor_Enum motor, uint32_t num_steps, CM_Dir_Enum dir, void (*handler)(void *), void * handler_args)
{
    CM_TurnStepsCmd_Struct cmd =
    {
     .num_steps = num_steps,
     .dir = dir,
     .handler = handler,
     .handler_args = handler_args
    };
    xQueueSend(motor==theta? theta_turnsteps_command : phi_turnsteps_command, &cmd, portMAX_DELAY);
}

void CM_align(CM_Motor_Enum motor, CM_Dir_Enum dir, void (*handler)(void *), void * handler_args)
{
    CM_AlignCmd_Struct cmd =
    {
     .dir = dir,
     .handler = handler,
     .handler_args = handler_args
    };
    xQueueSend(motor==theta? theta_align_command : phi_align_command, &cmd, portMAX_DELAY);
}

void CM_setAlignedInfo(CM_Motor_Enum motor, int32_t value)
{
    ASSERT((motor==theta) || (motor==phi));
    xSemaphoreTake(motor==theta? theta_ownership : phi_ownership, portMAX_DELAY);
    _retrieveOrientationInfo();
    if(motor==theta)
        orientation_info.theta_aligned = value;
    else
        orientation_info.phi_aligned = value;
    orientation_info.aligned_valid = true;
    _storeOrientationInfo();
    ASSERT(orientation_info.aligned_valid == true);
    xSemaphoreGive(motor==theta? theta_ownership : phi_ownership);
}

bool CM_getOrientationInfo(CM_Motor_Enum motor, CM_OrientationInfo_Enum info, int32_t * dest)
{
    ASSERT((motor==theta) || (motor==phi));
    bool rv;
    xSemaphoreTake(motor==theta? theta_ownership : phi_ownership, portMAX_DELAY);
    _retrieveOrientationInfo();
    if(info == aligned)
    {
        if(!orientation_info.aligned_valid)
            rv = false;
        else
        {
            *dest = motor==theta? orientation_info.theta_aligned : orientation_info.phi_aligned;
            rv = true;
        }
    }
    else if(info == current)
    {
        if(!orientation_info.current_valid)
            rv = false;
        else
        {
            *dest = motor==theta? orientation_info.theta_current : orientation_info.phi_current;
            rv = true;
        }
    }
    else
        ASSERT(false);
    xSemaphoreGive(motor==theta? theta_ownership : phi_ownership);
    return rv;
}

static void _turnMotorStepsTask(void * _motor)
{
    CM_Motor_Struct * motor = (CM_Motor_Struct *)_motor;
    SemaphoreHandle_t motor_ownership = motor==&Theta? theta_ownership : phi_ownership;
    QueueHandle_t cmd_queue = motor==&Theta? theta_turnsteps_command : phi_turnsteps_command;
    while(1)
    {
        CM_TurnStepsCmd_Struct cmd;
        xQueueReceive(cmd_queue, &cmd, portMAX_DELAY);
        xSemaphoreTake(motor_ownership, portMAX_DELAY);
        _retrieveOrientationInfo();
        bool orig_state = orientation_info.current_valid;
        orientation_info.current_valid = false;
        _storeOrientationInfo();
        if(cmd.num_steps != 0)
        {
            _enableMotor(motor, cmd.dir);
            _startTurnSteps(motor, cmd.num_steps);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            _stopTurn(motor);
            _disableMotor(motor);
        }
        orientation_info.current_valid = orig_state;
        if(motor == &Theta)
            orientation_info.theta_current += (cmd.dir == counterclockwise? -1 : 1)*cmd.num_steps;
        else
            orientation_info.phi_current += (cmd.dir == counterclockwise? -1 : 1)*cmd.num_steps;
        _storeOrientationInfo();
        xSemaphoreGive(motor_ownership);
        cmd.handler(cmd.handler_args);
    }
}

static void _alignTask(void * _motor)
{
    CM_Motor_Struct * motor = (CM_Motor_Struct *)_motor;
    SemaphoreHandle_t motor_ownership = motor==&Theta? theta_ownership : phi_ownership;
    QueueHandle_t cmd_queue = motor==&Theta? theta_align_command : phi_align_command;
    while(1)
    {
        CM_AlignCmd_Struct cmd;
        xQueueReceive(cmd_queue, &cmd, portMAX_DELAY);
        xSemaphoreTake(motor_ownership, portMAX_DELAY);
        _retrieveOrientationInfo();
        orientation_info.current_valid = false;
        _storeOrientationInfo();
#if DIST_PCB == true
        motor->es_port->IES &= ~motor->es_pin;
        motor->es_port->IFG &= ~motor->es_pin;
        motor->es_port->IE |= motor->es_pin;
        if(IO_readPin(motor->es_port, motor->es_pin) != 0)
        {
            motor->es_port->IES |= motor->es_pin;
            _enableMotor(motor, cmd.dir);
            _startTurn(motor, CM_STEP_FREQ);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            if(cmd.dir == counterclockwise)
            {
                motor->es_port->IES &= ~motor->es_pin;
                motor->es_port->IFG &= ~motor->es_pin;
                _enableMotor(motor, counterclockwise);
                _startTurn(motor, CM_STEP_FREQ);
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            }
        }
        else
        {
            motor->es_port->IES &= ~motor->es_pin;
            _enableMotor(motor, cmd.dir);
            _startTurn(motor, CM_STEP_FREQ);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            if(cmd.dir == clockwise)
            {
                motor->es_port->IES |= motor->es_pin;
                motor->es_port->IFG &= ~motor->es_pin;
                _enableMotor(motor, clockwise);
                _startTurn(motor, CM_STEP_FREQ);
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            }
        }
        motor->es_port->IES ^= motor->es_pin;
        motor->es_port->IFG &= ~motor->es_pin;
        _enableMotor(motor, cmd.dir==clockwise? counterclockwise : clockwise);
        _startTurn(motor, CM_STEP_FREQ>>3);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        motor->es_port->IE &= ~motor->es_pin;
        _disableMotor(motor);
#endif
        if(orientation_info.aligned_valid == true)
            orientation_info.current_valid = true;
        if(motor == &Theta)
            orientation_info.theta_current = -1*orientation_info.theta_aligned;
        else
            orientation_info.phi_current = -1*orientation_info.phi_aligned;
        _storeOrientationInfo();
        xSemaphoreGive(motor_ownership);
        cmd.handler(cmd.handler_args);
    }
}

static void _enableMotor(CM_Motor_Struct * motor, CM_Dir_Enum dir)
{
    IO_writePin(motor->dir_port, motor->dir_pin, (IO_Out_Enum)dir);
    IO_writePin(motor->sd_port, motor->sd_pin, (IO_Out_Enum)CM_PIN_IDLE);
}

static void _disableMotor(CM_Motor_Struct * motor)
{
    IO_writePin(motor->sd_port, motor->sd_pin, (IO_Out_Enum)!CM_PIN_IDLE);
}

static void _startTurnSteps(CM_Motor_Struct * motor, uint32_t steps)
{
    PWM_Config_Struct pwm_config =
    {
     .percent_on = CM_STEP_ONPCT,
     .output = motor->timer_output,
     .freq_hz = motor->freq,
     .initial_output = CM_PIN_IDLE,
     .gradual = true
    };
    PWM_configure(motor->timer_source, &pwm_config);
    PWM_start(motor->timer_source, steps, motor==&Theta? _eventTurnTheta : _eventTurnPhi);
}

static void _startTurn(CM_Motor_Struct * motor, uint32_t freq)
{
    PWM_Config_Struct pwm_config =
    {
     .percent_on = CM_STEP_ONPCT,
     .output = motor->timer_output,
     .freq_hz = freq,
     .initial_output = CM_PIN_IDLE,
     .gradual = true
    };
    PWM_configure(motor->timer_source, &pwm_config);
    PWM_start(motor->timer_source, 0, NULL);
}

static void _stopTurn(CM_Motor_Struct * motor)
{
    PWM_stop(motor->timer_source);
}

static void _eventTurnTheta(void)
{
    vTaskNotifyGiveFromISR(theta_turnsteps_task, NULL);
}

static void _eventTurnPhi(void)
{
    vTaskNotifyGiveFromISR(phi_turnsteps_task, NULL);
}

static void _eventAlignTheta(void)
{
    PWM_stop(Theta.timer_source);
    vTaskNotifyGiveFromISR(theta_align_task, NULL);
}

static void _eventAlignPhi(void)
{
    PWM_stop(Phi.timer_source);
    vTaskNotifyGiveFromISR(phi_align_task, NULL);
}

static void _faultTheta(void)
{
    ASSERT(false);
}

static void _faultPhi(void)
{
    ASSERT(false);
}

static void _storeOrientationInfo(void)
{
    FlashCtl_eraseSegment(ORIENTATION_INFO_BASE);
    FlashCtl_write8((uint8_t *) &orientation_info, ORIENTATION_INFO_BASE, sizeof(CM_OrientationInfo_Struct));
    while(FlashCtl_getStatus(FLASHCTL_BUSY));
}
static void _retrieveOrientationInfo(void)
{
    uint8_t idx;
    for(idx=0; idx<sizeof(CM_OrientationInfo_Struct); ++idx)
        ((uint8_t *) &orientation_info)[idx] = ORIENTATION_INFO_BASE[idx];
}

