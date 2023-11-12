/*
 * font.h
 *
 *  Created on: Nov 4, 2023
 *      Author: Kikkiu
 */

#ifndef INC_FONT_H_
#define INC_FONT_H_

#include "stdint.h"

typedef uint8_t uint_8;


typedef struct
{
    uint32_t width;
    uint32_t offset;
} FONT_CHAR_INFO;


typedef struct
{
	uint8_t height;
	char start_char;
	char end_char;
	uint8_t space_width;
	const FONT_CHAR_INFO *descriptors;
	const uint_8 *bitmaps;
} FONT_INFO;


extern const FONT_INFO Arial_12pt;
extern const FONT_INFO YaHei_36pt;

#endif /* INC_FONT_H_ */
