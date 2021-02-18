/*
 * error.h
 *
 *  Created on: Feb 8, 2021
 *      Author: jgamm
 */

#ifndef APP_HEADERS_ERROR_H_
#define APP_HEADERS_ERROR_H_

#include <stdint.h>

#define ASSERT(CONDITION) if(!(CONDITION)) _assert_failure(#CONDITION, __FILE__, __LINE__);

void _assert_failure(char * expression, char * file, uint16_t line);

#endif /* APP_HEADERS_ERROR_H_ */
