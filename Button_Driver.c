/*
 * Button_Driver.c
 *
 *  Created on: Sep 26, 2023
 *      Author: aaron
 */

#include "Button_Driver.h"

static GPIO_InitTypeDef button;




/// I changed this to remove the interupt shit cuae i was cinfused
void Init_Interrupt_Mode(){

	button.Pin = GPIO_PIN_0;

	button.Mode = GPIO_MODE_IT_RISING;
	button.Speed = GPIO_SPEED_FREQ_MEDIUM;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	HAL_GPIO_Init(GPIOA, &button);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

//	GPIO_ENABLE_DISABLE_INT(SET,INTERRUPT_CONTROL_EXT0_IQR_NUM);
}




void Clock_Enable(){

	__HAL_RCC_GPIOA_CLK_ENABLE();

}


/**
 * @brief Initializes a button as an input device.
 *
 * This function sets up a GPIO pin as an input for a button. It configures the button's pin number,
 * mode, and speed. The function also enables the clock for the GPIO port to which the button is connected.
 *
 * @note This function is specific to a button connected to a particular GPIO pin (in this case, GPIO_PIN_0)
 *       on GPIO port A. The function configures the button with medium speed and input mode. It assumes
 *       the use of the HAL library for GPIO and clock configuration. Make sure to adapt the pin number and
 *       port according to your hardware setup.
 *
 * Example usage:
 * @code
 *     Init_Button(); // Initialize the button connected to GPIO_PIN_0 on GPIO port A
 * @endcode
 */
void Init_Button(){


	button.Pin = GPIO_PIN_0;

	button.Mode = GPIO_MODE_INPUT;

	button.Speed = GPIO_SPEED_FREQ_MEDIUM;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	HAL_GPIO_Init(GPIOA, &button);

}


/**
 * @brief Checks if the button is pressed.
 *
 * This function reads the state of a GPIO pin configured for a button and returns the button's status.
 * It returns true if the button is pressed and false otherwise.
 *
 * @return A boolean value indicating the button's status. Returns true if the button is pressed, false otherwise.
 * @note This function relies on the HAL library's `HAL_GPIO_ReadPin` function. It checks the state of the
 *       GPIO pin assigned to the `button` variable. Ensure that `button.Pin` is correctly initialized and
 *       corresponds to the actual hardware configuration.
 *
 * Example usage:
 * @code
 *     if (Button_Pressed()) {
 *         // Code to execute when button is pressed
 *     }
 * @endcode
 */
_Bool Button_Pressed(){
	if(HAL_GPIO_ReadPin(GPIOA, button.Pin)){
		return true;
	}
	else{
		return false;
	}

}


