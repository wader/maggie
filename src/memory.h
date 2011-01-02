/*
 * memory.h
 * 	memory pointer variables, functions prototypes and macros for fast memory access 
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#define MEMORY_READ(addr) ( memory_map_read[((addr)&0xffff)>>12] ? *(memory_map_read[((addr)&0xffff)>>12]+((addr)&0x0fff)) : memory_read((addr)&0xffff) )
#define MEMORY_WRITE(addr,data) ( memory_map_write[((addr)&0xffff)>>12] ? *(memory_map_write[((addr)&0xffff)>>12]+((addr)&0x0fff)) = data : memory_write((addr)&0xffff,data) )

#define IO_P1		0x00
#define IO_SB		0x01
#define IO_SC		0x02
#define IO_DIV		0x04
#define IO_TIMA		0x05
#define IO_TMA		0x06
#define IO_TAC		0x07
#define IO_IF		0x0f
#define IO_LCDC		0x40
#define IO_STAT		0x41
#define IO_SCY		0x42
#define IO_SCX		0x43
#define IO_LY		0x44
#define IO_LYC		0x45
#define IO_DMA		0x46
#define IO_BGP		0x47
#define IO_OBP0		0x48
#define IO_OBP1		0x49
#define IO_WY		0x4a
#define IO_WX		0x4b

int memory_init();
int memory_reset();
void memory_uninit();
int memory_rom_internal_load(char *file);
int memory_rom_load(char *file);
int memory_stack_pop();
void memory_stack_push(int data);
int memory_read(register int addr);
void memory_write(register int addr,register int data);
void memory_dma(unsigned char data);
void memory_io_write(int addr,int data);
int memory_io_read(int addr);
void memory_switch_bank_rom(int b);
void memory_switch_bank_ram(int b);

unsigned char *memory_map_read[16];
unsigned char *memory_map_write[16];
unsigned char *memory_rom;
unsigned char *memory_ram_internal_low;
unsigned char *memory_ram_internal_high;
unsigned char *memory_ram_switchable;
unsigned char *memory_ram_video;
unsigned char *memory_ram_lcd_map_bg;
unsigned char *memory_ram_lcd_map_win;
unsigned char *memory_ram_lcd_tile_data;
unsigned char *memory_ram_lcd_oam;
unsigned char *memory_ram_lcd_oam_tile_data;

unsigned char *memory_rom_internal;

int memory_mbc_model;
int memory_mbc_enabled;
int memory_mbc_rom_bank;
int memory_mbc_ram_bank;

unsigned char io_p1;
unsigned char io_sb;
unsigned char io_sc;
unsigned char io_div;
unsigned char io_tima;
unsigned char io_tma;
unsigned char io_tac;
unsigned char io_if;
unsigned char io_lcdc;
unsigned char io_stat;
unsigned char io_scx;
unsigned char io_scy;
unsigned char io_ly;
unsigned char io_lyc;
unsigned char io_bgp;
unsigned char io_obp0;
unsigned char io_obp1;
unsigned char io_wy;
unsigned char io_wx;
unsigned char *io_mapper[0x80];
unsigned char io_unused[0x80];

#endif
