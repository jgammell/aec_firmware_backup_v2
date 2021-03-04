/*
 * error.c
 *
 *  Created on: Feb 8, 2021
 *      Author: jgamm
 */

#include "error.h"
#include "io_hal.h"
#include <msp430.h>
#include <stdbool.h>

#define LED5_PORT (P7)
#define LED5_PIN  (IO_PIN5)

void __attribute__ ((noreturn)) _assert_failure(char * expression, char * file, uint16_t line)
{
    __disable_interrupt();
    volatile uint32_t i;
    IO_PinConfig_Struct led_config =
    {
     .initial_out = ioOutLow,
     .dir = ioDirOutput,
     .ren = false,
     .ds = ioDsReduced,
     .sel = ioSelIo,
     .ies = ioIesFalling,
     .ie = false
    };
    IO_configurePin(LED5_PORT, LED5_PIN, &led_config);
    while(1)
    {
        for(i=0xFFFF; i<0x10000; --i);
        IO_writePin(LED5_PORT, LED5_PIN, (IO_Out_Enum)!IO_readPin(LED5_PORT, LED5_PIN));
    }
}


