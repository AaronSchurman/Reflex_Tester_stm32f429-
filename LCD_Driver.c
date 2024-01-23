/*
 * LCD_Driver.c
 *
 *  Created on: Nov 14, 2023
 *      Author: Mathias, modified by Xavion
 *      
 */

#include "LCD_Driver.h"

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor = 0xFFFF;

static SPI_HandleTypeDef SpiHandle;
uint32_t SpiTimeout = SPI_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */

//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT] = { 0 }; //16bpp pixel format.

//static void MX_LTDC_Init(void);
//static void MX_SPI5_Init(void);
static void SPI_MspInit(SPI_HandleTypeDef *hspi);
static void SPI_Error(void);

/* Provided Functions and API  - MOTIFY ONLY WITH EXTREME CAUTION!!! */

void LCD_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the LTDC clock */
	__HAL_RCC_LTDC_CLK_ENABLE();

	/* Enable GPIO clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/* GPIO Config
	 *
	 LCD pins
	 LCD_TFT R2 <-> PC.10
	 LCD_TFT G2 <-> PA.06
	 LCD_TFT B2 <-> PD.06
	 LCD_TFT R3 <-> PB.00
	 LCD_TFT G3 <-> PG.10
	 LCD_TFT B3 <-> PG.11
	 LCD_TFT R4 <-> PA.11
	 LCD_TFT G4 <-> PB.10
	 LCD_TFT B4 <-> PG.12
	 LCD_TFT R5 <-> PA.12
	 LCD_TFT G5 <-> PB.11
	 LCD_TFT B5 <-> PA.03
	 LCD_TFT R6 <-> PB.01
	 LCD_TFT G6 <-> PC.07
	 LCD_TFT B6 <-> PB.08
	 LCD_TFT R7 <-> PG.06
	 LCD_TFT G7 <-> PD.03
	 LCD_TFT B7 <-> PB.09
	 LCD_TFT HSYNC <-> PC.06
	 LCDTFT VSYNC <->  PA.04
	 LCD_TFT CLK   <-> PG.07
	 LCD_TFT DE   <->  PF.10
	 */

	/* GPIOA configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
	GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* GPIOB configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_8 |
	GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* GPIOC configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* GPIOD configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* GPIOF configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_10;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* GPIOG configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 |
	GPIO_PIN_11;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* GPIOB configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStructure.Alternate = GPIO_AF9_LTDC;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* GPIOG configuration */
	GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

void LTCD__Init(void) {
	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK) {
		LCD_Error_Handler();
	}

	ili9341_Init();
}

void LTCD_Layer_Init(uint8_t LayerIndex) {
	LTDC_LayerCfgTypeDef pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1; //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0) {
		pLayerCfg.FBStartAdress = (uintptr_t) frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK) {
		LCD_Error_Handler();
	}

}

// Draws a single pixel, should be useds only within this fileset and should not be seen by external clients. 
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color) {
	frameBuffer[y * LCD_PIXEL_WIDTH + x] = color; //You cannot do x*y to set the pixel.

}


/**
 * @brief Draws a character on the LCD screen.
 *
 * This function draws a character at the specified X and Y coordinates using the current font settings.
 * The character data is represented as an array of `uint16_t`, where each bit represents a pixel in the character.
 *
 * @param Xpos X-coordinate on the screen where the character will be drawn.
 * @param Ypos Y-coordinate on the screen where the character will be drawn.
 * @param c Pointer to the character data to be drawn.
 * @note This function relies on the global font setting `LCD_Currentfonts`, which should be set up beforehand.
 *       It also uses `LCD_Draw_Pixel` to draw individual pixels of the character.
 *       The character is drawn using the current text color `CurrentTextColor`.
 *
 * Example usage:
 * @code
 *     const uint16_t myChar[12] = {...}; // Character data for 'A'
 *     LCD_DrawChar(100, 50, myChar); // Draws the character 'A' at position (100, 50)
 * @endcode
 */
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c) {
	uint32_t index = 0, counter = 0;
	for (index = 0; index < LCD_Currentfonts->Height; index++) {
		for (counter = 0; counter < LCD_Currentfonts->Width; counter++) {
			if ((((c[index]
					& ((0x80 << ((LCD_Currentfonts->Width / 12) * 8)) >> counter))
					== 0x00) && (LCD_Currentfonts->Width <= 12))
					|| (((c[index] & (0x1 << counter)) == 0x00)
							&& (LCD_Currentfonts->Width > 12))) {
				//Background If want to overrite text under then add a set color here
			} else {
				LCD_Draw_Pixel(counter + Xpos, index + Ypos, CurrentTextColor);
			}
		}
	}
}

// Displays Char
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii) {
	Ascii -= 32;
	LCD_DrawChar(Xpos, Ypos,
			&LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void LCD_SetTextColor(uint16_t Color) {
	CurrentTextColor = Color;
}

void LCD_SetFont(FONT_t *fonts) {
	LCD_Currentfonts = fonts;
}

/**
 * @brief Draws a filled circle on the screen.
 *
 * This function draws a filled circle at the specified coordinates with the given radius and color.
 * The algorithm calculates each pixel within the circle's radius and fills it with the specified color.
 *
 * @param Xpos X-coordinate of the center of the circle on the screen.
 * @param Ypos Y-coordinate of the center of the circle on the screen.
 * @param radius Radius of the circle.
 * @param color Color of the circle in a 16-bit format.
 * @note The function uses a simple rasterization algorithm to determine which pixels
 *       fall within the circle's radius and then draws them using `LCD_Draw_Pixel`.
 *
 * Example usage:
 * @code
 *     LCD_Draw_Circle_Fill(120, 160, 20, 0xFFFF); // Draws a filled circle at (120, 160) with a radius of 20 pixels in white color
 * @endcode
 */
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius,
		uint16_t color) {
	for (int16_t y = -radius; y <= radius; y++) {
		for (int16_t x = -radius; x <= radius; x++) {
			if (x * x + y * y <= radius * radius) {
				LCD_Draw_Pixel(x + Xpos, y + Ypos, color);
			}
		}
	}
}

/**
 * @brief Draws a filled triangle on the screen.
 *
 * This function draws a filled triangle based on the specified base, height, and color.
 * The triangle is drawn with its base centered at the provided Xpos and Ypos coordinates.
 *
 * @param Xpos X-coordinate of the center of the triangle's base on the screen.
 * @param Ypos Y-coordinate of the center of the triangle's base on the screen.
 * @param base Length of the base of the triangle.
 * @param height Height of the triangle.
 * @param color Color of the triangle in a 16-bit format.
 * @note The function calculates the vertices of the triangle and fills it by drawing
 *       horizontal lines between the calculated start and end points for each row.
 *       This function relies on `LCD_Draw_Pixel` to draw individual pixels.
 *
 * Example usage:
 * @code
 *     LCD_Draw_Triangle_Fill(100, 150, 60, 30, 0xFFFF); // Draws a filled triangle centered at (100, 150) with a base of 60 and height of 30 in white color
 * @endcode
 */

void LCD_Draw_Triangle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t base,
		uint16_t height, uint16_t color) {

	//clculations for vertices of the line
	uint16_t x1 = Xpos - base / 2;
	uint16_t y1 = Ypos + height / 2;
	uint16_t x2 = Xpos + base / 2;
	uint16_t y2 = Ypos + height / 2;
	uint16_t x3 = Xpos;
	uint16_t y3 = Ypos - height / 2;

	// calculations for slopes of the line
	int16_t dx, dy, startx, endx;
	float slope1 = (float) (y1 - y3) / (x1 - x3);
	float slope2 = (float) (y2 - y3) / (x2 - x3);

	for (dy = y3; dy <= y1; dy++) {

		//draw triagle
		startx = slope1 * (dy - y3) + x3;
		endx = slope2 * (dy - y3) + x3;

		if (startx > endx) {
			int16_t temp = startx;
			startx = endx;
			endx = temp;
		}

		for (dx = startx; dx <= endx; dx++) {
			LCD_Draw_Pixel(dx, dy, color);
		}
	}
}


/**
 * @brief Draws a filled square on the screen.
 *
 * This function draws a square filled with a specified color. The square is defined by its center
 * coordinates, side length, and color. The square's position is calculated to be centered around
 * the provided x and y coordinates.
 *
 * @param x X-coordinate of the square's center on the screen.
 * @param y Y-coordinate of the square's center on the screen.
 * @param length The length of the sides of the square.
 * @param color Color of the square in a 16-bit format.
 * @note This function calls `LCD_Draw_Pixel` to draw each pixel of the square. Ensure that the
 *       `LCD_Draw_Pixel` function is correctly implemented and can draw individual pixels on the screen.
 *
 * Example usage:
 * @code
 *     LCD_Draw_Square_Fill(120, 160, 50, 0xFFFF); // Draws a filled square with a side length of 50 at (120, 160) with white color
 * @endcode
 */
void LCD_Draw_Square_Fill(uint16_t x, uint16_t y, uint16_t length, uint16_t color)
{
    for(uint16_t i = (x - length/2); i < (x + length/2); i++)
    {
        for(uint16_t j = (y - length/2); j < (y + length/2); j++)
        {
            LCD_Draw_Pixel(i, j, color);
        }
    }
}


/**
 * @brief Draws a random shape (circle, square, or triangle) on the screen.
 *
 * This function uses a random number generator to determine which shape to draw.
 * It can draw a circle, a square, or a triangle, each with predetermined dimensions.
 * The shape is chosen randomly with equal probability for each shape.
 *
 * @param Xpos X-coordinate of the shape's position on the screen.
 * @param Ypos Y-coordinate of the shape's position on the screen.
 * @param color Color of the shape in a 16-bit format.
 * @note The function draws one of the three shapes based on the result of the random number generator:
 *       - 0: Circle with radius 15 pixels.
 *       - 1: Triangle with a base of 90 pixels and height of 30 pixels.
 *       - 2: Square with side length of 50 pixels.
 *       The function assumes that the RNG function (`getRNG()`) is initialized and generates a valid random number.
 * @return void
 *
 * Example usage:
 * @code
 *     LCD_Draw_Random_Shape(100, 150, 0xFFFF); // Draws a random shape at (100, 150) with white color
 * @endcode
 */

void LCD_Draw_Random_Shape(uint16_t Xpos, uint16_t Ypos, uint16_t color) {

	uint32_t rng = getRNG();

	uint32_t choice = rng  % 3;

	if (choice == 0) {
		//draw a circle half the time

		LCD_Draw_Circle_Fill(Xpos, Ypos, 15, color);
	}

	if (choice == 1) {

		//draw a triangle half the time
		LCD_Draw_Triangle_Fill(Xpos, Ypos, 90, 30, color);
	}
	else{
		LCD_Draw_Square_Fill(Xpos, Ypos, 50, color);
	}
}

/**
 * @brief Draws a random shape (circle, triangle, or square) on the screen and returns the shape type.
 *
 * This function uses a random number generator to select a shape to draw: a circle, a triangle, or a square.
 * It then draws the selected shape on the screen at the specified coordinates and returns an integer
 * representing the type of shape drawn.
 *
 * @param Xpos X-coordinate of the shape's position on the screen.
 * @param Ypos Y-coordinate of the shape's position on the screen.
 * @param color Color of the shape in a 16-bit format.
 * @return An integer representing the shape drawn: 0 for circle, 1 for triangle, and 2 for square.
 * @note The shapes are drawn with predefined dimensions. The circle has a radius of 15 pixels,
 *       the triangle has a base of 90 pixels and a height of 30 pixels, and the square has a side of 50 pixels.
 *       This function assumes the `getRNG()` function is initialized and generates a valid random number.
 *
 * Example usage:
 * @code
 *     uint16_t shape = LCD_Draw_Random_Shape_Return_Shape(100, 150, 0xFFFF);
 *     // Draws a random shape at (100, 150) with white color and returns the shape type
 * @endcode
 */

uint16_t LCD_Draw_Random_Shape_Return_Shape(uint16_t Xpos, uint16_t Ypos, uint16_t color) {

	uint32_t rng = getRNG();

	uint16_t shape = 0;

	uint32_t choice = rng  % 3;

	if (choice == 0) {
		//draw a circle half the time

		LCD_Draw_Circle_Fill(Xpos, Ypos, 15, color);

		return shape = 0;

	}

	if (choice == 1) {

		//draw a triangle half the time
		LCD_Draw_Triangle_Fill(Xpos, Ypos, 90, 30, color);
		return shape = 1;
	}
	else{
		LCD_Draw_Square_Fill(Xpos, Ypos, 50, color);
		return shape = 2;
	}

	return shape;
}

/**
 * @brief Draws a vertical line on the screen.
 *
 * This function draws a vertical line starting from the specified coordinates (x, y).
 * The length and color of the line are also specified. The line is drawn downwards
 * from the starting point.
 *
 * @param x X-coordinate of the starting point of the line on the screen.
 * @param y Y-coordinate of the starting point of the line on the screen.
 * @param len Length of the line in pixels.
 * @param color Color of the line in a 16-bit format.
 * @note This function uses `LCD_Draw_Pixel` to draw each pixel of the line. It is
 *       essential that `LCD_Draw_Pixel` is properly implemented.
 *
 * Example usage:
 * @code
 *     LCD_Draw_Vertical_Line(100, 50, 30, 0xFFFF); // Draws a vertical line of length 30 pixels starting at (100, 50) in white color
 * @endcode
 */
void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len,
		uint16_t color) {
	for (uint16_t i = 0; i < len; i++) {
		LCD_Draw_Pixel(x, i + y, color);
	}
}

/**
 * @brief Draws a horizontal line on the screen.
 *
 * This function draws a horizontal line starting from the specified coordinates (x, y).
 * The line extends to the right of the starting point with the specified length and color.
 *
 * @param x X-coordinate of the starting point of the line on the screen.
 * @param y Y-coordinate of the starting point of the line on the screen.
 * @param len Length of the line in pixels.
 * @param color Color of the line in a 16-bit format.
 * @note The function iteratively draws pixels using `LCD_Draw_Pixel`.
 *       Ensure that `LCD_Draw_Pixel` is correctly implemented for proper functionality.
 *
 * Example usage:
 * @code
 *     LCD_Draw_Horizontal_Line(100, 50, 40, 0xFFFF); // Draws a horizontal line of length 40 pixels starting at (100, 50) in white color
 * @endcode
 */
void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len,
		uint16_t color) {
	for (uint16_t i = 0; i < len; i++) {
		LCD_Draw_Pixel(i + x, y, color);
	}
}

/**
 * @brief Clears the specified layer of the LCD with a given color.
 *
 * This function fills the entire specified layer of the LCD with a single color, effectively
 * clearing the layer. It writes the given color value to all pixels in the framebuffer of the layer.
 *
 * @param LayerIndex Index of the layer to be cleared. Currently, only layer 0 is supported.
 * @param Color Color to fill the layer, specified in a 16-bit format.
 * @note This function operates on a predefined framebuffer and its effect is based on the resolution
 *       defined by `LCD_PIXEL_WIDTH` and `LCD_PIXEL_HEIGHT`. The function only clears Layer 0,
 *       as specified by the condition `if (LayerIndex == 0)`.
 *
 * Example usage:
 * @code
 *     LCD_Clear(0, 0xFFFF); // Clears layer 0 of the LCD and sets it to white color
 * @endcode
 */
void LCD_Clear(uint8_t LayerIndex, uint16_t Color) {
	if (LayerIndex == 0) {
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++) {
			frameBuffer[i] = Color;
		}
	}
}

void LCD_Error_Handler(void) {
	for (;;)
		; // Something went wrong
}

// Demo using provided functions
void QuickDemo(void) {
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for (y = 0; y < LCD_PIXEL_HEIGHT; y++) {
		for (x = 0; x < LCD_PIXEL_WIDTH; x++) {
			if (x & 32)
				frameBuffer[x * y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x * y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10, 10, 250, LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230, 10, 250, LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	LCD_Draw_Circle_Fill(125, 150, 20, LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0, LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100, 140, 'H');
	LCD_DisplayChar(115, 140, 'e');
	LCD_DisplayChar(125, 140, 'l');
	LCD_DisplayChar(130, 140, 'l');
	LCD_DisplayChar(140, 140, 'o');

	LCD_DisplayChar(100, 160, 'W');
	LCD_DisplayChar(115, 160, 'o');
	LCD_DisplayChar(125, 160, 'r');
	LCD_DisplayChar(130, 160, 'l');
	LCD_DisplayChar(140, 160, 'd');
	HAL_Delay(2000);

}

/**
 * @brief Displays different levels on the LCD screen based on the input level.
 *
 * This function performs various display operations on the LCD screen based on the specified level.
 * It includes operations like clearing the screen, setting text color and font, displaying characters,
 * and drawing shapes. Each level has distinct display content and behavior.
 *
 * @param Level The level to display. Different levels have different display content.
 *              - LCD_LEVEL_1: Clears the screen, sets colors, displays "Level One" text, and provides instructions.
 *              - LCD_LEVEL_2: Displays "Level Two" text with specific instructions and draws a grid with random shapes.
 *              - LCD_LEVEL_3: Displays "Level Three" text with specific instructions.
 * @note This function uses several other functions like `LCD_Clear`, `LCD_SetTextColor`, `LCD_SetFont`,
 *       `LCD_DisplayChar`, and `LCD_Draw_Random_Shape_Return_Shape` to achieve its functionality.
 *       Ensure these functions are correctly implemented.
 *       The actual display content and behavior for each level are hardcoded within the function.
 *
 * Example usage:
 * @code
 *     LCD_Display(LCD_LEVEL_1); // Displays the content for Level 1
 * @endcode
 */

void LCD_Display(uint8_t Level) {

	if (Level == LCD_LEVEL_1) {

		//clear LCD screen and set color to blue
		LCD_Clear(0, LCD_COLOR_WHITE);
		HAL_Delay(2000);
		LCD_Clear(0, LCD_COLOR_BLUE);
		LCD_SetTextColor(LCD_COLOR_BLACK);
		LCD_SetFont(&Font16x24);

		//display Level 1 text
		LCD_DisplayChar(100, 140, 'L');
		LCD_DisplayChar(115, 140, 'e');
		LCD_DisplayChar(130, 140, 'v');
		LCD_DisplayChar(145, 140, 'e');
		LCD_DisplayChar(160, 140, 'l');

		LCD_DisplayChar(100, 160, 'O');
		LCD_DisplayChar(115, 160, 'n');
		LCD_DisplayChar(130, 160, 'e');

		HAL_Delay(2000);

		//clear screen
		LCD_Clear(0, LCD_COLOR_WHITE);

		//give instructions


		uint16_t shape = 0;
		uint16_t instr_length = 34;
		uint16_t x = 20;
		uint16_t y = 40;

		char instr[] = { "Press Button When Triangle Appears" };

		//loops over instruction string to display each character

		for (uint8_t i = 0; i < instr_length; i++) {

			//if you are close to the edge of the screen reset to the left side and move down by 50 pixels
			if (x >= 200) {
				y += 50;
				x = 20;
			}

			LCD_DisplayChar(x, y, instr[i]);

			x += 15;

		}

		HAL_Delay(2000);

		//draw a random shape for on the screen
		uint8_t flag = 0;
		while(flag == 0){
		HAL_Delay(1000);
		LCD_Clear(0, LCD_COLOR_WHITE);

		shape = LCD_Draw_Random_Shape_Return_Shape(125, 150, LCD_COLOR_BLACK);
		if (shape == 1){
			flag = 1;
		}
		}
	}

	if (Level == LCD_LEVEL_2) {

		//display Level 2 screen
		LCD_Clear(0, LCD_COLOR_WHITE);
		HAL_Delay(2000);
		LCD_Clear(0, LCD_COLOR_BLUE);
		LCD_SetTextColor(LCD_COLOR_BLACK);
		LCD_SetFont(&Font16x24);

		LCD_DisplayChar(100, 140, 'L');
		LCD_DisplayChar(115, 140, 'e');
		LCD_DisplayChar(130, 140, 'v');
		LCD_DisplayChar(145, 140, 'e');
		LCD_DisplayChar(160, 140, 'l');

		LCD_DisplayChar(100, 160, 'T');
		LCD_DisplayChar(115, 160, 'w');
		LCD_DisplayChar(130, 160, 'o');

		HAL_Delay(2000);

		LCD_Clear(0, LCD_COLOR_WHITE);

		//give instructions

		uint16_t instr_length = 51;
		uint16_t x = 20;
		uint16_t y = 40;

		char instr[] = { "Press Button When Triangle Appears In Top Right Box" };

		for (uint8_t i = 0; i < instr_length; i++) {

			if (x >= 200) {
				y += 50;
				x = 20;
			}

			LCD_DisplayChar(x, y, instr[i]);

			x += 15;
		}

		HAL_Delay(3000);

		LCD_Clear(0, LCD_COLOR_WHITE);

		//draw gird on the screen
		LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10,
				(LCD_PIXEL_HEIGHT - 20), LCD_COLOR_BLACK);
		LCD_Draw_Horizontal_Line(10, LCD_PIXEL_HEIGHT / 2,
				(LCD_PIXEL_WIDTH - 20), LCD_COLOR_BLACK);

		//create a flag to tell if the shape is in the upper right grid location
		uint8_t UpperRightFlag = 0;

		uint16_t shape = 0;

		uint16_t Cor_Shape = 1;

		//while the shape is not yet generated in the upper right corner
		while (!UpperRightFlag) {

			HAL_Delay(500);

			//get a new random number
			uint32_t rng = getRNG();

			//clear screen and draw a new grid (this gets rid of previous shape)
			LCD_Clear(0, LCD_COLOR_WHITE);

			LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10,
					(LCD_PIXEL_HEIGHT - 20), LCD_COLOR_BLACK);
			LCD_Draw_Horizontal_Line(10, LCD_PIXEL_HEIGHT / 2,
					(LCD_PIXEL_WIDTH - 20), LCD_COLOR_BLACK);

			//Divide max random number by 4 then check which 1/4 the the random number landed in
			//put a random shape into that quadrant
			if (rng < (0xffffffff / 4)) {
				//upper left

				LCD_Draw_Random_Shape(LCD_PIXEL_WIDTH / 4, LCD_PIXEL_HEIGHT / 4,
				LCD_COLOR_BLACK);
			}

			else if (rng < ((0xffffffff / 4) * 2)) {
				//upper right
				shape = LCD_Draw_Random_Shape_Return_Shape((LCD_PIXEL_WIDTH / 4) * 3,
				LCD_PIXEL_HEIGHT / 4, LCD_COLOR_BLACK);

				if(Cor_Shape == shape){
					UpperRightFlag = 1;
				}

			}

			else if (rng < ((0xffffffff / 4) * 3)) {
				//lower left

				LCD_Draw_Random_Shape((LCD_PIXEL_WIDTH / 4) * 3,
						(LCD_PIXEL_HEIGHT / 4) * 3, LCD_COLOR_BLACK);
			} else {
				//lower right

				LCD_Draw_Random_Shape((LCD_PIXEL_WIDTH / 4),
						(LCD_PIXEL_HEIGHT / 4) * 3, LCD_COLOR_BLACK);
			}
		}
	}

	if (Level == LCD_LEVEL_3) {

		//display Level 3 Screen

		LCD_Clear(0, LCD_COLOR_WHITE);
		HAL_Delay(1000);
		LCD_Clear(0, LCD_COLOR_BLUE);
		LCD_SetTextColor(LCD_COLOR_BLACK);
		LCD_SetFont(&Font16x24);

		LCD_DisplayChar(100, 140, 'L');
		LCD_DisplayChar(115, 140, 'e');
		LCD_DisplayChar(130, 140, 'v');
		LCD_DisplayChar(145, 140, 'e');
		LCD_DisplayChar(160, 140, 'l');

		LCD_DisplayChar(100, 160, 'T');
		LCD_DisplayChar(115, 160, 'h');
		LCD_DisplayChar(130, 160, 'r');
		LCD_DisplayChar(145, 160, 'e');
		LCD_DisplayChar(160, 160, 'e');

		HAL_Delay(2000);

		LCD_Clear(0, LCD_COLOR_WHITE);

		//give instructions

		uint16_t instr_length = 53;
		uint16_t x = 20;
		uint16_t y = 40;

		char instr[] =
				{ "Press Button When Circle Stops on Either Side of Line" };

		for (uint8_t i = 0; i < instr_length; i++) {

			if (x >= 230) {
				y += 50;
				x = 20;
			}

			LCD_DisplayChar(x, y, instr[i]);

			x += 15;
		}

		HAL_Delay(3000);

		//Three other function were created for each part of the shape movement and user input
		//see below
	}

}

/**
 * @brief Displays the first stage of Level 3 on the LCD screen.
 *
 * This function is part of a multi-stage display process for Level 3. It clears the screen, draws
 * a vertical line, and then animates a circle moving horizontally across the line. The animation
 * continues until the circle crosses a certain point on the screen.
 *
 * @note This function relies on `LCD_Clear`, `LCD_Draw_Vertical_Line`, and `LCD_Draw_Circle_Fill`
 *       to create the animation effect. It uses a while loop to update the position of the circle
 *       and redraw it after clearing the screen, creating the movement effect.
 *       The function is designed specifically for Level 3's first stage display and is tightly coupled
 *       with the LCD's resolution and specific display logic.
 *
 * Example usage:
 * @code
 *     LCD_Display_Lvl3_Stg1(); // Activates the first stage of Level 3 display
 * @endcode
 */

void LCD_Display_Lvl3_Stg1() {

	int16_t x = 120;
	int16_t y = 150;
	LCD_Clear(0, LCD_COLOR_WHITE);
	HAL_Delay(1000);

	LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10, (LCD_PIXEL_HEIGHT - 20),
	LCD_COLOR_BLACK);

	//keep drawing circle and the line until the circle is across the line
	//clears what's already on the screen at the beginning of every loop

	while (x < 145) {

		LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10,
				(LCD_PIXEL_HEIGHT - 20), LCD_COLOR_BLACK);
		LCD_Draw_Circle_Fill(x, y, 20, LCD_COLOR_BLACK);
		HAL_Delay(100);
		LCD_Clear(0, LCD_COLOR_WHITE);

		x++;
	}

	LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10, (LCD_PIXEL_HEIGHT - 20),
	LCD_COLOR_BLACK);
	LCD_Draw_Circle_Fill(x, y, 20, LCD_COLOR_BLACK);

}

/**
 * @brief Displays the second stage of Level 3 on the LCD screen.
 *
 * In this stage, a circle is animated to move horizontally across a vertical line in the opposite direction
 * to the first stage. The function draws a vertical line and a moving circle, updating its position in each
 * iteration of the while loop until it reaches a specific point.
 *
 * @note This function relies on `LCD_Clear`, `LCD_Draw_Vertical_Line`, and `LCD_Draw_Circle_Fill`
 *       to create the animation. It uses a loop to move the circle from right to left, crossing
 *       the vertical line. This function is specifically designed for the second stage of Level 3's display
 *       and depends on the LCD's resolution and specific display requirements for this level.
 *
 * Example usage:
 * @code
 *     LCD_Display_Lvl3_Stg2(); // Activates the second stage of Level 3 display
 * @endcode
 */

void LCD_Display_Lvl3_Stg2() {
	int16_t x = 145;
	int16_t y = 150;
	HAL_Delay(1000);

	LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10, (LCD_PIXEL_HEIGHT - 20),
	LCD_COLOR_BLACK);

	//keep drawing circle until the circle is across the line

	while (x > 95) {

		LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10,
				(LCD_PIXEL_HEIGHT - 20), LCD_COLOR_BLACK);
		LCD_Draw_Circle_Fill(x, y, 20, LCD_COLOR_BLACK);
		HAL_Delay(100);
		LCD_Clear(0, LCD_COLOR_WHITE);

		x--;
	}

	LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10, (LCD_PIXEL_HEIGHT - 20),
	LCD_COLOR_BLACK);
	LCD_Draw_Circle_Fill(x, y, 20, LCD_COLOR_BLACK);
}

/**
 * @brief Displays the third stage of Level 3 on the LCD screen.
 *
 * In this final stage of Level 3, a circle is animated to move horizontally across a vertical line,
 * similar to the first stage but in the opposite direction. The function draws a vertical line and a circle,
 * moving the circle from left to right until it crosses the line.
 *
 * @note This function uses `LCD_Clear`, `LCD_Draw_Vertical_Line`, and `LCD_Draw_Circle_Fill` to create
 *       the animation. The circle's position is updated in a loop until it moves across the predetermined point.
 *       This function is tailored for the third stage of Level 3's display, depending on specific display requirements
 *       and the LCD's resolution.
 *
 * Example usage:
 * @code
 *     LCD_Display_Lvl3_Stg3(); // Activates the third stage of Level 3 display
 * @endcode
 */
void LCD_Display_Lvl3_Stg3() {

	int16_t x = 95;
	int16_t y = 150;
	HAL_Delay(1000);

	LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10, (LCD_PIXEL_HEIGHT - 20),
	LCD_COLOR_BLACK);

	//keep drawing circle until the circle is across the line

	while (x < 145) {

		LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10,
				(LCD_PIXEL_HEIGHT - 20), LCD_COLOR_BLACK);
		LCD_Draw_Circle_Fill(x, y, 20, LCD_COLOR_BLACK);
		HAL_Delay(100);
		LCD_Clear(0, LCD_COLOR_WHITE);

		x++;
	}

	LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH / 2), 10, (LCD_PIXEL_HEIGHT - 20),
	LCD_COLOR_BLACK);
	LCD_Draw_Circle_Fill(x, y, 20, LCD_COLOR_BLACK);

}

/**
 * @brief Displays the given time in milliseconds on the LCD screen.
 *
 * This function converts the time in milliseconds to a string and displays it on the LCD.
 * It first converts the numeric time value to a character array, then calculates the length
 * of this array to display each character on the screen.
 *
 * @param Time Time in milliseconds to be displayed on the LCD.
 * @note The function uses `itoa` to convert the time from integer to string. It assumes
 *       that the provided `Time` value can be represented in up to 6 digits. The function
 *       also sets the text color and font before displaying the time and assumes these functions
 *       (`LCD_SetTextColor`, `LCD_SetFont`, `LCD_DisplayChar`) are implemented correctly.
 *
 * Example usage:
 * @code
 *     LCD_DisplayTime(123456); // Displays "Time: 123456" on the LCD
 * @endcode
 */

void LCD_DisplayTime(uint32_t Time) {

	char num[6] = { };

	itoa(Time, num, 10);

	uint16_t x = 110;
	uint16_t count = 0;

	for (uint8_t j = 0; j < 6; j++) {

		if (num[j] != '\0') {
			count++;
		}
	}

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);
	LCD_DisplayChar(100, 140, 'T');
	LCD_DisplayChar(115, 140, 'i');
	LCD_DisplayChar(125, 140, 'm');
	LCD_DisplayChar(140, 140, 'e');

	LCD_DisplayChar(150, 140, ':');

	for (uint8_t i = 0; i < count; i++) {

		LCD_DisplayChar(x + (i * 15), 160, num[i]);
	}
}

/*
 * Displays a Average Time in milliseconds on the screen
 *
 * display has maximum 6 nums of resolution
 * thus you can show up to 999.999 seconds
 *
 * only change here from the LCD_DisplayTime() is letters displayed on the screen
 *
 * Parameters
 *
 * uint32_t Time = time in milliseconds
 *
 *return void
 */

void LCD_Display_AVG_Time(uint32_t Time) {
	char num[6] = { };

	itoa(Time, num, 10);

	uint16_t x = 100;
	uint16_t count = 0;

	for (uint8_t j = 0; j < 6; j++) {

		if (num[j] != '\0') {
			count++;
		}
	}

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(40, 140, 'A');
	LCD_DisplayChar(55, 140, 'v');
	LCD_DisplayChar(70, 140, 'e');
	LCD_DisplayChar(85, 140, 'r');
	LCD_DisplayChar(100, 140, 'a');
	LCD_DisplayChar(115, 140, 'g');
	LCD_DisplayChar(130, 140, 'e');

	LCD_DisplayChar(145, 140, 'T');
	LCD_DisplayChar(160, 140, 'i');
	LCD_DisplayChar(175, 140, 'm');
	LCD_DisplayChar(190, 140, 'e');

	LCD_DisplayChar(200, 140, ':');

	for (uint8_t i = 0; i < count; i++) {

		LCD_DisplayChar(x + (i * 15), 170, num[i]);
	}
}

/**
 * @brief Displays the average time in milliseconds on the LCD screen.
 *
 * This function is designed to display the average time, converted from milliseconds to a string,
 * on the LCD. It calculates the number of characters in the time string and displays each character,
 * including the label "Average Time:" preceding the time value.
 *
 * @param Time Average time in milliseconds to be displayed.
 * @note The function uses `itoa` for integer to string conversion, assuming that the `Time` value
 *       can be represented in up to 6 digits. It sets the text color and font for display,
 *       and depends on the correct implementation of `LCD_SetTextColor`, `LCD_SetFont`, and `LCD_DisplayChar`.
 *       The function specifically formats the display to show the text "Average Time:" followed by the time value.
 *
 * Example usage:
 * @code
 *     LCD_Display_AVG_Time(654321); // Displays "Average Time: 654321" on the LCD
 * @endcode
 */

void LCD_Start_Screen() {

	LCD_Clear(0, LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	uint16_t instr_length = 29;
	uint16_t x = 20;
	uint16_t y = 40;

	char instr[] = { "Ready to Test Your Reflexes !" };

	for (uint8_t i = 0; i < instr_length; i++) {

		if (x >= 230) {
			y += 50;
			x = 20;
		}

		LCD_DisplayChar(x, y, instr[i]);

		x += 15;
	}

	HAL_Delay(3000);
}

/**
 * @brief Displays the end screen with final scores for different levels on the LCD.
 *
 * This function is designed to display the end screen of a game or application. It shows the final times
 * or scores for three levels. Each time value is converted from an integer to a string and then displayed
 * on the screen alongside the level labels.
 *
 * @param Tim1 Time or score for Level One.
 * @param Tim2 Time or score for Level Two.
 * @param Tim3 Time or score for Level Three.
 * @note The function uses `itoa` for integer to string conversion and assumes that each time value can
 *       be represented in up to 6 digits. It sets the text color and font for display, and depends on the
 *       correct implementation of `LCD_SetTextColor`, `LCD_SetFont`, and `LCD_DisplayChar`.
 *       The function specifically formats the display to show the text "Level One:", "Level Two:", and "Level Three:"
 *       followed by their respective time values.
 *
 * Example usage:
 * @code
 *     LCD_End_Screen(123, 456, 789); // Displays the final scores for three levels as "Level One: 123", "Level Two: 456", "Level Three: 789"
 * @endcode
 */

void LCD_End_Screen(uint32_t Tim1, uint32_t Tim2, uint32_t Tim3) {

	char num1[6] = { };

	itoa(Tim1, num1, 10);

	uint16_t count1 = 0;

	for (uint8_t j = 0; j < 6; j++) {

		if (num1[j] != '\0') {
			count1++;
		}
	}

	char num2[6] = { };

	itoa(Tim2, num2, 10);

	uint16_t count2 = 0;

	for (uint8_t j = 0; j < 6; j++) {

		if (num2[j] != '\0') {
			count2++;
		}
	}

	char num3[6] = { };

	itoa(Tim3, num3, 10);

	uint16_t count3 = 0;

	for (uint8_t j = 0; j < 6; j++) {

		if (num3[j] != '\0') {
			count3++;
		}
	}

	//display "Here are your final Scores !"

	LCD_Clear(0, LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	uint16_t instr_length = 28;
	uint16_t x = 20;
	uint16_t y = 40;

	char instr[] = { "Here are your final Scores !" };

	for (uint8_t i = 0; i < instr_length; i++) {

		if (x >= 230) {
			y += 50;
			x = 20;
		}

		LCD_DisplayChar(x, y, instr[i]);

		x += 15;
	}

	//displays each Level and :
	LCD_DisplayChar(10, 200, 'L');
	LCD_DisplayChar(25, 200, 'e');
	LCD_DisplayChar(40, 200, 'v');
	LCD_DisplayChar(55, 200, 'e');
	LCD_DisplayChar(70, 200, 'l');

	LCD_DisplayChar(85, 200, 'O');
	LCD_DisplayChar(100, 200, 'n');
	LCD_DisplayChar(115, 200, 'e');
	LCD_DisplayChar(130, 200, ':');

	LCD_DisplayChar(10, 230, 'L');
	LCD_DisplayChar(25, 230, 'e');
	LCD_DisplayChar(40, 230, 'v');
	LCD_DisplayChar(55, 230, 'e');
	LCD_DisplayChar(70, 230, 'l');

	LCD_DisplayChar(85, 230, 'T');
	LCD_DisplayChar(100, 230, 'w');
	LCD_DisplayChar(115, 230, 'o');
	LCD_DisplayChar(130, 230, ':');

	LCD_DisplayChar(10, 260, 'L');
	LCD_DisplayChar(25, 260, 'e');
	LCD_DisplayChar(40, 260, 'v');
	LCD_DisplayChar(55, 260, 'e');
	LCD_DisplayChar(70, 260, 'l');

	LCD_DisplayChar(85, 260, 'T');
	LCD_DisplayChar(100, 260, 'h');
	LCD_DisplayChar(115, 260, 'r');
	LCD_DisplayChar(130, 260, 'e');
	LCD_DisplayChar(145, 260, 'e');
	LCD_DisplayChar(160, 260, ':');

	// displays each time next the the appropriate level

	y = 200;

	x = 145;

	for (uint8_t i = 0; i < count1; i++) {

		LCD_DisplayChar(x + (i * 15), y, num1[i]);
	}

	y += 30;

	for (uint8_t i = 0; i < count2; i++) {

		LCD_DisplayChar(x + (i * 15), y, num2[i]);
	}

	y += 30;
	x = 175;

	for (uint8_t i = 0; i < count3; i++) {

		LCD_DisplayChar(x + (i * 15), y, num3[i]);
	}
	HAL_Delay(10000);
}

/*        APPLICATION SPECIFIC FUNCTION DEFINITIONS - PUT YOUR NEWLY CREATED FUNCTIONS HERE       */

/* Lower Level Functions for LTCD. 	MOTIFY ONLY WITH EXTREME CAUTION!!  */
static uint8_t Is_LCD_IO_Initialized = 0;

/**
 * @brief  Power on the LCD.
 * @param  None
 * @retval None
 */
void ili9341_Init(void) {
	/* Initialize ILI9341 low level bus layer ----------------------------------*/
	LCD_IO_Init();

	/* Configure LCD */
	ili9341_Write_Reg(0xCA);
	ili9341_Send_Data(0xC3);				//param 1
	ili9341_Send_Data(0x08);				//param 2
	ili9341_Send_Data(0x50);				//param 3
	ili9341_Write_Reg(LCD_POWERB); //CF
	ili9341_Send_Data(0x00);				//param 1
	ili9341_Send_Data(0xC1);				//param 2
	ili9341_Send_Data(0x30);				//param 3
	ili9341_Write_Reg(LCD_POWER_SEQ); //ED
	ili9341_Send_Data(0x64);
	ili9341_Send_Data(0x03);
	ili9341_Send_Data(0x12);
	ili9341_Send_Data(0x81);
	ili9341_Write_Reg(LCD_DTCA);
	ili9341_Send_Data(0x85);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x78);
	ili9341_Write_Reg(LCD_POWERA);
	ili9341_Send_Data(0x39);
	ili9341_Send_Data(0x2C);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x34);
	ili9341_Send_Data(0x02);
	ili9341_Write_Reg(LCD_PRC);
	ili9341_Send_Data(0x20);
	ili9341_Write_Reg(LCD_DTCB);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x00);
	ili9341_Write_Reg(LCD_FRMCTR1);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x1B);
	ili9341_Write_Reg(LCD_DFC);
	ili9341_Send_Data(0x0A);
	ili9341_Send_Data(0xA2);
	ili9341_Write_Reg(LCD_POWER1);
	ili9341_Send_Data(0x10);
	ili9341_Write_Reg(LCD_POWER2);
	ili9341_Send_Data(0x10);
	ili9341_Write_Reg(LCD_VCOM1);
	ili9341_Send_Data(0x45);
	ili9341_Send_Data(0x15);
	ili9341_Write_Reg(LCD_VCOM2);
	ili9341_Send_Data(0x90);
	ili9341_Write_Reg(LCD_MAC);
	ili9341_Send_Data(0xC8);
	ili9341_Write_Reg(LCD_3GAMMA_EN);
	ili9341_Send_Data(0x00);
	ili9341_Write_Reg(LCD_RGB_INTERFACE);
	ili9341_Send_Data(0xC2);
	ili9341_Write_Reg(LCD_DFC);
	ili9341_Send_Data(0x0A);
	ili9341_Send_Data(0xA7);
	ili9341_Send_Data(0x27);
	ili9341_Send_Data(0x04);

	/* Colomn address set */
	ili9341_Write_Reg(LCD_COLUMN_ADDR);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0xEF);

	/* Page address set */
	ili9341_Write_Reg(LCD_PAGE_ADDR);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x01);
	ili9341_Send_Data(0x3F);
	ili9341_Write_Reg(LCD_INTERFACE);
	ili9341_Send_Data(0x01);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x06);

	ili9341_Write_Reg(LCD_GRAM);
	LCD_Delay(200);

	ili9341_Write_Reg(LCD_GAMMA);
	ili9341_Send_Data(0x01);

	ili9341_Write_Reg(LCD_PGAMMA);
	ili9341_Send_Data(0x0F);
	ili9341_Send_Data(0x29);
	ili9341_Send_Data(0x24);
	ili9341_Send_Data(0x0C);
	ili9341_Send_Data(0x0E);
	ili9341_Send_Data(0x09);
	ili9341_Send_Data(0x4E);
	ili9341_Send_Data(0x78);
	ili9341_Send_Data(0x3C);
	ili9341_Send_Data(0x09);
	ili9341_Send_Data(0x13);
	ili9341_Send_Data(0x05);
	ili9341_Send_Data(0x17);
	ili9341_Send_Data(0x11);
	ili9341_Send_Data(0x00);
	ili9341_Write_Reg(LCD_NGAMMA);
	ili9341_Send_Data(0x00);
	ili9341_Send_Data(0x16);
	ili9341_Send_Data(0x1B);
	ili9341_Send_Data(0x04);
	ili9341_Send_Data(0x11);
	ili9341_Send_Data(0x07);
	ili9341_Send_Data(0x31);
	ili9341_Send_Data(0x33);
	ili9341_Send_Data(0x42);
	ili9341_Send_Data(0x05);
	ili9341_Send_Data(0x0C);
	ili9341_Send_Data(0x0A);
	ili9341_Send_Data(0x28);
	ili9341_Send_Data(0x2F);
	ili9341_Send_Data(0x0F);

	ili9341_Write_Reg(LCD_SLEEP_OUT);
	LCD_Delay(200);
	ili9341_Write_Reg(LCD_DISPLAY_ON);
	/* GRAM start writing */
	ili9341_Write_Reg(LCD_GRAM);
}

/**
 * @brief  Enables the Display.
 * @param  None
 * @retval None
 */
void ili9341_DisplayOn(void) {
	/* Display On */
	ili9341_Write_Reg(LCD_DISPLAY_ON);
}

/**
 * @brief  Disables the Display.
 * @param  None
 * @retval None
 */
void ili9341_DisplayOff(void) {
	/* Display Off */
	ili9341_Write_Reg(LCD_DISPLAY_OFF);
}

/**
 * @brief  Writes  to the selected LCD register.
 * @param  LCD_Reg: address of the selected register.
 * @retval None
 */
void ili9341_Write_Reg(uint8_t LCD_Reg) {
	LCD_IO_WriteReg(LCD_Reg);
}

/**
 * @brief  Writes data to the selected LCD register.
 * @param  LCD_Reg: address of the selected register.
 * @retval None
 */
void ili9341_Send_Data(uint16_t RegValue) {
	LCD_IO_WriteData(RegValue);
}

/**
 * @brief  Reads the selected LCD Register.
 * @param  RegValue: Address of the register to read
 * @param  ReadSize: Number of bytes to read
 * @retval LCD Register Value.
 */
uint32_t ili9341_ReadData(uint16_t RegValue, uint8_t ReadSize) {
	/* Read a max of 4 bytes */
	return (LCD_IO_ReadData(RegValue, ReadSize));
}

/******************************* SPI Routines *********************************/

/**
 * @brief  SPI Bus initialization
 */
static void SPI_Init(void) {
	if (HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET) {
		/* SPI configuration -----------------------------------------------------*/
		SpiHandle.Instance = DISCOVERY_SPI;
		/* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz)
		 to verify these constraints:
		 - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
		 - l3gd20 SPI interface max baudrate is 10MHz for write/read
		 - PCLK2 frequency is set to 90 MHz
		 */
		SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

		/* On STM32F429I-Discovery, LCD ID cannot be read then keep a common configuration */
		/* for LCD and GYRO (SPI_DIRECTION_2LINES) */
		/* Note: To read a register a LCD, SPI_DIRECTION_1LINE should be set */
		SpiHandle.Init.Direction = SPI_DIRECTION_2LINES;
		SpiHandle.Init.CLKPhase = SPI_PHASE_1EDGE;
		SpiHandle.Init.CLKPolarity = SPI_POLARITY_LOW;
		SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
		SpiHandle.Init.CRCPolynomial = 7;
		SpiHandle.Init.DataSize = SPI_DATASIZE_8BIT;
		SpiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
		SpiHandle.Init.NSS = SPI_NSS_SOFT;
		SpiHandle.Init.TIMode = SPI_TIMODE_DISABLED;
		SpiHandle.Init.Mode = SPI_MODE_MASTER;

		SPI_MspInit(&SpiHandle);
		HAL_SPI_Init(&SpiHandle);
	}
}

/**
 * @brief  Reads 4 bytes from device.
 * @param  ReadSize: Number of bytes to read (max 4 bytes)
 * @retval Value read on the SPI
 */
static uint32_t SPI_Read(uint8_t ReadSize) {
	HAL_StatusTypeDef status = HAL_OK;
	uint32_t readvalue;

	status = HAL_SPI_Receive(&SpiHandle, (uint8_t*) &readvalue, ReadSize,
			SpiTimeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Re-Initialize the BUS */
		SPI_Error();
	}

	return readvalue;
}

/**
 * @brief  Writes a byte to device.
 * @param  Value: value to be written
 */
static void SPI_Write(uint16_t Value) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_SPI_Transmit(&SpiHandle, (uint8_t*) &Value, 1, SpiTimeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Re-Initialize the BUS */
		SPI_Error();
	}
}

/**
 * @brief  SPI error treatment function.
 */
static void SPI_Error(void) {
	/* De-initialize the SPI communication BUS */
	HAL_SPI_DeInit(&SpiHandle);

	/* Re- Initialize the SPI communication BUS */
	SPI_Init();
}

/**
 * @brief  SPI MSP Init.
 * @param  hspi: SPI handle
 */
static void SPI_MspInit(SPI_HandleTypeDef *hspi) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI clock */
	DISCOVERY_SPI_CLK_ENABLE();

	/* Enable DISCOVERY_SPI GPIO clock */
	DISCOVERY_SPI_GPIO_CLK_ENABLE();

	/* configure SPI SCK, MOSI and MISO */
	GPIO_InitStructure.Pin = (DISCOVERY_SPI_SCK_PIN | DISCOVERY_SPI_MOSI_PIN
			| DISCOVERY_SPI_MISO_PIN);
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = DISCOVERY_SPI_AF;
	HAL_GPIO_Init(DISCOVERY_SPI_GPIO_PORT, &GPIO_InitStructure);
}

/********************************* LINK LCD ***********************************/

/**
 * @brief  Configures the LCD_SPI interface.
 */
void LCD_IO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	if (Is_LCD_IO_Initialized == 0) {
		Is_LCD_IO_Initialized = 1;

		/* Configure in Output Push-Pull mode */
		LCD_WRX_GPIO_CLK_ENABLE();
		GPIO_InitStructure.Pin = LCD_WRX_PIN;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);

		LCD_RDX_GPIO_CLK_ENABLE();
		GPIO_InitStructure.Pin = LCD_RDX_PIN;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(LCD_RDX_GPIO_PORT, &GPIO_InitStructure);

		/* Configure the LCD Control pins ----------------------------------------*/
		LCD_NCS_GPIO_CLK_ENABLE();

		/* Configure NCS in Output Push-Pull mode */
		GPIO_InitStructure.Pin = LCD_NCS_PIN;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
		HAL_GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);

		/* Set or Reset the control line */
		LCD_CS_LOW();
		LCD_CS_HIGH();

		SPI_Init();
	}
}

/**
 * @brief  Writes register value.
 */
void LCD_IO_WriteData(uint16_t RegValue) {
	/* Set WRX to send data */
	LCD_WRX_HIGH();

	/* Reset LCD control line(/CS) and Send data */
	LCD_CS_LOW();
	SPI_Write(RegValue);

	/* Deselect: Chip Select high */
	LCD_CS_HIGH();
}

/**
 * @brief  Writes register address.
 */
void LCD_IO_WriteReg(uint8_t Reg) {
	/* Reset WRX to send command */
	LCD_WRX_LOW();

	/* Reset LCD control line(/CS) and Send command */
	LCD_CS_LOW();
	SPI_Write(Reg);

	/* Deselect: Chip Select high */
	LCD_CS_HIGH();
}

/**
 * @brief  Reads register value.
 * @param  RegValue Address of the register to read
 * @param  ReadSize Number of bytes to read
 * @retval Content of the register value
 */
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize) {
	uint32_t readvalue = 0;

	/* Select: Chip Select low */
	LCD_CS_LOW();

	/* Reset WRX to send command */
	LCD_WRX_LOW();

	SPI_Write(RegValue);

	readvalue = SPI_Read(ReadSize);

	/* Set WRX to send data */
	LCD_WRX_HIGH();

	/* Deselect: Chip Select high */
	LCD_CS_HIGH();

	return readvalue;
}

/**
 * @brief  Wait for loop in ms.
 * @param  Delay in ms.
 */
void LCD_Delay(uint32_t Delay) {
	HAL_Delay(Delay);
}

/*       Generated HAL Stuff      */

//   * @brief LTDC Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_LTDC_Init(void)
// {
//   /* USER CODE BEGIN LTDC_Init 0 */
//   /* USER CODE END LTDC_Init 0 */
//   LTDC_LayerCfgTypeDef pLayerCfg = {0};
//   LTDC_LayerCfgTypeDef pLayerCfg1 = {0};
//   /* USER CODE BEGIN LTDC_Init 1 */
//   /* USER CODE END LTDC_Init 1 */
//   hltdc.Instance = LTDC;
//   hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
//   hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
//   hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
//   hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
//   hltdc.Init.HorizontalSync = 7;
//   hltdc.Init.VerticalSync = 3;
//   hltdc.Init.AccumulatedHBP = 14;
//   hltdc.Init.AccumulatedVBP = 5;
//   hltdc.Init.AccumulatedActiveW = 654;
//   hltdc.Init.AccumulatedActiveH = 485;
//   hltdc.Init.TotalWidth = 660;
//   hltdc.Init.TotalHeigh = 487;
//   hltdc.Init.Backcolor.Blue = 0;
//   hltdc.Init.Backcolor.Green = 0;
//   hltdc.Init.Backcolor.Red = 0;
//   if (HAL_LTDC_Init(&hltdc) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   pLayerCfg.WindowX0 = 0;
//   pLayerCfg.WindowX1 = 0;
//   pLayerCfg.WindowY0 = 0;
//   pLayerCfg.WindowY1 = 0;
//   pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
//   pLayerCfg.Alpha = 0;
//   pLayerCfg.Alpha0 = 0;
//   pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
//   pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
//   pLayerCfg.FBStartAdress = 0;
//   pLayerCfg.ImageWidth = 0;
//   pLayerCfg.ImageHeight = 0;
//   pLayerCfg.Backcolor.Blue = 0;
//   pLayerCfg.Backcolor.Green = 0;
//   pLayerCfg.Backcolor.Red = 0;
//   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   pLayerCfg1.WindowX0 = 0;
//   pLayerCfg1.WindowX1 = 0;
//   pLayerCfg1.WindowY0 = 0;
//   pLayerCfg1.WindowY1 = 0;
//   pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
//   pLayerCfg1.Alpha = 0;
//   pLayerCfg1.Alpha0 = 0;
//   pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
//   pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
//   pLayerCfg1.FBStartAdress = 0;
//   pLayerCfg1.ImageWidth = 0;
//   pLayerCfg1.ImageHeight = 0;
//   pLayerCfg1.Backcolor.Blue = 0;
//   pLayerCfg1.Backcolor.Green = 0;
//   pLayerCfg1.Backcolor.Red = 0;
//   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN LTDC_Init 2 */
//   /* USER CODE END LTDC_Init 2 */
// }
/**
 * @brief SPI5 Initialization Function
 * @param None
 * @retval None
 */
// static void MX_SPI5_Init(void)
// {
//   /* USER CODE BEGIN SPI5_Init 0 */
//   /* USER CODE END SPI5_Init 0 */
//   /* USER CODE BEGIN SPI5_Init 1 */
//   /* USER CODE END SPI5_Init 1 */
//   /* SPI5 parameter configuration*/
//   hspi5.Instance = SPI5;
//   hspi5.Init.Mode = SPI_MODE_MASTER;
//   hspi5.Init.Direction = SPI_DIRECTION_2LINES;
//   hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
//   hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
//   hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
//   hspi5.Init.NSS = SPI_NSS_SOFT;
//   hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//   hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
//   hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
//   hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//   hspi5.Init.CRCPolynomial = 10;
//   if (HAL_SPI_Init(&hspi5) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN SPI5_Init 2 */
//   /* USER CODE END SPI5_Init 2 */
// }
