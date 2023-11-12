/*
 * sh1106.c
 *
 *  Created on: Nov 3, 2023
 *      Author: Kikkiu
 */

#include "sh1106.h"
#include "stm32f1xx_hal.h"
#include "string.h"
#include "stdlib.h"

extern I2C_HandleTypeDef hi2c1;

uint8_t buf[2];
uint8_t SH1106_Buffer[OLED_HEIGHT/8][OLED_WIDTH];
uint8_t ram_page = 0;


/*//////////////////////////// DISPLAY FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void SH1106_WriteData(uint8_t *data, size_t size)
{
	HAL_I2C_Mem_Write(&hi2c1, DISPLAY_ADDR, 0x40, 1, data, size, HAL_MAX_DELAY);
}


void SH1106_WriteCmdSingle(uint8_t cmd)
{
	HAL_I2C_Mem_Write(&hi2c1, DISPLAY_ADDR, 0x00, 1, &cmd, 1, HAL_MAX_DELAY);
}


void SH1106_WriteCmdDouble(uint8_t cmd, uint8_t data)
{
	buf[0] = cmd;
	buf[1] = data;
	HAL_I2C_Mem_Write(&hi2c1, DISPLAY_ADDR, 0x00, 1, buf, 2, HAL_MAX_DELAY);
}


void SH1106_UpdateScreen(void)
{
	for (uint8_t i = 0; i < OLED_HEIGHT / 8; i++)
	{
		SH1106_WriteCmdSingle(0xB0 + i);	// page select
		SH1106_WriteCmdSingle(0x00);		// reset horizontal position (lower bits)
		SH1106_WriteCmdSingle(0x10);		// reset horizontal position (upper bits)
		SH1106_WriteData(SH1106_Buffer[i], OLED_WIDTH);	// write the entire page
	}
}


void SH1106_ClearScreen(void)
{
	for (uint32_t i = 0; i < OLED_HEIGHT / 8; i++)
		memset(SH1106_Buffer[i], 0, OLED_WIDTH);
	SH1106_UpdateScreen();
}


void SH1106_DisplayOFF(void)
{
	SH1106_WriteCmdSingle(0xAE);
}


void SH1106_DisplayON(void)
{
	SH1106_WriteCmdSingle(0xAF);
}


/*//////////////////////////// GRAPHICS FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
uint8_t SH1106_DrawPixel(uint8_t x, uint8_t y, SH1106_COLOR color)
{
	x += 2;
	if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
		return 0;
	if (color == BLUE)
		SH1106_Buffer[y / 8][x] |= 0x01 << (y % 8);
	else
		SH1106_Buffer[y / 8][x] &= ~(0x01 << (y % 8));
	return 1;
}


uint8_t SH1106_DrawByte(uint8_t x, uint8_t y, uint8_t byte, SH1106_COLOR color)
{
	if (x < 0 || y < 0 || x > OLED_WIDTH || y > OLED_HEIGHT || y + 8 > OLED_HEIGHT)
		return 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		if (byte >> i & 0x01)
			SH1106_DrawPixel(x, y + i, color);
	}

	return 1;
}


uint8_t SH1106_DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color)
{
	uint8_t pixel_x = x1;
	uint8_t pixel_y = y1;

	if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 > OLED_WIDTH || x2 > OLED_WIDTH || y1 > OLED_HEIGHT || y2 > OLED_HEIGHT)
		return 0;

	for (uint32_t i = 0; i <= abs(y2 - y1); i++)
	{
		for (uint32_t j = 0; j <= abs(x2- x1); j++)
		{
			SH1106_DrawPixel(pixel_x, pixel_y, color);
			pixel_x++;
		}

		pixel_y++;
		pixel_x = x1;
	}

	SH1106_UpdateScreen();
	return 1;
}


uint8_t SH1106_DrawHollowRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, SH1106_COLOR color)
{
	if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 > OLED_WIDTH || x2 > OLED_WIDTH || y1 > OLED_HEIGHT || y2 > OLED_HEIGHT)
		return 0;

	for (uint32_t i = 0; i < thickness; i++)
	{
		SH1106_DrawLine(x1, y1 + i, x2, y1 + i, 1, 0, color);
		SH1106_DrawLine(x1 + i, y1, x1 + i, y2, 1, 0, color);
		SH1106_DrawLine(x1, y2 - i, x2, y2 - i, 1, 0, color);
		SH1106_DrawLine(x2 - i, y1, x2 - i, y2, 1, 0, color);
	}

	SH1106_UpdateScreen();

	return 1;
}


uint8_t SH1106_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, uint8_t update_screen, SH1106_COLOR color)
{
	int8_t dx = abs(x2 - x1);
	int8_t dy = abs(y2 - y1);
	int8_t incx = (x1 < x2) ? 1 : -1;
	int8_t incy = (y1 < y2) ? 1 : -1;

	int8_t x = x1;
	int8_t y = y1;

	int8_t pdx, pdy, es, el, err;

	if (dx > dy)
	{
		pdx = incx;
		pdy = 0;
		es = dy;
		el = dx;
	} else
	{
		pdx = 0;
		pdy = incy;
		es = dx;
		el = dy;
	}

	err = el / 2;

	SH1106_DrawPixel(x1, y1, color);

	for (int8_t t = 0; t < el; t++)
	{
		err -= es;

		if (err < 0) {
			err += el;
			x += incx;
			y += incy;
		} else {
			x += pdx;
			y += pdy;
		}

		for (int8_t i = 0; i < thickness; i++)
		{
			if (x >= 0 && x <= 127 && y >= 0 && y <= 63)
			{
				SH1106_DrawPixel(x, y, color);

				for (int8_t j = 1; j <= thickness / 2; j++)
				{
					if (x + j * pdy >= 0 && x + j * pdy <= OLED_WIDTH && y - j * pdx >= 0 && y - j * pdx <= OLED_HEIGHT)
						SH1106_DrawPixel(x + j * pdy, y - j * pdx, color);

					if (x - j * pdy >= 0 && x - j * pdy <= OLED_WIDTH && y + j * pdx >= 0 && y + j * pdx <= OLED_HEIGHT)
						SH1106_DrawPixel(x - j * pdy, y + j * pdx, color);
				}
			}
		}
	}

	if (update_screen)
		SH1106_UpdateScreen();
	return 1;
}


void SH1106_WriteCharsPageMode(char *chars, size_t size, FONT_INFO font, int8_t start_page)
{
	uint32_t horizontal_pos = 2;	// starts from 2, because first 2 columns aren't visible
	uint8_t separation = font.height / 2;
	if (separation == 0)
		separation = 1;

	uint8_t space_character_width = separation * 2;
	uint32_t char_position = 0;
	uint32_t char_width = 0;

	ram_page = (start_page == -1) ? font.height - 1 : start_page;	// font.height is in pages, no need to convert

	// estimation of text width
	if (font.descriptors[chars[0] - font.start_char].width * size >= OLED_WIDTH)
		separation = 0;

	for (uint32_t k = 0; k < size; k++)
	{
		char_position = font.descriptors[chars[k] - font.start_char].offset;
		char_width = font.descriptors[chars[k] - font.start_char].width;

		if (chars[k] == ' ')
		{
			// index would go past the end of the array, so stop the execution
			if (horizontal_pos + space_character_width >= OLED_WIDTH)
				break;

			for (uint32_t j = 0; j < font.height; j++)
			{
				for (uint32_t i = 0; i < space_character_width; i++)
					SH1106_Buffer[ram_page - j][i + horizontal_pos] = 0x00;
			}

			horizontal_pos += space_character_width;
			continue;
		}

		// index would go past the end of the array, so stop the execution
		if (horizontal_pos + char_width + (k == size - 1) ? 0 : separation >= OLED_WIDTH)
			break;

		for (uint32_t j = 0; j < font.height; j++)
		{
			for (uint32_t i = 0; i < char_width; i++)
				SH1106_Buffer[ram_page - j][i + horizontal_pos] = font.bitmaps[i * font.height + j + char_position];

			// if it's the last character, it's useless putting a separator
			if (k == size - 1)
				continue;

			for (uint32_t i = 0; i < separation; i++)
				SH1106_Buffer[ram_page - j][i + horizontal_pos + char_width] = 0x00;
		}
		if (k == size - 1)
			separation = 0;	// if it's the last character, there's no separator
		horizontal_pos += char_width + separation * 2;
	}

	SH1106_UpdateScreen();
}


void SH1106_WriteChars(uint8_t x, uint8_t y, char *chars, size_t size, FONT_INFO font)
{
	uint8_t cur_x = x;	// SH1106_DrawPixel() already increments x by 2
	uint8_t cur_y = y;
	uint8_t separation = font.height / 2;
	uint8_t space_character_width = separation * 2;
	uint32_t char_position = 0;
	uint32_t char_width = 0;

	// estimation of text width
	if (font.descriptors[chars[0] - font.start_char].width * size >= OLED_WIDTH)
		separation = 0;

	for (uint32_t k = 0; k < size; k++)
	{
		if (chars[k] == ' ')
		{
			if (cur_x + space_character_width > OLED_WIDTH)
				break;

			for (uint32_t i = 0; i < space_character_width; i++)
			{
				for (uint32_t j = 0; j < font.height; j++)
				{
					SH1106_DrawPixel(cur_x, cur_y, BLACK);
					cur_y++;
				}
				cur_x++;
				cur_y = y;
			}

			continue;
		}

		char_position = font.descriptors[chars[k] - font.start_char].offset;
		char_width = font.descriptors[chars[k] - font.start_char].width;

		// checks whether the character will go out of bounds
		if (cur_x + char_width + (k == size - 1) ? 0 : separation > OLED_WIDTH)
			break;

		for (uint32_t i = 0; i < char_width; i++)
		{
			for (uint32_t j = 0; j < font.height; j++)
			{
				uint8_t col_page_data = font.bitmaps[char_position + i * font.height + font.height - 1 - j];
				for (uint32_t h = 0; h < 8; h++)
				{
					if ((col_page_data >> h & 0x01) == 1)
						SH1106_DrawPixel(cur_x, cur_y, BLUE);
					cur_y++;
				}
			}
			cur_x++;
			cur_y = y;
		}

		// if it's the last character, it's useless putting a separator
		if (k == size - 1)
			continue;

		for (uint32_t i = 0; i < separation; i++)
		{
			for (uint32_t j = 0; j < font.height * 8; j++)
			{
				SH1106_DrawPixel(cur_x, cur_y, BLACK);
				cur_y++;
			}
			cur_y = y;
			cur_x++;
		}
	}

	SH1106_UpdateScreen();
}


uint8_t SH1106_DrawBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
	if (x < 0 || y < 0 || x + width > OLED_WIDTH || y + height > OLED_HEIGHT)
		return 0;

	for (uint32_t i = 0; i < height / 8; i++)
	{
		for (uint32_t j = 0; j < width; j++)
			SH1106_DrawByte(x + j, y + i * 8, bitmap[j + width * i], BLUE);
	}

	SH1106_UpdateScreen();
	return 1;
}


void SH1106_Init(void)
{
	SH1106_WriteCmdSingle(0xAE);		// display off
	SH1106_WriteCmdDouble(0xA8, 0x3F);	// multiplex ratio set to 64
	SH1106_WriteCmdDouble(0xD3, 0x00);	// line offset from top of the display
	SH1106_WriteCmdSingle(0x40 | 0x00);	// display start line (column)
	SH1106_WriteCmdSingle(0xA1);		// segment re-map (x coordinate, A0 or A1)
	SH1106_WriteCmdSingle(0xC8);		// output scan direction (y coordinate, left to right, C0 or C8)
	SH1106_WriteCmdDouble(0xDA, 0x12);	// sequential COM pin configuration
	uint8_t discharge = 0x00;
	uint8_t precharge = 0x00;
	SH1106_WriteCmdDouble(0x22, discharge | (precharge << 4));	// charge pump configuration
	SH1106_WriteCmdDouble(0x81, 0x0F);	// contrast control. from 00 to FF
	SH1106_WriteCmdSingle(0xA4);		// enables RAM content to be displayed
	SH1106_WriteCmdSingle(0x32);		// charge pump voltage
	SH1106_WriteCmdSingle(0xA6);		// non-inverted display
	SH1106_WriteCmdDouble(0xD5, 0xF0);	// display clock frequency (max)
	SH1106_WriteCmdSingle(0xAF);		// display on

	HAL_Delay(250);
	SH1106_ClearScreen();
}
