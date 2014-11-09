/* Declarations for lcd.c */

#ifndef _H_LCD
#define _H_LCD

#include "emu.h"

void lcd_draw_frame(u8 buffer[240][160]);

void lcd_cx_draw_frame(u16 *buffer, u32 *colormasks);

void lcd_reset(void);

u32 lcd_read_word(u32 addr);

void lcd_write_word(u32 addr, u32 value);

void *lcd_save_state(size_t *size);

void lcd_reload_state(void *state);

#endif
