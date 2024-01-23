/*
 * Button_Driver.h
 *
 *  Created on: Sep 26, 2023
 *      Author: aaron
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_

#include "stm32f4xx_hal.h"

#include <stdbool.h>

#define BUTTON_DRIVER_PORT GPIOA
#define BUTTON_DRIVER_PIN 0
#define BUTTON_DRIVER_IS_PRESSED 1
#define BUTTON_DRIBER_IS_NOT_PRESSED 0




void Init_Button();

void Clock_Enable();

_Bool Button_Pressed();

void Init_Interrupt_Mode();


#endif /* BUTTON_DRIVER_H_ */
