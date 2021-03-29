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

static void     (*taHandler[3])(void);
static uint32_t ta_up_phase[3];
static uint32_t ta_const_phase[3];
static uint32_t ta_down_phase[3];
static uint16_t ta_target_period[3];
static uint8_t  ta_output[3];
static bool     ta_gradual[3];

void PWM_configure(PWM_Sources_Enum source, PWM_Config_Struct * config)
{
    ASSERT(config->percent_on <= 100);
    uint32_t smclk_freq = UCS_getSMCLK();
    ASSERT(smclk_freq/config->freq_hz < 1UL<<16);
    uint16_t ticks_period = smclk_freq/config->freq_hz;
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
    if(config->gradual)
    {
        TA_setCcr(_TAx(source), 0, 10*ticks_period);
        TA_setCcr(_TAx(source), config->output, 5*ticks_period);
    }
    else
    {
        TA_setCcr(_TAx(source), 0, ticks_period);
        TA_setCcr(_TAx(source), config->output, (uint16_t)(((uint32_t) config->percent_on)*ticks_period)/100);
    }
    ta_gradual[source] = config->gradual;
    ta_output[source] = config->output;
    ta_target_period[source] = ticks_period;
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
    taHandler[source] = handler;
    if(!ta_gradual[source])
    {
        ta_up_phase[source] = 0;
        ta_down_phase[source] = 0;
        ta_const_phase[source] = num_pulses;
    }
    else
    {
        if(num_pulses >= 9*(ta_target_period[source]))
        {
            ta_up_phase[source] = (9*(ta_target_period[source]))>>1 + (9*(ta_target_period[source]))&1;
            ta_down_phase[source] = (9*(ta_target_period[source]))>>1;
            ta_const_phase[source] = num_pulses - 9*(ta_target_period[source]);
        }
        else
        {
            ta_up_phase[source] = (num_pulses >> 1) + (num_pulses & 1);
            ta_down_phase[source] = num_pulses >> 1;
            ta_const_phase[source] = 0;
        }
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
    taHandler[source] = (void *) 0;
    ta_up_phase[source] = 0;
    ta_down_phase[source] = 0;
    ta_const_phase[source] = 0;
}

#include <msp430.h>
#pragma vector=TIMER0_A0_VECTOR
void __attribute__ ((interrupt)) ta0IRQHandler(void)
{
    if(ta_up_phase[0] > 0)
    {
        --(ta_up_phase[0]);
        TA0->CCR[0] -= 2;
        TA0->CCR[ta_output[0]] -= 1;
    }
    else if(ta_const_phase[0] > 0)
    {
        --(ta_const_phase[0]);
    }
    else if(ta_down_phase[0] > 0)
    {
        --(ta_down_phase[0]);
        TA0->CCR[0] += 2;
        TA0->CCR[ta_output[0]] += 1;
    }
    else
    {
        TA0->CTL &= ~TA_CTL_MC;
        TA0->CCTL[0] &= ~TA_CCTL_CCIE;
        (taHandler[0])();
    }
}

#pragma vector=TIMER1_A0_VECTOR
void __attribute__ ((interrupt)) ta1IRQHandler(void)
{
    if(ta_up_phase[1] > 0)
    {
        --(ta_up_phase[1]);
        TA1->CCR[0] -= 2;
        TA1->CCR[ta_output[1]] -= 1;
    }
    else if(ta_const_phase[1] > 0)
    {
        --(ta_const_phase[1]);
    }
    else if(ta_down_phase[1] > 0)
    {
        --(ta_down_phase[1]);
        TA1->CCR[0] += 2;
        TA1->CCR[ta_output[1]] += 1;
    }
    else
    {
        TA1->CTL &= ~TA_CTL_MC;
        TA1->CCTL[0] &= ~TA_CCTL_CCIE;
        (taHandler[1])();
    }
}

#pragma vector=TIMER2_A0_VECTOR
void __attribute__ ((interrupt)) ta2IRQHandler(void)
{
    if(ta_up_phase[2] > 0)
    {
        --(ta_up_phase[2]);
        TA2->CCR[0] -= 2;
        TA2->CCR[ta_output[2]] -= 1;
    }
    else if(ta_const_phase[2] > 0)
    {
        --(ta_const_phase[2]);
    }
    else if(ta_down_phase[2] > 0)
    {
        --(ta_down_phase[2]);
        TA2->CCR[0] += 2;
        TA2->CCR[ta_output[2]] += 1;
    }
    else
    {
        TA2->CTL &= ~TA_CTL_MC;
        TA2->CCTL[0] &= ~TA_CCTL_CCIE;
        (taHandler[2])();
    }
}
