/*
 * lcd.c
 * 	Handle rendering of background, window and sprites
 * 
 * (c)2001 Mattias Wadman
 */

#include <stdlib.h>
#include <memory.h>

#include "lcd.h"
#include "memory.h"
#include "lcdc.h"
#include "maggie.h"

int lcd_init()
{	
	if(!(lcd_bitmap = malloc(0x5a00)))
	{
		maggie_error("ERROR: lcd_init: malloc: lcd_bitmap failed\n");
		return FALSE;
	}
	if(!(lcd_layer_bg = malloc(0x5a00)))
	{
		maggie_error("ERROR: lcd_init: malloc: lcd_layer_bg failed\n");
		return FALSE;
	}

	return TRUE;
}

void lcd_uninit()
{
	if(lcd_bitmap)
		free(lcd_bitmap);
	
	if(lcd_layer_bg)
		free(lcd_layer_bg);
}

int lcd_reset()
{
	memset(lcd_bitmap,0,0x5a00);
	memset(lcd_layer_bg,0,0x5a00);
	
	return TRUE;
}

void lcd_render_line()
{
	if(io_ly == 0)
	{
		memset(lcd_bitmap,0,0x5a00);
		memset(lcd_layer_bg,0,0x5a00);
	}
	
 	if(!(io_lcdc&LCDC_OPERATION))
		return;
	
	if(io_ly > 143)
		maggie_error("WARNING: lcd_render_line: io_ly=%d (will not render)\n",io_ly);
	else
	{
		if(io_lcdc&LCDC_BG_DISPLAY)
			lcd_render_line_background();

		if(io_lcdc&LCDC_WIN_DISPLAY)
			lcd_render_line_window();
	
		if(io_lcdc&LCDC_OBJ_DISPLAY)
			lcd_render_line_sprite();
	}
}

void lcd_render_line_background_part(int x,int y,unsigned char b1,unsigned char b2)
{
	int i,r,o;
	unsigned char p;

	if(y > 159)
		return;

	for(i=0;i < 8 && x+i < 160;i++)
	{
		r = 7-i;
		p = (((b2&(1<<r)&&1))<<1)+((b1&(1<<r)&&1));
		if(x+i < 0)
			continue;
		o = y*160;
		*(lcd_layer_bg+o+x+i) = p;	
		*(lcd_bitmap+o+x+i) = lcd_palette_bgwin[p];
	}
}

void lcd_render_line_background()
{
	unsigned char b1,b2;
	unsigned char *map;
	int dy,i,r,t;
			
	r = ((io_scy+io_ly)%256)>>3;
	dy = ((io_scy+io_ly)%256)%8;

	map = memory_ram_lcd_map_bg+(r<<5);
		
	if(io_lcdc & LCDC_TILE_DATA)
		for(i=0;i < 21;i++)
		{
			t = ((io_scx+(i<<3))%256)>>3;
			b1 = *(memory_ram_lcd_tile_data+(*(map+t)<<4)+(dy<<1));
			b2 = *(memory_ram_lcd_tile_data+(*(map+t)<<4)+(dy<<1)+1);
			lcd_render_line_background_part((i<<3)-((io_scx)%8),io_ly,b1,b2);
		}
	else
		for(i=0;i < 21;i++)
		{
			t = ((io_scx+(i<<3))%256)>>3;
			b1 = *(memory_ram_lcd_tile_data+((signed char)*(map+t)<<4)+(dy<<1));
			b2 = *(memory_ram_lcd_tile_data+((signed char)*(map+t)<<4)+(dy<<1)+1);
			lcd_render_line_background_part((i<<3)-((io_scx)%8),io_ly,b1,b2);
		}
}


void lcd_render_line_window_part(int x,int y,unsigned char b1,unsigned char b2)
{
	int i,r;
	unsigned char p;

	if(y > 159)
		return;

	for(i=0;i < 8 && x+i < 160;i++)
	{
		r = 7-i;
		p = (((b2&(1<<r)&&1))<<1)+((b1&(1<<r)&&1));
		if(x+i < 0)
			continue;
		*(lcd_bitmap+(y*160)+x+i) = lcd_palette_bgwin[p];
	}
}

void lcd_render_line_window()
{
	unsigned char b1,b2;
	unsigned char *map;
	int i,dy;
	
	if(io_ly < io_wy)
		return;
	
	map = memory_ram_lcd_map_win+(((io_ly-io_wy)>>3)<<5);
	
	if(io_lcdc & LCDC_TILE_DATA)
		for(i=0;i < 20;i++)
		{
			if((i<<3)+io_wx-7 > 160)
				break;
			
			dy = (io_ly-io_wy)%8;		
			b1 = *(memory_ram_lcd_tile_data+(*(map+i)<<4)+(dy<<1));
			b2 = *(memory_ram_lcd_tile_data+(*(map+i)<<4)+(dy<<1)+1);	
			lcd_render_line_window_part((i<<3)+io_wx-7,io_ly,b1,b2);
		}
	else
		for(i=0;i < 20;i++)
		{
			if((i<<3)+io_wx-7 > 160)
				break;
			
			dy = (io_ly-io_wy)%8;	
			b1 = *(memory_ram_lcd_tile_data+((signed char)*(map+i)<<4)+(dy<<1));
			b2 = *(memory_ram_lcd_tile_data+((signed char)*(map+i)<<4)+(dy<<1)+1);
			lcd_render_line_window_part((i<<3)+io_wx-7,io_ly,b1,b2);
		}
}

void lcd_render_line_sprite_part(int x,int y,unsigned char b1,unsigned char b2,int palette,int flip_x,int priority)
{
	int i,r;
	unsigned char p;

	if(y > 159)
		return;

	for(i=0;i < 8 && x+i < 160;i++)
	{
		if(flip_x)
			r = i;
		else
			r = 7-i;
		p = (((b2&(1<<r)&&1))<<1)+((b1&(1<<r)&&1));
		
		if(x+i < 0 || p == 0) // point outside screen or color is 0
			continue;
		
		if(priority)
		{
			if(*(lcd_layer_bg+(y*160)+x+i) == 0) // if 0, in front of background "layer"
				*(lcd_bitmap+(y*160)+x+i) = lcd_palette_obj[palette][p];
		}
		else
			*(lcd_bitmap+(y*160)+x+i) = lcd_palette_obj[palette][p];	
	}
}

void lcd_render_line_sprite()
{
	unsigned char b1,b2,sprite_flags;
	int sprite_pattern,sprite_palette,sprite_x,sprite_y,sprite_dy,i;
	
	// TEMP FIX: reversed sprite draw order to get sprite with low OAM number on top.
	
	lcd_sprite_count = 0;
	
	if(io_lcdc&LCDC_OBJ_SIZE)
		for(i=39;i > -1;i--)
		{
			sprite_x = *(memory_ram_lcd_oam+(i<<2)+1)-8;
			sprite_y = *(memory_ram_lcd_oam+(i<<2))-16;
		
			if(io_ly < sprite_y || io_ly > sprite_y+15)
				continue;
			
			lcd_sprite_count++;
			
			sprite_dy = io_ly-sprite_y;
			sprite_pattern = *(memory_ram_lcd_oam+(i<<2)+2)&0xfe; // 8x16 mode, reset bit 0
			sprite_flags = *(memory_ram_lcd_oam+(i<<2)+3);
			sprite_palette = (sprite_flags&LCDC_OAM_PALETTE?1:0);
			
			if(sprite_flags&LCDC_OAM_FLIP_Y)
				sprite_dy = 15-sprite_dy;
			
			b1 = *(memory_ram_lcd_oam_tile_data+(sprite_pattern<<4)+(sprite_dy<<1));
			b2 = *(memory_ram_lcd_oam_tile_data+(sprite_pattern<<4)+(sprite_dy<<1)+1);

			lcd_render_line_sprite_part(sprite_x,io_ly,b1,b2,sprite_palette,sprite_flags&LCDC_OAM_FLIP_X,sprite_flags&LCDC_OAM_PRIORITY);
		}
	else
		for(i=39;i > -1;i--)
		{
			sprite_x = *(memory_ram_lcd_oam+(i<<2)+1)-8;
			sprite_y = *(memory_ram_lcd_oam+(i<<2))-16;
			
			if(io_ly < sprite_y || io_ly > sprite_y+7)
				continue;
			
			lcd_sprite_count++;
			
			sprite_dy = io_ly-sprite_y;	
			sprite_pattern = *(memory_ram_lcd_oam+(i<<2)+2);
			sprite_flags = *(memory_ram_lcd_oam+(i<<2)+3);
			sprite_palette = (sprite_flags&LCDC_OAM_PALETTE?1:0);
			
			if(sprite_flags&LCDC_OAM_FLIP_Y)
				sprite_dy = 7-sprite_dy;
			
			b1 = *(memory_ram_lcd_oam_tile_data+(sprite_pattern<<4)+(sprite_dy<<1));
			b2 = *(memory_ram_lcd_oam_tile_data+(sprite_pattern<<4)+(sprite_dy<<1)+1);
				
			lcd_render_line_sprite_part(sprite_x,io_ly,b1,b2,sprite_palette,sprite_flags&LCDC_OAM_FLIP_X,sprite_flags&LCDC_OAM_PRIORITY);
		}
}

