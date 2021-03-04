/*
 * check_alignment.h
 *
 *  Created on: Mar 2, 2021
 *      Author: jgamm
 */

#ifndef APP_HEADERS_CHECK_ALIGNMENT_H_
#define APP_HEADERS_CHECK_ALIGNMENT_H_

#include <stdint.h>
#include <stdbool.h>

#define CA_TXRXID_PORT    (P5)
#define CA_TXRXID_PIN     (IO_PIN6)
#define CA_INTENSITY_PORT (P6)
#define CA_INTENSITY_PIN  (IO_PIN1)
#define CA_INTENSITY_APIN (ADC12_A_INPUT_A1)
#define CA_INTENSITY_AMEN (ADC12_A_MEMORY_0)
#define CA_INTENSITY_AIFG (ADC12_A_IFG0)
#define CA_POWER_PORT     (P6)
#define CA_POWER_PIN      (IO_PIN3)

void CA_init(void);

void CA_writeLaser(bool state, void (*handler)(void));

void CA_measureSensor(void (*handler)(uint16_t));

#endif /* APP_HEADERS_CHECK_ALIGNMENT_H_ */
