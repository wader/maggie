/*
 * lcdc.h
 * 	LCD Control constant and function prototypes
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __LCDC_H__
#define __LCDC_H__

#define LCDC_OPERATION		0x80
#define LCDC_WIN_TILE_MAP	0x40
#define LCDC_WIN_DISPLAY	0x20
#define LCDC_TILE_DATA		0x10
#define LCDC_BG_TILE_MAP	0x08
#define LCDC_OBJ_SIZE		0x04
#define LCDC_OBJ_DISPLAY	0x02
#define LCDC_BG_DISPLAY		0x01

#define LCDC_OAM_PRIORITY	0x80
#define LCDC_OAM_FLIP_Y		0x40
#define LCDC_OAM_FLIP_X		0x20
#define LCDC_OAM_PALETTE	0x10

#define LCDC_STAT_LYC_INT	0x40
#define LCDC_STAT_OAM_INT	0x20
#define LCDC_STAT_VBLANK_INT	0x10
#define LCDC_STAT_HBLANK_INT	0x08
#define LCDC_STAT_LYC_FLAG	0x04

#define LCDC_STAT_MODE_HBLANK	0x00 // Mode 00
#define LCDC_STAT_MODE_VBLANK	0x01 // Mode 01
#define LCDC_STAT_MODE_OAM	0x02 // Mode 10
#define LCDC_STAT_MODE_TRANSFER	0x03 // Mode 11

int lcdc_reset();
void lcdc_palette_change(int data, unsigned char *palette);
void lcdc_change(int data);
void lcdc_stat_change(int data);
void lcdc_step(int cycles);

int lcdc_stat_count;

#endif
