/*
 * pwm.c
 *
 *  Created on: Feb 20, 2021
 *      Author: jgamm
 */

#include "pwm.h"
#include "timera_hal.h"
#include "io_hal.h"
#include "error.h"
#include "ucs.h"

#define _TAx(VAL) ((VAL) == timerA0? TA0 : (VAL) == timerA1? TA1 : TA2)
#define _PIN(TIMER, OUTPUT) ((TIMER) == timerA0? P1 : P2),\
                            ((TIMER) == timerA0? (OUTPUT)+1 : (TIMER)==timerA1? (OUTPUT)-1 : (OUTPUT)+3)

#define SMCLK_FREQ (UCS_getSMCLK())//(6000000UL)

static void (*ta0Handler)(void);
static uint32_t ta0_pending;
static void (*ta1Handler)(void);
static uint32_t ta1_pending;
static void (*ta2Handler)(void);
static uint32_t ta2_pending;

void PWM_configure(PWM_Sources_Enum source, PWM_Config_Struct * config)
{
    ASSERT(config->percent_on <= 100);
    ASSERT(SMCLK_FREQ/config->freq_hz < 1UL<<16);
    uint16_t ticks_period = SMCLK_FREQ/config->freq_hz;
    TA_reset(_TAx(source));
    TA_CtlConfig_Struct ctl_config =
    {
     .ie = false,
     .mc = taCtlMcStop,
     .id = taCtlId1,
     .tassel = taCtlTasselSMCLK
    };
    TA_configureCtl(_TAx(source), &ctl_config);
    TA_CctlConfig_Struct cctl_config =
    {
     .out = false,
     .ie = false,
     .outmod = taCctlOutmodOUT,
     .cap = false,
     .scs = false,
     .ccis = taCctlCcisCCIxA,
     .cm = taCctlCmNo
    };
    TA_configureCctl(_TAx(source), 0, &cctl_config);
    cctl_config.outmod = taCctlOutmodSetreset;
    TA_configureCctl(_TAx(source), config->output, &cctl_config);
    TA_setCcr(_TAx(source), 0, ticks_period);
    TA_setCcr(_TAx(source), config->output, (uint16_t)(((uint32_t) config->percent_on)*ticks_period)/100);
    IO_PinConfig_Struct io_config =
    {
     .initial_out = (IO_Out_Enum)config->initial_output,
     .dir = ioDirOutput,
     .ren = false,
     .ds = ioDsReduced,
     .sel = ioSelPeripheral,
     .ies = ioIesRising,
     .ie = false
    };
    IO_configurePin(_PIN(source, config->output), &io_config);
}

void PWM_start(PWM_Sources_Enum source, uint32_t num_pulses, void (*handler)(void))
{
    switch(source)
    {
    case timerA0:
        ta0Handler = handler;
        ta0_pending = num_pulses-1;
        break;
    case timerA1:
        ta1Handler = handler;
        ta1_pending = num_pulses-1;
        break;
    case timerA2:
        ta2Handler = handler;
        ta2_pending = num_pulses-1;
        break;
    }
    if(num_pulses != 0)
    {
        _TAx(source)->CCTL[0] |= TA_CCTL_CCIE;
    }
    TA_start(_TAx(source), taCtlMcUp);
}

void PWM_stop(PWM_Sources_Enum source)
{
    TA_stop(_TAx(source));
    switch(source)
    {
    case timerA0:
        ta0Handler = (void *) 0;
        ta0_pending = 0;
        break;
    case timerA1:
        ta1Handler = (void *) 0;
        ta1_pending = 0;
        break;
    case timerA2:
        ta2Handler = (void *) 0;
        ta2_pending = 0;
        break;
    }
}

#include <msp430.h>
#pragma vector=TIMER0_A0_VECTOR
void __attribute__ ((interrupt)) ta0IRQHandler(void)
{
    if(ta0_pending == 0)
    {
        TA0->CTL &= ~TA_CTL_MC;
        TA0->CCTL[0] &= ~TA_CCTL_CCIE;
        (*ta0Handler)();
    }
    --ta0_pending;
}

#pragma vector=TIMER1_A0_VECTOR
void __attribute__ ((interrupt)) ta1IRQHandler(void)
{
    if(ta1_pending == 0)
    {
        TA1->CTL &= ~TA_CTL_MC;
        TA1->CCTL[0] &= ~TA_CCTL_CCIE;
        (*ta1Handler)();
    }
    --ta1_pending;
}

#pragma vector=TIMER2_A0_VECTOR
void __attribute__ ((interrupt)) ta2IRQHandler(void)
{
    if(ta2_pending == 0)
    {
        TA2->CTL &= ~TA_CTL_MC;
        TA2->CCTL[0] &= ~TA_CCTL_CCIE;
        (*ta2Handler)();
    }
    --ta2_pending;
}
