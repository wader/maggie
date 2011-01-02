/*
 * lcd.h
 * 	functions prototypes, pointer variables to the current lcd screen and
 * 	internal variables used when rendering screen.
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __LCD_H__
#define __LCD_H__

int lcd_init();
void lcd_uninit();
void lcd_update();
void lcd_render_line();
void lcd_render_line_background();
void lcd_render_line_background_part(int x,int y,unsigned char b1,unsigned char b2);
void lcd_render_line_window();
void lcd_render_line_window_part(int x,int y,unsigned char b1,unsigned char b2);
void lcd_render_line_sprite();
void lcd_render_line_sprite_part(int x,int y,unsigned char b1,unsigned char b2,int palette,int flip_x,int priority);

unsigned char *lcd_bitmap;
unsigned char *lcd_layer_bg;
unsigned char lcd_palette_bgwin[4];
unsigned char lcd_palette_obj[2][4];
int lcd_sprite_count;

#endif
