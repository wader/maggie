/*
 * memory.c
 *	Handle read and write to memory, bank switching, loading of cartridge and IO registers
 * 
 * (c)2001 Mattias Wadman
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "memory.h"
#include "cpu.h"
#include "cartridge.h"
#include "lcd.h"
#include "lcdc.h"
#include "timer.h"
#include "joypad.h"
#include "maggie.h"


int memory_rom_internal_load(char *file)
{
    FILE *stream;

    memory_rom_internal = malloc(256);
    stream = fopen(file, "rb");
    fread(memory_rom_internal, 1, 256, stream);
    fclose(stream);
    
    return TRUE;
}

/*
 * Load ROM file into memory
 */
int memory_rom_load(char *file)
{
	FILE *stream;
	struct stat s;
	int c,i;
	char *b;

	if(*file == '-' && *(file+1) == '\0')
		stream = stdin;
	else if((stream = fopen(file,"rb")) == NULL)
		return maggie_error("ERROR: memory_rom_load: fopen failed opening \"%s\"\n",file);
	
	if(!(b = malloc(0x8000)))
		return maggie_error("ERROR: memory_rom_load: stdin: malloc: buffer failed (%d)\n",memory_rom);
	
	for(i=0;(c = fread(b,1,0x8000,stream));)
	{
		i++;
		cartridge_size = i*0x8000;
		
		if(!(memory_rom = realloc(memory_rom,cartridge_size)))
		{
			free(b);
			return maggie_error("ERROR: memory_rom_load: stdin: realloc: rom failed (%d)\n",cartridge_size);
		}
			
		memcpy(memory_rom+(i-1)*0x8000,b,c);
	}

	free(b);

	if(*file != '-' && *(file+1) != '\0')
	{	
		fstat(fileno(stream),&s);
		cartridge_size = s.st_size;
		fclose(stream);
		
		if(cartridge_size&0xfff)
			maggie_error("WARNING: memory_rom_load: funny file size? (%lu %lx)\n",cartridge_size,cartridge_size);
	}
	else
		fflush(stdin);

	return TRUE;
}

/*
 * Called from memory_io_write.
 * Does a transfer of 0xa0 bytes from 0x<data>00 to 0xfe00
 */
void memory_dma(unsigned char data)
{
	if(memory_map_read[data>>4] == NULL)
		maggie_error("WARNING: memory_dma: memory_map_read[%d] not set\n",data>>4);
	else
		memcpy(memory_ram_lcd_oam,memory_map_read[data>>4]+((data&0x0f)<<8),0xa0);
}

/*
 * Allocates memory for the memory system
 */
int memory_init()
{
	if(!(memory_ram_video = malloc(0x2000)))
	{
		maggie_error("ERROR: memory_init: malloc: ram_video failed\n");
		return FALSE;
	}

	if(!(memory_ram_internal_low = malloc(0x2000)))
	{
		maggie_error("ERROR: memory_init: malloc: ram_internal_low failed\n");
		return FALSE;
	}
	
	if(!(memory_ram_internal_high = malloc(0x1000)))
	{
		maggie_error("ERROR: memory_init: malloc: ram_internal_high failed\n");
		return FALSE;
	}
	
	return TRUE;
}

void memory_uninit()
{
	if(memory_rom)
		free(memory_rom);
	if(memory_ram_video)
		free(memory_ram_video);
	if(memory_ram_internal_low)
		free(memory_ram_internal_low);
	if(memory_ram_internal_high)
		free(memory_ram_internal_high);
	if(memory_ram_switchable)
		free(memory_ram_switchable);
}

/*
 * Set the default memory read and write map, allocates cartridge RAM and set various
 * register values.
 *
 * Memory map:
 * 0x0000-0x0fff ROM bank 0 fixed
 * 0x1000-0x1fff ROM bank 0 fixed
 * 0x2000-0x2fff ROM bank 0 fixed
 * 0x3000-0x3fff ROM bank 0 fixed
 * 0x4000-0x4fff ROM bank switchable
 * 0x5000-0x5fff ROM bank switchable
 * 0x6000-0x6fff ROM bank switchable
 * 0x7000-0x7fff ROM bank switchable
 * 0x8000-0x8fff RAM Video, tile patterns, maps (switchable on GBC?)
 * 0x9000-0x9fff RAM Video, tile patterns, maps (switchable on GBC?)
 * 0xa000-0xafff RAM bank switchable
 * 0xb000-0xbfff RAM bank switchable
 * 0xc000-0xcfff RAM internal (internal bank fixed 0 on GBC?)
 * 0xd000-0xdfff RAM internal (switchable bank on GBC only?)
 * 0xe000-0xefff Echo of internal RAM (nothing on GBC?)
 * 0xf000-0xffff Echo of internal RAM continue, OAM, IO Registers, RAM internal (zero page), Interrupt Enable
 * 
 */
int memory_reset()
{
	int i;

	if(cartridge_banks_ram > 0)
		if(!(memory_ram_switchable = malloc(cartridge_banks_ram*0x2000)))
		{
			maggie_error("ERROR: memory_reset: malloc: ram_switchable falied (%d)\n",cartridge_banks_ram*0x2000);
			return FALSE;
		}
	
	/* silently give RAM to cartridges with no RAM :) */
	if(cartridge_banks_ram == 0)
		if(!(memory_ram_switchable = malloc(0x2000)))
		{
			maggie_error("ERROR: memory_reset: malloc: ram_switchable falied (%d)\n",0x2000);
			return FALSE;
		}
	
	
	//memory_map_read[0x0] = memory_rom;
        // ROM hack: make read call memory_read...
	memory_map_read[0x0] = NULL;
	
        
        memory_map_read[0x1] = memory_rom+0x1000;
	memory_map_read[0x2] = memory_rom+0x2000;
	memory_map_read[0x3] = memory_rom+0x3000;
	memory_map_read[0x4] = NULL;
	memory_map_read[0x5] = NULL;
	memory_map_read[0x6] = NULL;
	memory_map_read[0x7] = NULL;
	memory_map_read[0x8] = memory_ram_video;		
	memory_map_read[0x9] = memory_ram_video+0x1000;		
	memory_map_read[0xa] = NULL;
	memory_map_read[0xb] = NULL;
	memory_map_read[0xc] = memory_ram_internal_low;
	memory_map_read[0xd] = memory_ram_internal_low+0x1000;
	memory_map_read[0xe] = NULL;
	memory_map_read[0xf] = NULL;
	
	memory_map_write[0x0] = NULL;
	memory_map_write[0x1] = NULL;
	memory_map_write[0x2] = NULL;
	memory_map_write[0x3] = NULL;
	memory_map_write[0x4] = NULL;
	memory_map_write[0x5] = NULL;
	memory_map_write[0x6] = NULL;
	memory_map_write[0x7] = NULL;
	memory_map_write[0x8] = memory_ram_video;
	memory_map_write[0x9] = memory_ram_video+0x1000;
	memory_map_write[0xa] = NULL;
	memory_map_write[0xb] = NULL;
	memory_map_write[0xc] = memory_ram_internal_low;
	memory_map_write[0xd] = memory_ram_internal_low+0x1000;
	memory_map_write[0xe] = NULL;
	memory_map_write[0xf] = NULL;
	
	memory_mbc_model = 0; // is this really default to 0? Zoomar.gb animation works if default is 1
	memory_mbc_enabled = FALSE;
	memory_mbc_rom_bank = 1;
	memory_mbc_ram_bank = 0;

	// Fixed 32KB ROM
	memory_switch_bank_rom(memory_mbc_rom_bank);
	
	// Even cartridges with no get some switchable RAM (see above) 
	memory_switch_bank_ram(0);
	
	// random data in low RAM and cartridge RAM at start
	for(i=0;i < 0x2000;i++)
		*(memory_ram_internal_low+i) = random()%256;
	for(i=0;i < 0x2000*cartridge_banks_ram;i++)
		*(memory_ram_switchable+i) = random()%256;

	// zero fill video and high RAM
	for(i=0;i < 0x2000;i++)
		*(memory_ram_video+i) = 0;
	for(i=0;i < 0x1000;i++)
		*(memory_ram_internal_high+i) = 0;	
	
	// video memory pointer used by the lcd functions
	memory_ram_lcd_map_bg = NULL;
	memory_ram_lcd_map_win = NULL;
	memory_ram_lcd_tile_data = NULL;
	memory_ram_lcd_oam = memory_ram_internal_high+0x0e00;
	memory_ram_lcd_oam_tile_data = memory_ram_video;
	
	for(i=0;i < 0x80;i++)
	{
		io_unused[i] = 0xff;
		io_mapper[i] = &io_unused[i];
	}
	
	// map used io registers to known variables
	io_mapper[IO_P1]	= &io_p1;
	io_mapper[IO_SB]	= &io_sb;
	io_mapper[IO_SC]	= &io_sc;
	io_mapper[IO_DIV]	= &io_div;
	io_mapper[IO_TIMA]	= &io_tima;
	io_mapper[IO_TMA]	= &io_tma;
	io_mapper[IO_TAC]	= &io_tac;
	io_mapper[IO_IF]	= &io_if;
	io_mapper[IO_LCDC]	= &io_lcdc;
	io_mapper[IO_STAT]	= &io_stat;
	io_mapper[IO_SCY]	= &io_scy;
	io_mapper[IO_SCX]	= &io_scx;
	io_mapper[IO_LY]	= &io_ly;
	io_mapper[IO_LYC]	= &io_lyc;
	io_mapper[IO_BGP]	= &io_bgp;
	io_mapper[IO_OBP0]	= &io_obp0;
	io_mapper[IO_OBP1]	= &io_obp1;
	io_mapper[IO_WY]	= &io_wy;
	io_mapper[IO_WX]	= &io_wx;
	
	// need to be set somewhere
	io_if	= 0xe1;
	io_sb	= 0x00;
	io_sc	= 0x73;
	
	// initial values to unknown/unused registers
	io_unused[0x10] = 0x80;
	io_unused[0x11] = 0xbf;
	io_unused[0x12] = 0xf3;
	io_unused[0x14] = 0xbf;
	io_unused[0x16] = 0x3f;
	io_unused[0x17] = 0x00;
	io_unused[0x19] = 0xbf;
	io_unused[0x1a] = 0x7f;
	io_unused[0x1b] = 0xff;
	io_unused[0x1c] = 0x9f;
	io_unused[0x1e] = 0xbf;
	io_unused[0x20] = 0xff;
	io_unused[0x21] = 0x80;
	io_unused[0x22] = 0x00;
	io_unused[0x23] = 0xbf;
	io_unused[0x24] = 0x77;
	io_unused[0x25] = 0xfe;
	io_unused[0x26]	= 0xf1; // 0xf0 if SGB
	io_unused[0x27] = 0x00;
	io_unused[0x28] = 0x00;
	io_unused[0x29] = 0x00;
	io_unused[0x2a] = 0x00;
	io_unused[0x2b] = 0x00;
	io_unused[0x2c] = 0x00;
	io_unused[0x2d] = 0x00;
	io_unused[0x2e] = 0x00; 
	io_unused[0x2f] = 0x00;
	io_unused[0x30] = 0x06; // 0x30-0x3f sound wave
	io_unused[0x31] = 0xfe;
	io_unused[0x32] = 0x0e;
	io_unused[0x33] = 0x7f;
	io_unused[0x34] = 0x00;
	io_unused[0x35] = 0xff;
	io_unused[0x36] = 0x58;
	io_unused[0x37] = 0xdf;
	io_unused[0x38] = 0x00;
	io_unused[0x39] = 0xec;
	io_unused[0x3a] = 0x00;
	io_unused[0x3b] = 0xbf;
	io_unused[0x3c] = 0x0c;
	io_unused[0x3d] = 0xed;
	io_unused[0x3e] = 0x03;
	io_unused[0x3f] = 0xf7;
	io_unused[0x4d] = 0xfe; // sgb,cgb cpu speed, 0xfe normal 4 mhz (or 1 mhz) speed
	io_unused[0x51] = 0x00;
	io_unused[0x52] = 0x00;
	io_unused[0x53] = 0x00;
	io_unused[0x54] = 0x00;
	io_unused[0x55] = 0xff;
	io_unused[0x56] = 0x00;
	io_unused[0x57] = 0x00;
	io_unused[0x58] = 0x00;
	io_unused[0x59] = 0x00;
	io_unused[0x5a] = 0x00;
	io_unused[0x5b] = 0x00;
	io_unused[0x5c] = 0x00;
	io_unused[0x5d] = 0x00;
	io_unused[0x5e] = 0x00;
	io_unused[0x5f] = 0x00;
	io_unused[0x60] = 0x00;
	io_unused[0x61] = 0x00;
	io_unused[0x62] = 0x00;
	io_unused[0x63] = 0x00;
	io_unused[0x64] = 0x00;
	io_unused[0x65] = 0x00;
	io_unused[0x66] = 0x00;
	io_unused[0x67] = 0x00;
	io_unused[0x68] = 0xc0;
	io_unused[0x69] = 0x00;
	io_unused[0x6a] = 0xc1;
	io_unused[0x6b] = 0x00;
	io_unused[0x6c] = 0x00;
	io_unused[0x6d] = 0x00;
	io_unused[0x6e] = 0x00;
	io_unused[0x6f] = 0x00;
	io_unused[0x70] = 0xf8;
	io_unused[0x71] = 0x00;
	io_unused[0x72] = 0x00;
	io_unused[0x73] = 0x00;
	io_unused[0x74] = 0x00;
	io_unused[0x75] = 0x00;
	io_unused[0x76] = 0x00;
	io_unused[0x77] = 0x00;
	io_unused[0x78] = 0x00;
	io_unused[0x79] = 0x00;
	io_unused[0x7a] = 0x00;
	io_unused[0x7b] = 0x00;
	io_unused[0x7c] = 0x00;
	io_unused[0x7d] = 0x00;
	io_unused[0x7e] = 0x00;
	io_unused[0x7f] = 0x00;

	return TRUE;
}

/*
 * Used to pop something from the stack
 */
int memory_stack_pop()
{
	if(cpu_reg_sp+2 > 0xffff)
	{
		maggie_error("IGNORING: memory_stack_pop outside memory %x\n",cpu_reg_sp+2);
		return 0;
	}
	
	cpu_reg_sp += 2;
	return (MEMORY_READ(cpu_reg_sp-1)<<8)+MEMORY_READ(cpu_reg_sp-2);
}

/*
 * Used to push something on the stack
 */ 
void memory_stack_push(int data)
{
	cpu_reg_sp -= 2;
	MEMORY_WRITE(cpu_reg_sp,data&0x00ff);
	MEMORY_WRITE(cpu_reg_sp+1,data>>8);
}

/*
 * Called if MEMORY_READ macro gets a address where memory_map_read is NULL.
 * Probably 0xe000 to 0xffff.
 */
int memory_read(register int addr)
{
	switch(addr>>12)
	{
            case 0x0:
                if(addr < 0x0100)
                    return memory_rom_internal[addr&0x00ff];
                else
                    return memory_rom[addr&0xdfff];
                break;

		case 0xe:
			return MEMORY_READ(addr&~0xdffff);
			break;
		case 0xf:
			if(addr>>8 == 0xff && !(addr&BIT_7))
				return memory_io_read(addr&0x00ff);
			else if(addr>>8 < 0xfe)
				return MEMORY_READ(addr&~0xdffff);
			else if(addr == 0xffff)
				return cpu_ie;
			else
				return *(memory_ram_internal_high+(addr&0x0fff));
			break;	
		default:
			maggie_error("WARNING: memory_read: should not be here (%.4x)\n",addr);
			break;
	}
	return 0;
}

/*
 * Called if MEMORY_WRITE macro gets a address where memory_map_write is NULL.
 * Probably 0x0000 to 0x7fff or 0xe000 to 0xffff.
 *
 * TODO: MBC switching is currently broken
 */
void memory_write(register int addr, register int data)
{
	int page = addr>>12;

	switch(page)
	{
		case 0xe:
			MEMORY_WRITE(addr&~0xdffff,data);
			break;
		case 0xf:
			if(addr>>8 == 0xff && !(addr&BIT_7))
				memory_io_write(addr&0x00ff,data);
			else if(addr>>8 < 0xfe)
				MEMORY_WRITE(addr&~0xdffff,data);
			else if(addr == 0xffff)
				cpu_ie = data;
			else
				*(memory_ram_internal_high+(addr&0x0fff)) = data;
			break;
		default:
		
			switch(cartridge_current_type->mbc)
			{
				case CARTRIDGE_MBC_MBC1: case CARTRIDGE_MBC_HUC1:
					if(page == 0x2 || page == 0x3)
						memory_mbc_rom_bank = (memory_mbc_rom_bank&0x60) | ((data&0x1f) == 0 ? 1 : (data&0x1f));
					else if(page == 0x4 || page == 0x5)
						if(memory_mbc_model)
							memory_mbc_ram_bank = data&0x03;
						else
							memory_mbc_rom_bank = (memory_mbc_rom_bank&0x1f) | ((data&0x03)<<5);
					else if(page == 0x6 || page == 0x7)
						memory_mbc_model = data&0x01;
					break;
				case CARTRIDGE_MBC_MBC2:
					if(addr & 0x0100)
						memory_mbc_rom_bank = ((data&0x0f) == 0 ? 1 : (data&0x0f));
					break;
				case CARTRIDGE_MBC_MBC3:
					if(page == 0x2 || page == 0x3)
						memory_mbc_rom_bank = ((data&0x7f) == 0 ? 1 : (data&0x7f));
					else if(page == 0x4 || page == 0x5)
						memory_mbc_ram_bank = data&0x03;
					break;
				case CARTRIDGE_MBC_RUMBLE:
					if(page == 0x4 || page == 0x5)
						data &= 0x07; // remove rumble motor bit
					/* fallthru */ 
				case CARTRIDGE_MBC_MBC5:
					if(page == 0x2)
						memory_mbc_rom_bank = (memory_mbc_rom_bank&0x100) | data;
					else if(page == 0x3)
						memory_mbc_rom_bank = (memory_mbc_rom_bank&0xff) | ((data&0x01)<<9);
					else if(page == 0x4 || page == 0x5)
						memory_mbc_ram_bank = data&0x0f;
					break;
			}
			
			printf("MEMORY_WRITE: %.4x:%.2x ROM=%.2x RAM=%.2x\n",addr,data,memory_mbc_rom_bank,memory_mbc_ram_bank);
			
			memory_switch_bank_rom(memory_mbc_rom_bank);
			memory_switch_bank_ram(memory_mbc_ram_bank);
	
			break;
	}
}

/*
 * Called from memory_io_write and memory_reset
 * Does a ROM bank switch by updating the memory_map_read.
 */
void memory_switch_bank_rom(int b)
{
	if(b >= cartridge_banks_rom)
		maggie_error("IGNORING: memory_switch_bank_rom: b=%d >= cartridge_banks_rom=%d\n",b,cartridge_banks_rom);
	else
	{
		memory_map_read[0x4] = memory_rom+0x0000+(b*0x4000);
		memory_map_read[0x5] = memory_rom+0x1000+(b*0x4000);
		memory_map_read[0x6] = memory_rom+0x2000+(b*0x4000);
		memory_map_read[0x7] = memory_rom+0x3000+(b*0x4000);
	}
}

/*
 * Called from memory_io_write and memory_reset
 * Does a RAM bank switch by updating the memory_map_read and memory_map_write.
 */
void memory_switch_bank_ram(int b)
{
	// see comments in memory_init
	if(b != 0 && b >= cartridge_banks_ram)
		maggie_error("IGNORING: memory_switch_bank_ram: b=%d >= cartridge_banks_ram=%d\n",b,cartridge_banks_ram);
	else
	{	
		memory_map_read[0xa] = memory_ram_switchable+0x0000+(b*0x2000);
		memory_map_read[0xb] = memory_ram_switchable+0x1000+(b*0x2000);
		memory_map_write[0xa] = memory_ram_switchable+0x0000+(b*0x2000);
		memory_map_write[0xb] = memory_ram_switchable+0x1000+(b*0x2000);
	}
}

/*
 * Called from memory_write.
 * Set a value of a address in the range 0xff00-0xff7f, will just set the value or forward
 * it to a function.
 */
void memory_io_write(int addr, int data)
{
	switch(addr)
	{
                case 0x50:
                        /* internal ROM hack */
                        if(data > 0)
                        {
                            printf("switching internal rom\n");
	                    memory_map_read[0x0] = memory_rom;
                        }
                        break;
		case IO_DMA: memory_dma(data); break;
		case IO_DIV: io_div = 0; break;
		case IO_BGP: lcdc_palette_change(data,&lcd_palette_bgwin[0]); io_bgp = data; break;
		case IO_OBP0: lcdc_palette_change(data,&lcd_palette_obj[0][0]); io_obp0 = data; break;
		case IO_OBP1: lcdc_palette_change(data,&lcd_palette_obj[1][0]); io_obp1 = data; break;
		case IO_LCDC: lcdc_change(data); break;
		case IO_STAT: lcdc_stat_change(data); break;
		case IO_LY: lcdc_stat_change(io_stat); break; // should do lcdc reset?
		case IO_LYC: io_lyc = data; lcdc_stat_change(io_stat); break;
		case IO_TAC: timer_change(data); break;
		case IO_P1: joypad_change(data); break;
		case IO_SC:
			io_sc = data|0x7e;
			if(data&0x80) // serial transfer start
			{
				io_sb = 0xff; // no gameboy at other end
				io_sc = data & 0x7f; // serial transfer done
				//cpu_interrupt(CPU_INTERRUPT_TIMER); // should be delayed?
			}
			break;
		default:
			*io_mapper[addr] = data;
			break;
	}
}

/*
 * Called from memory_read.
 * Read a value of a address in the range 0xff00-0xff7f, will just read the value or forward
 * it to a function.
 */
int memory_io_read(int addr)
{
	switch(addr)
	{
		default:
			return *io_mapper[addr];
			break;
	}
}

