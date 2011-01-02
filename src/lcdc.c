/*
 * Functions that handle changes to LCD and the LCDC STAT I/O register
 * 
 * (c)2001 Mattias Wadman
 */

#include "lcdc.h"
#include "lcd.h"
#include "cpu.h"
#include "memory.h"
#include "joypad.h"
#include "maggie.h"


int lcdc_reset()
{
	lcdc_stat_count = 114;
	io_stat = 0x01; // VBLANK
	
	memory_io_write(IO_STAT,0x00); // will call lcdc_change
	memory_io_write(IO_LCDC,0x91); // set tile data and bg/win map memory pointers
	
	// set palettes
	memory_io_write(IO_BGP,0xfc);
	memory_io_write(IO_OBP0,0xff);
	memory_io_write(IO_OBP1,0xff);

	io_ly = 0;
	io_lyc = 0;
	io_scy = 0;
	io_scx = 0;
	io_wy = 0;
	io_wx = 0;

	return TRUE;
}

/*
 * Called from memory_io_write.
 * Updates the memory pointers used by the LCD render functions.
 */
void lcdc_change(int data)
{
        io_lcdc = data;
	memory_ram_lcd_tile_data = (io_lcdc&LCDC_TILE_DATA?memory_ram_video:memory_ram_video+0x1000);
	memory_ram_lcd_map_bg = memory_ram_video+(io_lcdc&LCDC_BG_TILE_MAP?0x1c00:0x1800);
	memory_ram_lcd_map_win = memory_ram_video+(io_lcdc&LCDC_WIN_TILE_MAP?0x1c00:0x1800);									
}

/*
 * Called from memory_io_write.
 * Limit write access and do possibly do LCDC (LYC) interrupt
 */
void lcdc_stat_change(int data)
{
	io_stat = 0x80|(io_stat&(0x80|LCDC_STAT_LYC_FLAG|0x03)) | (data&~(0x80|LCDC_STAT_LYC_FLAG|0x03));
	
	if(io_lyc == io_ly)
	{
		io_stat |= LCDC_STAT_LYC_FLAG;
		if(data & LCDC_STAT_LYC_INT)
			cpu_interrupt(CPU_INTERRUPT_LCDC);
	}
	else
		io_stat &= ~LCDC_STAT_LYC_FLAG;
}

/*
 * Called from memory_io_write
 * Update palette used by the LCD render functions.
 */
void lcdc_palette_change(int data, unsigned char *palette)
{
	palette[0] = data&0x03;
	palette[1] = (data>>2)&0x03;
	palette[2] = (data>>4)&0x03;
	palette[3] = data>>6;
}

/*
 * The functions tries to imitate the LCDC MODE transition and possibly do VBLANK
 * and LCDC interrupts. It also calls the LCD render function and increse the LY
 * register value.
 */
void lcdc_step(int cycles)
{
	static int gap = FALSE;
	int mode;
	
	lcdc_stat_count -= cycles;
	
	if(lcdc_stat_count < 0)
	{
		mode = io_stat&0x03;
		io_stat &= 0xfc;
		if(io_ly < 144) // 0-143
		{
			switch(mode)
			{
				case LCDC_STAT_MODE_VBLANK: // only on first line
					io_stat |= LCDC_STAT_MODE_TRANSFER;
					lcdc_stat_count += 20;
					if(io_stat & LCDC_STAT_OAM_INT)
						cpu_interrupt(CPU_INTERRUPT_LCDC);
					break;
				case LCDC_STAT_MODE_OAM:
					io_stat |= LCDC_STAT_MODE_TRANSFER;
					lcdc_stat_count += 43;
					break;
				case LCDC_STAT_MODE_TRANSFER:
					io_stat |= LCDC_STAT_MODE_HBLANK;
					if(io_stat & LCDC_STAT_HBLANK_INT)
						cpu_interrupt(CPU_INTERRUPT_LCDC);
					lcdc_stat_count += 51;
					break;
				case LCDC_STAT_MODE_HBLANK:
					lcd_render_line();
					io_ly++;
					if(io_ly == 144)
					{
						io_stat |= LCDC_STAT_MODE_VBLANK;
						lcdc_stat_count += 3; // 3 cycles gap to interrupt.. more?
						gap = TRUE;
					}
					else
					{
						io_stat |= LCDC_STAT_MODE_OAM;
						lcdc_stat_count += 23; // should be 20
						if(io_stat & LCDC_STAT_OAM_INT)
							cpu_interrupt(CPU_INTERRUPT_LCDC);
						lcdc_stat_change(io_stat); // will check for ly=lyc
					}
					break;
				default:
					maggie_error("WARNING: lcdc_step: unknown mode\n");
					break;
			}
		}
		else if(io_ly < 153) // 144-152
		{
			io_stat |= LCDC_STAT_MODE_VBLANK;
			if(io_ly == 144)
			{
				if(gap)
				{
					video_update();
					joypad_update();

					cpu_interrupt(CPU_INTERRUPT_VBLANK);
					lcdc_stat_change(io_stat);
					gap = FALSE;
				}
				else
					io_ly++;
				lcdc_stat_change(io_stat);
				lcdc_stat_count += 114;
			}
			else
			{
				io_ly++;
				lcdc_stat_change(io_stat);
				if(io_ly == 153)
					lcdc_stat_count += 3;
				else
					lcdc_stat_count += 114;
			}
		}
		else if(io_ly == 153)
		{
			io_stat |= LCDC_STAT_MODE_VBLANK;
			io_ly = 0;
			lcdc_stat_change(io_stat);
			lcdc_stat_count += 114;
		}
		else
			maggie_error("WARNING: lcdc_step io_ly > 153\n");
	}
}

