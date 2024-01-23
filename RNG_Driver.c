/*
 * RNG_Driver.c
 *
 *  Created on: Nov 28, 2023
 *      Author: aaron
 */

#include "RNG_Driver.h"

static RNG_HandleTypeDef RNG_Handler;

static HAL_StatusTypeDef Status;

/**
 * @brief Initializes the Random Number Generator (RNG) hardware.
 *
 * This function enables the clock for the RNG hardware and initializes it. It is essential
 * for operations that require random number generation, such as generating random shapes or values.
 *
 * @note This function uses the HAL (Hardware Abstraction Layer) library to interact with the RNG hardware.
 *       It checks if the initialization is successful and asserts an application error if not.
 *       The function configures the RNG hardware instance and ensures it is ready for use.
 *
 * Example usage:
 * @code
 *     Init_RNG(); // Initialize the RNG hardware for subsequent random number generation
 * @endcode
 */
void Init_RNG(void) {

	__HAL_RCC_RNG_CLK_ENABLE();

	RNG_Handler.Instance = RNG;
//	RNG_Handler.ErrorCode = 0;
//	RNG_Handler.Lock = 0;
//	RNG_Handler.State = 0;

	if (HAL_RNG_Init(&RNG_Handler) != HAL_OK) {
		// Initialization Error
		APPLICATION_ASSERT(0);
	}

}

/**
 * @brief Generates and returns a random number.
 *
 * This function checks if the Random Number Generator (RNG) hardware is ready and then generates a random number.
 * It is reliant on the proper initialization and readiness of the RNG hardware.
 *
 * @return A 32-bit random number. Returns 0 if the RNG hardware is not ready or if an error occurs during number generation.
 * @note The function uses the HAL library's `HAL_RNG_GenerateRandomNumber` function to generate the random number.
 *       It includes a check for the RNG hardware's state and verifies the number generation process.
 *       Ensure that `Init_RNG` has been called to initialize the RNG hardware before using this function.
 *
 * Example usage:
 * @code
 *     uint32_t randomNumber = getRNG(); // Retrieves a random 32-bit number
 * @endcode
 */
uint32_t getRNG(void) {

	uint32_t Random_num = 0;

	//check if the device is ready

	if (RNG_Handler.State == HAL_RNG_STATE_READY) {
		//generate random num

		Status = HAL_RNG_GenerateRandomNumber(&RNG_Handler, &Random_num);

		RNG_Veryify_Hall();

	}

	//get random num

	return Random_num;

	//return random number

}

/*
 * Checks to make sure the status of the HAL is still okay
 *
 * Goes into a safe infinite loop if there is an error.
 *
 *return uint32_t Random_num
 */
void RNG_Veryify_Hall() {

	if (Status != HAL_OK) {
		APPLICATION_ASSERT(0);
	}

}

