/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 **/

#include "main.h"
#include "ApplicationCode.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void systemClockOverride(void);
//static void MX_GPIO_Init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	// The default system configuration function is "suspect" so we need to make our own clock configuration
	// Note - You, the developer, MAY have to play with some of this coniguration as you progress in your project
	systemClockOverride();

	ApplicationInit();

	// make sure the button interrupt is not already scheduled
	removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);
	// Schedule the start screen
	addSchedulerEvent(SCHEDULER_START_SCREEN);

// Declare global variables

	uint32_t eventsToRUN;
	uint32_t react_Time_LVL_1;
	uint32_t react_Time_LVL_2;
	uint32_t react_Time_LVL_3;
	uint32_t tick1;
	uint32_t tick2;

	// main loop for the program
	// This will poll what events are scheduled complete them and remove them from the scheduler

	while (1) {
		//get the scheduled events
		eventsToRUN = getScheduledEvents();

		if (eventsToRUN & SCHEDULER_START_SCREEN) {

			//display Start Screen
			Start_Screen();

			//add level 1 to the scheduler
			addSchedulerEvent(SCHEDULER_LEVEL_1);
			removeSchedulerEvent(SCHEDULER_START_SCREEN);
		}

		if (eventsToRUN & SCHEDULER_LEVEL_1) {

			//Display level 1
			Display_LVL(LCD_LEVEL_1);

			//make sure they didn't pre press the button during animation
			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			// get time start time
			tick1 = GetTick();

			uint32_t ButtonEvent = 0;

			//check if button has been pressed yet
			while (!(ButtonEvent & SCHEDULER_BUTTON_PRESSED)) {
				ButtonEvent = getScheduledEvents();
			}

			// once button is pressed get time 2
			tick2 = GetTick();

			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			// check how much time has elapsed
			uint32_t time_elapsed = GetTime(tick1, tick2);

			//save time for final screen display
			react_Time_LVL_1 = time_elapsed;

			//display that time
			DisplayTime(time_elapsed);

			HAL_Delay(4000);

			//move on to level 2
			addSchedulerEvent(SCHEDULER_LEVEL_2);

			removeSchedulerEvent(SCHEDULER_LEVEL_1);

		}
		if (eventsToRUN & SCHEDULER_LEVEL_2) {

			// this follows the same button polling structure as level 1

			Display_LVL(LCD_LEVEL_2);

			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			tick1 = GetTick();

			int32_t ButtonEvent = 0;

			while (!(ButtonEvent & SCHEDULER_BUTTON_PRESSED)) {
				ButtonEvent = getScheduledEvents();
			}
			tick2 = GetTick();

			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			uint32_t time_elapsed = GetTime(tick1, tick2);

			react_Time_LVL_2 = time_elapsed;

			DisplayTime(time_elapsed);

			HAL_Delay(4000);

			addSchedulerEvent(SCHEDULER_LEVEL_3);

			removeSchedulerEvent(SCHEDULER_LEVEL_2);

		}
		if (eventsToRUN & SCHEDULER_LEVEL_3) {

			// This follows the previous button polling from level 1 and 2
			// However it does it three times and plays each part of the animation after each button event
			// It then displays your average time and saves the best one for display on the final screen

			//display level 3

			Display_LVL(LCD_LEVEL_3);

			// first part of animation
			Display_Lvl3_Stg1();

			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			tick1 = GetTick();

			int32_t ButtonEvent = 0;

			while (!(ButtonEvent & SCHEDULER_BUTTON_PRESSED)) {
				ButtonEvent = getScheduledEvents();
			}

			tick2 = GetTick();

			uint32_t time_elapsed_1 = GetTime(tick1, tick2);

			// second part of animation
			Display_Lvl3_Stg2();

			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			tick1 = GetTick();

			ButtonEvent = 0;

			while (!(ButtonEvent & SCHEDULER_BUTTON_PRESSED)) {
				ButtonEvent = getScheduledEvents();
			}

			tick2 = GetTick();

			uint32_t time_elapsed_2 = GetTime(tick1, tick2);

			// third part of animation
			Display_Lvl3_Stg3();

			removeSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

			tick1 = GetTick();

			ButtonEvent = 0;

			while (!(ButtonEvent & SCHEDULER_BUTTON_PRESSED)) {
				ButtonEvent = getScheduledEvents();
			}

			tick2 = GetTick();

			uint32_t time_elapsed_3 = GetTime(tick1, tick2);

			uint32_t time_elapsed_AVG = (time_elapsed_1 + time_elapsed_2
					+ time_elapsed_3) / 3;

			// find best time
			if (time_elapsed_1 > time_elapsed_2) {
				react_Time_LVL_3 = time_elapsed_2;
			}
			if (time_elapsed_2 > time_elapsed_3) {
				react_Time_LVL_3 = time_elapsed_3;
			}

			Display_AVG_Time(time_elapsed_AVG);

			HAL_Delay(3000);

			removeSchedulerEvent(SCHEDULER_LEVEL_3);
			addSchedulerEvent(SCHEDULER_END_SCREEN);

		}
		if (eventsToRUN & SCHEDULER_END_SCREEN) {

			//display end screen
			End_Screen(react_Time_LVL_1, react_Time_LVL_2, react_Time_LVL_3);

			removeSchedulerEvent(SCHEDULER_END_SCREEN);
		}
	}

}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 50;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

void systemClockOverride(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();

	// __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); // not needed, power scaling consumption for when not running at max freq.

	/* Enable HSE Osc and activate PLL with HSE source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

// /**
//   * @brief GPIO Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_GPIO_Init(void)
// {
//   GPIO_InitTypeDef GPIO_InitStruct = {0};
// /* USER CODE BEGIN MX_GPIO_Init_1 */
// /* USER CODE END MX_GPIO_Init_1 */

//   /* GPIO Ports Clock Enable */
//   __HAL_RCC_GPIOC_CLK_ENABLE();
//   __HAL_RCC_GPIOF_CLK_ENABLE();
//   __HAL_RCC_GPIOH_CLK_ENABLE();
//   __HAL_RCC_GPIOA_CLK_ENABLE();
//   __HAL_RCC_GPIOB_CLK_ENABLE();
//   __HAL_RCC_GPIOG_CLK_ENABLE();
//   __HAL_RCC_GPIOE_CLK_ENABLE();
//   __HAL_RCC_GPIOD_CLK_ENABLE();

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(ACP_RST_GPIO_Port, ACP_RST_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(GPIOD, RDX_Pin|WRX_DCX_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pins : A0_Pin A1_Pin A2_Pin A3_Pin
//                            A4_Pin A5_Pin SDNRAS_Pin A6_Pin
//                            A7_Pin A8_Pin A9_Pin */
//   GPIO_InitStruct.Pin = A0_Pin|A1_Pin|A2_Pin|A3_Pin
//                           |A4_Pin|A5_Pin|SDNRAS_Pin|A6_Pin
//                           |A7_Pin|A8_Pin|A9_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
//   HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

//   /*Configure GPIO pin : SDNWE_Pin */
//   GPIO_InitStruct.Pin = SDNWE_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
//   HAL_GPIO_Init(SDNWE_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : NCS_MEMS_SPI_Pin CSX_Pin OTG_FS_PSO_Pin */
//   GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//   /*Configure GPIO pins : B1_Pin MEMS_INT1_Pin MEMS_INT2_Pin TP_INT1_Pin */
//   GPIO_InitStruct.Pin = B1_Pin|MEMS_INT1_Pin|MEMS_INT2_Pin|TP_INT1_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//   /*Configure GPIO pin : ACP_RST_Pin */
//   GPIO_InitStruct.Pin = ACP_RST_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(ACP_RST_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pin : OTG_FS_OC_Pin */
//   GPIO_InitStruct.Pin = OTG_FS_OC_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(OTG_FS_OC_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pin : BOOT1_Pin */
//   GPIO_InitStruct.Pin = BOOT1_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : A10_Pin A11_Pin BA0_Pin BA1_Pin
//                            SDCLK_Pin SDNCAS_Pin */
//   GPIO_InitStruct.Pin = A10_Pin|A11_Pin|BA0_Pin|BA1_Pin
//                           |SDCLK_Pin|SDNCAS_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
//   HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

//   /*Configure GPIO pins : D4_Pin D5_Pin D6_Pin D7_Pin
//                            D8_Pin D9_Pin D10_Pin D11_Pin
//                            D12_Pin NBL0_Pin NBL1_Pin */
//   GPIO_InitStruct.Pin = D4_Pin|D5_Pin|D6_Pin|D7_Pin
//                           |D8_Pin|D9_Pin|D10_Pin|D11_Pin
//                           |D12_Pin|NBL0_Pin|NBL1_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
//   HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

//   /*Configure GPIO pins : OTG_HS_ID_Pin OTG_HS_DM_Pin OTG_HS_DP_Pin */
//   GPIO_InitStruct.Pin = OTG_HS_ID_Pin|OTG_HS_DM_Pin|OTG_HS_DP_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   GPIO_InitStruct.Alternate = GPIO_AF12_OTG_HS_FS;
//   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//   /*Configure GPIO pin : VBUS_HS_Pin */
//   GPIO_InitStruct.Pin = VBUS_HS_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(VBUS_HS_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : D13_Pin D14_Pin D15_Pin D0_Pin
//                            D1_Pin D2_Pin D3_Pin */
//   GPIO_InitStruct.Pin = D13_Pin|D14_Pin|D15_Pin|D0_Pin
//                           |D1_Pin|D2_Pin|D3_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
//   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

//   /*Configure GPIO pin : TE_Pin */
//   GPIO_InitStruct.Pin = TE_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   HAL_GPIO_Init(TE_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : RDX_Pin WRX_DCX_Pin */
//   GPIO_InitStruct.Pin = RDX_Pin|WRX_DCX_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

//   /*Configure GPIO pin : I2C3_SDA_Pin */
//   GPIO_InitStruct.Pin = I2C3_SDA_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
//   HAL_GPIO_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pin : I2C3_SCL_Pin */
//   GPIO_InitStruct.Pin = I2C3_SCL_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
//   HAL_GPIO_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : STLINK_RX_Pin STLINK_TX_Pin */
//   GPIO_InitStruct.Pin = STLINK_RX_Pin|STLINK_TX_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
//   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//   /*Configure GPIO pins : LD3_Pin LD4_Pin */
//   GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

//   /*Configure GPIO pins : SDCKE1_Pin SDNE1_Pin */
//   GPIO_InitStruct.Pin = SDCKE1_Pin|SDNE1_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//   GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
//   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

// /* USER CODE BEGIN MX_GPIO_Init_2 */
// /* USER CODE END MX_GPIO_Init_2 */
// }

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
