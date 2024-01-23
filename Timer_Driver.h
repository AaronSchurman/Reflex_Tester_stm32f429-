/*
 * Timer_Driver.h
 *
 *  Created on: Nov 30, 2023
 *      Author: aaron
 */

#ifndef INC_TIMER_DRIVER_H_
#define INC_TIMER_DRIVER_H_

#include "stm32f4xx_hal.h"

uint32_t getTick(void);

//returns the diff in the start and stop tick in milliseconds

uint32_t getTime(uint32_t StartTick, uint32_t StopTick);


#endif /* INC_TIMER_DRIVER_H_ */
