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

#define DISPLAY_ADDR 0x78	// 0x3C << 1
#define OLED_WIDTH 132		// screen is 128 pixels wide but RAM has 132 bytes
#define OLED_HEIGHT 64
#define OLED_BUFFER_SIZE OLED_WIDTH * OLED_HEIGHT / 8

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
uint8_t SH1106_DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color, bool erase_last_write, bool update_screen);
uint8_t SH1106_DrawHollowRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, SH1106_COLOR color, bool erase_last_write);
uint8_t SH1106_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, bool update_screen, SH1106_COLOR color, bool erase_last_write);
uint8_t SH1106_DrawBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool erase_last_write);
uint8_t SH1106_WriteChars(uint8_t x, uint8_t y, char *chars, size_t size, FONT_INFO font, bool erase_last_write);
void SH1106_FillScreen(uint8_t color);
void SH1106_ClearScreen(void);
void SH1106_DisplayOFF(void);
void SH1106_DisplayON(void);
void SH1106_EnterSleepMode(void);
void SH1106_ExitSleepMode(void);
void SH1106_WriteBufferChanges(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y, bool erase_last_write);
void SH1106_UpdateEntireFrame(void);
void SH1106_SetScreenAutoUpdate(bool);
void SH1106_SetHighSpeedUpdate(bool);

#endif /* INC_SH1106_H_ */
