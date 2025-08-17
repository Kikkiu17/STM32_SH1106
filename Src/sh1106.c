/*
 * sh1106.c
 *
 *  Created on: Nov 3, 2023
 *      Author: Kikkiu
 */

#include "sh1106.h"
#include "stdlib.h"
#include <string.h>
#include "math.h"

uint8_t SH1106_Buffer[OLED_HEIGHT/8][OLED_WIDTH];

extern I2C_HandleTypeDef SCREEN_I2C;

/*//////////////////////////// DISPLAY FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void SH1106_WriteData(uint8_t *data, size_t size)
{
	HAL_I2C_Mem_Write(&SCREEN_I2C, DISPLAY_ADDR, 0x40, 1, data, size, I2C_DELAY);
}


void SH1106_WriteCmdSingle(uint8_t cmd)
{
	HAL_I2C_Mem_Write(&SCREEN_I2C, DISPLAY_ADDR, 0x00, 1, &cmd, 1, I2C_DELAY);
}


void SH1106_WriteCmdDouble(uint8_t cmd, uint8_t data)
{
	uint8_t buf[2];
	buf[0] = cmd;
	buf[1] = data;
	HAL_I2C_Mem_Write(&SCREEN_I2C, DISPLAY_ADDR, 0x00, 1, buf, 2, I2C_DELAY);
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
}


void SH1106_DisplayOFF(void)
{
	SH1106_WriteCmdSingle(0xAE);
}


void SH1106_DisplayON(void)
{
	SH1106_WriteCmdSingle(0xAF);
}


void SH1106_SetBrightness(uint8_t brightness)
{
	if (brightness >= 0 && brightness <= 255)
		SH1106_WriteCmdDouble(0x81, brightness);
}


void SH1106_EnterSleepMode(void)
{
	SH1106_DisplayOFF();
	SH1106_WriteCmdDouble(0xAD, 0x8A);
}


void SH1106_ExitSleepMode(void)
{
	SH1106_DisplayOFF();	// panel must be off before issuing 0xAD command
	SH1106_WriteCmdDouble(0xAD, 0x8B);
	SH1106_DisplayON();
}


/*//////////////////////////// GRAPHICS FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void SH1106_FillScreen(uint8_t color)
{
	SH1106_DrawRect(0, 0, 128, 64, color);
}


// needs if (enable_screen_update) SH1106_WriteBufferChanges() after
uint8_t SH1106_DrawPixel(uint8_t x, uint8_t y, SH1106_COLOR color)
{
	x += WIDE_RAM_COMPENSATION;
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
	if (x < 0 || y < 0)
		return 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		if (byte >> i & 0x01)
		{
			if (x > OLED_WIDTH || y + i > OLED_HEIGHT)
				break;

			SH1106_DrawPixel(x, y + i, color);
		}
	}

	return 1;
}


uint8_t SH1106_DrawRect(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, SH1106_COLOR color)
{
	if (x1 < 0 || width <= 0 || y1 < 0 || height <= 0)
		return 0;

	if (x1 > OLED_WIDTH || x1 + width > OLED_WIDTH || y1 > OLED_HEIGHT || y1 + height > OLED_HEIGHT)
		return 0;

	uint8_t pixel_x = x1;
	uint8_t pixel_y = y1;

	for (uint32_t i = 0; i < height; i++)
	{
		for (uint32_t j = 0; j < width; j++)
		{
			SH1106_DrawPixel(pixel_x, pixel_y, color);
			pixel_x++;
		}

		pixel_y++;
		pixel_x = x1;
	}

	return 1;
}


uint8_t SH1106_DrawHollowRect(uint8_t x1, uint8_t y1, uint8_t width, uint8_t height, uint8_t thickness, SH1106_COLOR color)
{
	if (x1 < 0 || width <= 0 || y1 < 0 || height <= 0 || thickness <= 0)
		return 0;

	if (x1 > OLED_WIDTH || x1 + width > OLED_WIDTH || y1 > OLED_HEIGHT || y1 + height > OLED_HEIGHT)
		return 0;

	uint8_t x2 = x1 + width - 1;
	uint8_t y2 = y1 + height - 1;

	for (uint32_t i = 0; i < thickness; i++)
	{
		SH1106_DrawLine(x1, y1 + i, x2, y1 + i, 1, color);
		SH1106_DrawLine(x1 + i, y1, x1 + i, y2, 1, color);
		SH1106_DrawLine(x1, y2 - i, x2, y2 - i, 1, color);
		SH1106_DrawLine(x2 - i, y1, x2 - i, y2, 1, color);
	}

	return 1;
}


uint8_t SH1106_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t thickness, SH1106_COLOR color)
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

		if (err < 0)
		{
			err += el;
			x += incx;
			y += incy;
		}
		else
		{
			x += pdx;
			y += pdy;
		}

		for (int8_t i = 0; i < thickness; i++)
		{
			if (x >= 0 && x < 128 && y >= 0 && y < 64)
			{
				SH1106_DrawPixel(x, y, color);

				for (int8_t j = 1; j <= thickness / 2; j++)
				{
					if (x + j * pdy >= 0 && x + j * pdy < OLED_WIDTH && y - j * pdx >= 0 && y - j * pdx < OLED_HEIGHT)
						SH1106_DrawPixel(x + j * pdy, y - j * pdx, color);

					if (x - j * pdy >= 0 && x - j * pdy < OLED_WIDTH && y + j * pdx >= 0 && y + j * pdx < OLED_HEIGHT)
						SH1106_DrawPixel(x - j * pdy, y + j * pdx, color);
				}
			}
		}
	}

	return 1;
}


uint32_t SH1106_GetTextWidth(char *chars, size_t size, FONT_INFO font)
{
	uint8_t separator_character_width = font.height / 2;
	uint8_t space_character_width = font.height;
	uint32_t total_width = 0;
	for (uint32_t k = 0; k < size; k++)
	{
		if (chars[k] < 32) // checks if it's an actual character
			continue;

		if (chars[k] == ' ')
		{
			total_width += space_character_width;
			continue;
		}

		// checks if character exists in the font provided
		if (chars[k] - font.start_char > font.end_char - font.start_char)
			continue;

		total_width += font.descriptors[chars[k] - font.start_char].width;

		// if it's the last character, it's useless putting a separator
		if (k == size - 1)
			continue;

		total_width += separator_character_width;
	}

	return total_width;
}


// erase_last_write is ignored if SH1106_SetScreenAutoUpdate(false) has been called before
uint8_t SH1106_WriteChars(uint8_t x, uint8_t y, char *chars, size_t size, FONT_INFO font)
{
	uint8_t cur_x = x;	// SH1106_DrawPixel() already increments x by 2
	uint8_t cur_y = y;
	uint32_t char_position = 0;
	uint32_t char_width = 0;
	uint8_t max_x = 0;
	uint8_t separator_character_width = font.height / 2;

	for (uint32_t k = 0; k < size; k++)
	{
		if (k + 1 < size && chars[k] == '\n')
		{
			// new line
			cur_x = x;
			y += font.height * 8;
			cur_y = y;
			continue;
		}

		if (chars[k] == '\0')
			return 0;

		if (chars[k] < 32) // checks if it's an actual character
			continue;

		if (chars[k] != ' ')
		{
			// checks if character exists in the font provided
			if (chars[k] - font.start_char > font.end_char - font.start_char)
				continue;

			char_position = font.descriptors[chars[k] - font.start_char].offset;
			char_width = font.descriptors[chars[k] - font.start_char].width;

			// checks whether the character will go out of bounds
			// if yes, add a new line and write the remaining characters
			if ((cur_x + char_width + ((k == size - 1) ? 0 : separator_character_width)) > OLED_WIDTH - 4)
			{
				cur_x = x;
				y += font.height * 8;
				if (y + font.height * 8 > OLED_HEIGHT)
					break;
				cur_y = y;
			}

			// catches any weird, negative widths
			else if ((cur_x + char_width + (k == size - 1) ? 0 : separator_character_width) < 0)
				continue;

			for (uint32_t i = 0; i < char_width; i++)
			{
				for (uint32_t j = 0; j < font.height; j++)
				{
					uint8_t col_page_data = font.bitmaps[char_position + i * font.height + font.height - 1 - j];
					for (uint32_t h = 0; h < 8; h++)
					{
						if (col_page_data >> h & 0x01)
						{
							if (cur_y > OLED_HEIGHT)
								return 0;
							SH1106_DrawPixel(cur_x, cur_y, BLUE);
						}
						cur_y++;
					}
				}
				cur_x++;
				if (cur_x > max_x)
					max_x = cur_x;
				cur_y = y;
			}
		}

		// this point is reached if the character is a whitespace

		// if it's the last character, it's useless putting a separator or whitespace
		if (k == size - 1)
			continue;

		uint8_t space_character_width = separator_character_width * 2;
		uint8_t width_to_write = 0;

		if (cur_x + space_character_width > OLED_WIDTH)
			continue;

		if (chars[k] == ' ')
			width_to_write = space_character_width;
		else
			width_to_write = separator_character_width;

		for (uint32_t i = 0; i < width_to_write; i++)
		{
			for (uint32_t j = 0; j < font.height; j++)
			{
				SH1106_DrawPixel(cur_x, cur_y, BLACK);
				cur_y++;
			}
			cur_x++;
			if (cur_x > max_x)
				max_x = cur_x;
			cur_y = y;
		}
	}

	return 1;
}


void SH1106_ScrollDown(uint8_t scroll_px)
{
	if (OLED_HEIGHT != 64)
		return;
	for (uint32_t k = 0; k < OLED_WIDTH; k++)
	{
		uint64_t all_pages = 0x0000000000000000;

		for (uint32_t i = 0; i < 8; i++)
			all_pages |= (uint64_t)SH1106_Buffer[7 - i][k] << (56 - i * 8);

		all_pages = all_pages << (uint64_t)scroll_px;

		for (uint32_t i = 0; i < 8; i++)
			SH1106_Buffer[7 - i][k] = (uint8_t)(all_pages >> (56 - i * 8));
	}
}


void SH1106_ScrollUp(uint8_t scroll_px)
{
	if (OLED_HEIGHT != 64)
		return;
	for (uint32_t k = 0; k < OLED_WIDTH; k++)
	{
		uint64_t all_pages = 0x0000000000000000;

		for (uint32_t i = 0; i < 8; i++)
			all_pages |= (uint64_t)SH1106_Buffer[7 - i][k] << (56 - i * 8);

		all_pages = all_pages >> (uint64_t)scroll_px;

		for (uint32_t i = 0; i < 8; i++)
			SH1106_Buffer[7 - i][k] = (uint8_t)(all_pages >> (56 - i * 8));
	}
}


uint8_t SH1106_DrawBitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
	if (x < 0 || y < 0)
		return 0;

	uint8_t h = height / 8;
	uint8_t hmod = height % 8;
	if (hmod != 0)
		h += 1;

	for (uint32_t i = 0; i < h; i++)
	{
		for (uint32_t j = 0; j < width; j++)
		{
			if (j + width * i < width * height)
				SH1106_DrawByte(x + j, y + i * 8, bitmap[j + width * i], BLUE);
			else
				SH1106_DrawByte(x + j, y + i * 8, 0, BLUE);
		}
	}

	return 1;
}


void leftPadding(char *dest, char* src, uint8_t desired_size, char fill_character)
{
	uint32_t str_len = strlen(src);
	memset(dest, 0, desired_size);
	if (str_len == desired_size)
	{
		strcpy(dest, src);
		return;
	}

	if (str_len > desired_size)
	{
		strncpy(dest, src, desired_size);
	}
	else
	{
		for (uint32_t i = 0; i < desired_size - str_len; i++)
			dest[i] = fill_character;
		strcat(dest, src);
	}
}


void SH1106_Init(void)
{
	// https://www.displayfuture.com/Display/datasheet/controller/SH1106.pdf
	HAL_Delay(10);
	SH1106_WriteCmdSingle(0xAE);		// display off
	SH1106_WriteCmdDouble(0x8D, 0x14);	// turn on DC-DC converter
	SH1106_WriteCmdDouble(0xA8, 0x3F);	// multiplex ratio set to 64
	SH1106_WriteCmdDouble(0xD3, 0x00);	// line offset from top of the display
	SH1106_WriteCmdSingle(0x40 | 0x00);	// display start line (row)
	SH1106_WriteCmdSingle(0xA1);		// segment re-map (x coordinate, A0 or A1)
	SH1106_WriteCmdSingle(0xC8);		// output scan direction (y coordinate, left to right, C0 or C8)
	SH1106_WriteCmdDouble(0xDA, 0x12);	// sequential COM pin configuration
	SH1106_WriteCmdSingle(0x31);		// charge pump voltage - set a lower charge pump voltage LOWEST: 0x30; HIGHEST: 0x33
	SH1106_WriteCmdDouble(0x81, BRIGHTNESS);	// contrast control. from 00 to FF
	SH1106_WriteCmdSingle(0xA4);		// enables RAM content to be displayed
	SH1106_WriteCmdSingle(0xA6);		// non-inverted display
	SH1106_WriteCmdDouble(0xD5, 0xF0);	// display clock frequency (max)
	SH1106_WriteCmdSingle(0xAF);		// display on display on

	HAL_Delay(5);
	SH1106_ClearScreen();
	SH1106_UpdateScreen();
}
