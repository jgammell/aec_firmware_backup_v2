/*
 * interface.c
 *
 *  Created on: Feb 21, 2021
 *      Author: jgamm
 */

static SemaphoreHandle_t usb_ownership = NULL;

static TaskHandle_t rx_task = NULL;
static TaskHandle_t tx_task = NULL;

static void _rxTask(void *);
static void _txTask(void *);

void IF_init(void)
{

}
