/*
 * RNG_Driver.h
 *
 *  Created on: Nov 28, 2023
 *      Author: aaron
 */

#ifndef INC_RNG_DRIVER_H_
#define INC_RNG_DRIVER_H_

#include "stm32f4xx_hal.h"
#include "ErrorHandling.h"
#include <stdio.h>

#define TIMEOUT 20000

#define RNG_DR_ADR 0x08
#define READ (1 << 7)


void Init_RNG(void);

uint32_t getRNG(void);

void RNG_Veryify_Hall();



#endif /* INC_RNG_DRIVER_H_ */
