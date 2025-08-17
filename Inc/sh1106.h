/*
 * sh1106.h
 *
 *  Created on: Nov 3, 2023
 *      Author: Kikkiu
 */

#ifndef INC_SH1106_H_
#define INC_SH1106_H_

#include "stdint.h"
#include "stddef.h"
#include "font.h"
#include "string.h"

#include "stm32f1xx_hal.h"

#define SCREEN_I2C hi2c1
#define I2C_DELAY 100

#define DISPLAY_ADDR 0x3C << 1
#define OLED_WIDTH 128		// SH1106 controller has a 132 byte wide RAM but only 128 pixels. if you're using SD1306, put 128
#define OLED_HEIGHT 64
#define BRIGHTNESS 0x10		// brightness (contrast): 0x00 -- 0xFF; 0x3C is medium brightness

#define OLED_BUFFER_SIZE OLED_WIDTH * OLED_HEIGHT / 8
#if OLED_WIDTH == 132
#define WIDE_RAM_COMPENSATION 2	// pixels are shifted by 2 to the right in SH1106
#elif OLED_WIDTH == 128
#define WIDE_RAM_COMPENSATION 0
#endif

typedef uint8_t bool;
#define true 1
#define false 0

typedef enum
{
	BLACK = 0x00,
	BLUE = 0x01
} SH1106_COLOR;

void SH1106_Init(void);
uint8_t SH1106_DrawPixel(bool x, bool y, SH1106_COLOR color);
uint8_t SH1106_DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color);
uint8_t SH1106_DrawHollowRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, SH1106_COLOR color);
uint8_t SH1106_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, SH1106_COLOR color);
void SH1106_ScrollDown(uint8_t scroll_px);
void SH1106_ScrollUp(uint8_t scroll_px);
uint8_t SH1106_DrawBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
uint8_t SH1106_WriteChars(uint8_t x, uint8_t y, char *chars, size_t size, FONT_INFO font);
uint32_t SH1106_GetTextWidth(char *chars, size_t size, FONT_INFO font);
void leftPadding(char *dest, char* src, uint8_t desired_size, char fill_character);
void SH1106_FillScreen(uint8_t color);
void SH1106_ClearScreen(void);
void SH1106_DisplayOFF(void);
void SH1106_DisplayON(void);
void SH1106_SetBrightness(uint8_t brightness);
void SH1106_EnterSleepMode(void);
void SH1106_ExitSleepMode(void);
void SH1106_UpdateScreen(void);

#endif /* INC_SH1106_H_ */
