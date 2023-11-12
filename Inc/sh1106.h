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

#define DISPLAY_ADDR 0x78	// 0x3C << 1
#define OLED_WIDTH 132		// screen is 128px wide but RAM has 132 columns
#define OLED_HEIGHT 64
#define OLED_BUFFER_SIZE OLED_WIDTH * OLED_HEIGHT / 8

typedef enum
{
	BLACK = 0x00,
	BLUE = 0x01
} SH1106_COLOR;

void SH1106_Init(void);
uint8_t SH1106_DrawPixel(uint8_t x, uint8_t y, SH1106_COLOR color);
uint8_t SH1106_DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color);
uint8_t SH1106_DrawHollowRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, SH1106_COLOR color);
uint8_t SH1106_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, uint8_t update_screen, SH1106_COLOR color);
uint8_t SH1106_DrawBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void SH1106_WriteCharsPageMode(char *chars, size_t size, FONT_INFO font, int8_t start_page);
void SH1106_WriteChars(uint8_t x, uint8_t y, char *chars, size_t size, FONT_INFO font);
void SH1106_ClearScreen(void);
void SH1106_DisplayOFF(void);
void SH1106_DisplayON(void);

#endif /* INC_SH1106_H_ */
