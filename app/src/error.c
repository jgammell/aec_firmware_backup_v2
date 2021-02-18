/*
 * error.c
 *
 *  Created on: Feb 8, 2021
 *      Author: jgamm
 */

#include "error.h"

void __attribute__ ((noreturn)) _assert_failure(char * expression, char * file, uint16_t line)
{
    while(1);
}


