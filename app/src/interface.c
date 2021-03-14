/*
 * interface.c
 *
 *  Created on: Mar 11, 2021
 *      Author: jgamm
 */

#include <stdint.h>
#include <stdbool.h>
#include <msp430.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

#include "interface.h"
#include "error.h"
#include "control_motors.h"
#include "check_alignment.h"

#include "timerb_hal.h"
#include "io_hal.h"
#include "io_interrupts.h"
#include "hal.h"

#define MAX(A, B)   ((A)>=(B)? (A) : (B))
#define MIN(A, B)   ((A)<=(B)? (A) : (B))
#define STRLEN_C(S) ((sizeof((S))/sizeof(char))-1)

static bool pending_cmds_empty[MAX_CMD_NUM] = {[0 ... (MAX_CMD_NUM-1)]=true};
static char pending_cmds[MAX_CMD_NUM][MAX_CMD_LEN];

static SemaphoreHandle_t rx_available = NULL;
static SemaphoreHandle_t tx_available = NULL;

static TaskHandle_t rx_task = NULL;

static void _rxTask(void *);
static void _parseAndExecuteCmd(char *, uint8_t);
static void _ack(void);
static void _nack(void);
static void _done(void *);
static void _iden(void *);
static void _rsensor(uint16_t, void *);
static void _rassert(void *);
static void _invbsl(void *);
static void _invbslButton(void);
static void _toCaps(char *, uint8_t);
static bool _strEq(const char *, const char *, uint8_t);
static uint8_t _strFind(const char *, char, uint8_t);
static void _strCpy(const char *, char *, uint8_t);
static uint8_t _storePendingCmd(const char *);
static void _rmPendingCmd(uint8_t);
static void _sendData(const char * s, uint8_t n);

void IF_informRx(void)
{
    xSemaphoreGiveFromISR(rx_available, NULL);
}

void IF_informTx(void)
{
    xSemaphoreGiveFromISR(tx_available, NULL);
}

void IF_init(void)
{
    IO_PinConfig_Struct button_config =
    {
     .initial_out = ioOutHigh,
     .dir = ioDirInput,
     .ren = true,
     .ds = ioDsReduced,
     .sel = ioSelIo,
     .ies = ioIesFalling,
     .ie = true
    };
    IO_configurePin(SW2_PORT, SW2_PIN, &button_config);
    IO_attachInterrupt(SW2_PORT, SW2_PIN, _invbslButton);

    static StaticSemaphore_t rx_available_buffer;
    ASSERT(rx_available == NULL);
    rx_available = xSemaphoreCreateBinaryStatic(&rx_available_buffer);
    ASSERT(rx_available != NULL);

    static StaticSemaphore_t tx_available_buffer;
    ASSERT(tx_available == NULL);
    tx_available = xSemaphoreCreateBinaryStatic(&tx_available_buffer);
    ASSERT(tx_available != NULL);
    xSemaphoreGive(tx_available);

    static StackType_t rx_task_stack_buffer[configMINIMAL_STACK_SIZE];
    static StaticTask_t rx_task_task_buffer;
    ASSERT(rx_task == NULL);
    rx_task = xTaskCreateStatic(_rxTask, "rx task", configMINIMAL_STACK_SIZE, NULL, 2, rx_task_stack_buffer, &rx_task_task_buffer);
    ASSERT(rx_task != NULL);

    USB_setup(true, true);
}

static void _rxTask(void * params)
{
    static char rx_buffer[MAX_CMD_LEN];
    uint8_t count = 0;
    while(1)
    {
        xSemaphoreTake(rx_available, portMAX_DELAY);
        count += USBCDC_receiveDataInBuffer((uint8_t *)(rx_buffer+count), MAX_CMD_LEN, CDC0_INTFNUM);
        ASSERT(count<=MAX_CMD_LEN);
        ASSERT(count>=1);
        if(_strFind(rx_buffer, IF_CMDDELIM_CHAR, count) < count)
        {
            _parseAndExecuteCmd(rx_buffer, count);
            count = 0;
        }
    }
}

static void _parseAndExecuteCmd(char * s, uint8_t n)
{
    _toCaps(s, n);
    uint8_t pref_len = _strFind(s, IF_ARGDELIM_CHAR, n);
    if(pref_len == n)
    {
        pref_len = _strFind(s, IF_CMDDELIM_CHAR, n);
        ASSERT(pref_len < n);
    }
    uint8_t cmd_idx = _storePendingCmd(s);
    void (*wb_handler)(void *) = 0;
    void * wb_args = 0;
    if((pref_len==STRLEN_C(IF_IDEN_PREF)) && _strEq(s, IF_IDEN_PREF, pref_len))
    {
        wb_handler = _iden;
        wb_args = (void *)cmd_idx;
    }
    else if((pref_len==STRLEN_C(IF_ALIGN_PREF)) && _strEq(s, IF_ALIGN_PREF, pref_len))
    {
        s += pref_len+1;
        uint8_t arg0_len = _strFind(s, IF_CMDDELIM_CHAR, n);
        ASSERT(arg0_len < n);
        CM_Motor_Enum motor;
        if((arg0_len==STRLEN_C(IF_ALIGN_ARG0_PHI)) && _strEq(s, IF_ALIGN_ARG0_PHI, arg0_len))
            motor = phi;
        else if((arg0_len==STRLEN_C(IF_ALIGN_ARG0_THETA)) && _strEq(s, IF_ALIGN_ARG0_THETA, arg0_len))
            motor = theta;
        else
        {
            _rmPendingCmd(cmd_idx);
            _nack();
            return;
        }
        CM_align(motor, _done, (void *)cmd_idx);
    }
    else if((pref_len==STRLEN_C(IF_WLASER_PREF)) && _strEq(s, IF_WLASER_PREF, pref_len))
    {
        s += pref_len+1;
        uint8_t arg0_len = _strFind(s, IF_CMDDELIM_CHAR, n);
        ASSERT(arg0_len < n);
        bool state;
        if(CA_idProbe() == true)
        {
            _rmPendingCmd(cmd_idx);
            _nack();
            return;
        }
        if((arg0_len==STRLEN_C(IF_WLASER_ARG0_OFF)) && _strEq(s, IF_WLASER_ARG0_OFF, arg0_len))
            state = false;
        else if((arg0_len==STRLEN_C(IF_WLASER_ARG0_ON)) && _strEq(s, IF_WLASER_ARG0_ON, arg0_len))
            state = true;
        else
        {
            _rmPendingCmd(cmd_idx);
            _nack();
            return;
        }
        CA_writeLaser(state, _done, (void *)cmd_idx);
    }
    else if((pref_len==STRLEN_C(IF_RSENSOR_PREF)) && _strEq(s, IF_RSENSOR_PREF, pref_len))
    {
        if(CA_idProbe() == false)
        {
            _rmPendingCmd(cmd_idx);
            _nack();
            return;
        }
        CA_measureSensor(_rsensor, (void *)cmd_idx);

    }
    else if((pref_len==STRLEN_C(IF_RASSERT_PREF)) && _strEq(s, IF_RASSERT_PREF, pref_len))
    {
        wb_handler = _rassert;
        wb_args = (void *)cmd_idx;
    }
    else if((pref_len==STRLEN_C(IF_INVBSL_PREF)) && _strEq(s, IF_INVBSL_PREF, pref_len))
    {
        wb_handler = _invbsl;
        wb_args = (void *)cmd_idx;
    }
    else if((pref_len==STRLEN_C(IF_MOVE_PREF)) && _strEq(s, IF_MOVE_PREF, pref_len))
    {
        s += pref_len+1;
        n -= pref_len+1;
        uint8_t arg0_len = _strFind(s, IF_ARGDELIM_CHAR, MAX_CMD_LEN);
        ASSERT(arg0_len < MAX_CMD_LEN);
        CM_Motor_Enum motor;
        if((arg0_len==STRLEN_C(IF_MOVE_ARG0_PHI)) && _strEq(s, IF_MOVE_ARG0_PHI, arg0_len))
            motor = phi;
        else if((arg0_len==STRLEN_C(IF_MOVE_ARG0_THETA)) && _strEq(s, IF_MOVE_ARG0_THETA, arg0_len))
            motor = theta;
        else
        {
            _rmPendingCmd(cmd_idx);
            _nack();
            return;
        }
        s += arg0_len+1;
        n -= arg0_len+1;
        uint8_t arg1_len = _strFind(s, IF_ARGDELIM_CHAR, n);
        ASSERT(arg1_len < n);
        CM_Dir_Enum dir;
        if((arg1_len==STRLEN_C(IF_MOVE_ARG1_CW)) && _strEq(s, IF_MOVE_ARG1_CW, arg1_len))
            dir = clockwise;
        else if((arg1_len==STRLEN_C(IF_MOVE_ARG1_CCW)) && _strEq(s, IF_MOVE_ARG1_CCW, arg1_len))
            dir = counterclockwise;
        else
        {
            _rmPendingCmd(cmd_idx);
            _nack();
            return;
        }
        s += arg1_len+1;
        n -= arg1_len-1;
        uint8_t arg2_len = _strFind(s, IF_CMDDELIM_CHAR, n);
        ASSERT(arg2_len < n);
        uint32_t num_steps;
        uint32_t base;
        uint8_t idx;
        for(num_steps=0, base=1, idx=arg2_len-1; idx<0xFF; num_steps+=base*((uint32_t)(s[idx]-'0')), --idx, base*=10);

        CM_turnMotorSteps(motor, num_steps, dir, _done, (void *)cmd_idx);
    }
    else
    {
        _rmPendingCmd(cmd_idx);
        _nack();
        return;
    }
    _ack();
    if(wb_handler!=0)
        (*wb_handler)(wb_args);
}

static void _ack(void)
{
    static const char ack_msg[] = IF_ACK_CMD IF_CMDDELIM_STR;
    _sendData(ack_msg, STRLEN_C(ack_msg));
}

static void _nack(void)
{
    static const char nack_msg[] = IF_NACK_CMD IF_CMDDELIM_STR;
    _sendData(nack_msg, STRLEN_C(nack_msg));
}

static void _done(void * _idx)
{
    uint8_t idx = (uint8_t)(uintptr_t)(_idx);
    uint8_t len = _strFind(pending_cmds[idx], IF_CMDDELIM_CHAR, MAX_CMD_LEN)+1;
    ASSERT(len<MAX_CMD_LEN+1);
    _sendData(pending_cmds[idx], len);
    _rmPendingCmd(idx);
}

static void _iden(void * done_args)
{
    static char id_msg[MAX(STRLEN_C(IF_IDEN_RV_TEST), STRLEN_C(IF_IDEN_RV_PROBE))+STRLEN_C(IF_CMDDELIM_STR)];
    if(CA_idProbe())
    {
        _strCpy(IF_IDEN_RV_PROBE IF_CMDDELIM_STR, id_msg, STRLEN_C(id_msg)+1);
        _sendData(id_msg, STRLEN_C(IF_IDEN_RV_PROBE)+1);
    }
    else
    {
        _strCpy(IF_IDEN_RV_TEST IF_CMDDELIM_STR, id_msg, STRLEN_C(id_msg)+1);
        _sendData(id_msg, STRLEN_C(IF_IDEN_RV_TEST)+1);
    }
    _done(done_args);
}

static void _rsensor(uint16_t output, void * done_args)
{
    static char rv_msg[] = "00000" IF_CMDDELIM_STR;
    uint8_t i;
    uint16_t base;
    for(i=0, base=10000; i<5; ++i, base/=10)
        rv_msg[i] = (char)('0'+((output/base)%10));
    _sendData(rv_msg, STRLEN_C(rv_msg));
    _done(done_args);
}

static void _rassert(void * done_args)
{
    static char done_str[131];
    ERROR_AssertInfo_Struct * info = ERROR_lastAssertInfo();
    if(info == 0)
    {
        _done(done_args);
        return;
    }
    uint8_t idx = 0;
    for(idx=0; idx<info->file_len; ++idx)
        done_str[idx] = info->file[idx];
    done_str[idx] = IF_CMDDELIM_CHAR;
    for(idx=0; idx<info->expression_len; ++idx)
        done_str[idx+info->file_len+1] = info->expression[idx];
    idx += info->file_len+1;
    done_str[idx] = IF_CMDDELIM_CHAR;
    ++idx;
    uint16_t base;
    for(base=10000; base>0; base/=10, ++idx)
        done_str[idx] = '0'+((info->line/base)%10);
    done_str[idx] = IF_CMDDELIM_CHAR;
    _sendData(done_str, idx+1);
    _done(done_args);
}

static void _invbslButton(void)
{
    _invbsl((void *)MAX_CMD_NUM);
}

static void _invbsl(void * done_args)
{
    if(done_args != (void *)MAX_CMD_NUM)
        _done(done_args);
    while (USBCDC_getInterfaceStatus(CDC0_INTFNUM, done_args, done_args) & USBCDC_WAITING_FOR_SEND);
    __disable_interrupt();
    TB_reset(TB0);
    ((void (*)()) 0x1000)();
}

static void _toCaps(char * s, uint8_t n)
{
    ASSERT(n>0);
    for(n-=1; n<0xFF; --n)
        if(('a'<=s[n]) && (s[n]<='z'))
            s[n] += ('A'-'a');
}

static bool _strEq(const char * s1, const char * s2, uint8_t n)
{
    ASSERT(n>0);
    for(n-=1; n<0xFF; --n)
        if(s1[n] != s2[n])
            return false;
    return true;
}

static uint8_t _strFind(const char * s, const char target, uint8_t n)
{
    ASSERT(n>0);
    uint8_t idx;
    for(idx=0; (idx<n) && (s[idx]!=target); ++idx);
    return idx;
}

static void _strCpy(const char * src, char * dest, uint8_t n)
{
    ASSERT(n>0);
    for(n-=1; n<0xFF; --n)
        dest[n] = src[n];
}

static uint8_t _storePendingCmd(const char * s)
{
    uint8_t idx;
    for(idx=0; (idx<MAX_CMD_NUM) && (pending_cmds_empty[idx]==false); ++idx);
    ASSERT(idx<MAX_CMD_NUM);
    uint8_t len = _strFind(s, IF_CMDDELIM_CHAR, MAX_CMD_LEN)+1;
    ASSERT(len<MAX_CMD_LEN+1);
    _strCpy(s, pending_cmds[idx], len);
    pending_cmds_empty[idx] = false;
    return idx;
}

static void _rmPendingCmd(uint8_t idx)
{
    ASSERT(idx<MAX_CMD_NUM);
    pending_cmds_empty[idx] = true;
}

static void _sendData(const char * s, uint8_t n)
{
    xSemaphoreTake(tx_available, portMAX_DELAY);
    USBCDC_sendDataInBackground((uint8_t *)s, n, CDC0_INTFNUM, 1);
}
