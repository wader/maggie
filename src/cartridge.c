/*
 * cartridge.c
 * 	Read and validates cartridge header
 * 
 * (c)2001 Mattias Wadman
 */

#include <stdio.h>
#include <string.h>

#include "cartridge.h"
#include "memory.h"
#include "maggie.h"

struct cartridge_type cartridge_type_table[] =
{
/*	type	ram		mbc						mmm01	battery	timer	rumble	text */
	{0x00,	FALSE,	CARTRIDGE_MBC_ROM_ONLY,	FALSE,	FALSE,	FALSE,	FALSE,	"ROM ONLY"},
	{0x01,	FALSE,	CARTRIDGE_MBC_MBC1,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC1"},
	{0x02,	TRUE,	CARTRIDGE_MBC_MBC1,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC1+RAM"},
	{0x03,	TRUE,	CARTRIDGE_MBC_MBC1,		FALSE,	TRUE,	FALSE,	FALSE,	"ROM+MBC1+RAM+BATTERY"},
	{0x05,	FALSE,	CARTRIDGE_MBC_MBC2,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC2"},
	{0x06,	FALSE,	CARTRIDGE_MBC_MBC2,		FALSE,	TRUE,	FALSE,	FALSE,	"ROM+MBC2+BATTERY"},
	{0x08,	TRUE,	CARTRIDGE_MBC_ROM_ONLY,	FALSE,	FALSE,	FALSE,	FALSE,	"ROM+RAM"},
	{0x09,	TRUE,	CARTRIDGE_MBC_ROM_ONLY,	FALSE,	TRUE,	FALSE,	FALSE,	"ROM+RAM+BATTERY"},
	{0x0b,	FALSE,	CARTRIDGE_MBC_ROM_ONLY,	TRUE,	FALSE,	FALSE,	FALSE,	"ROM+MMM01"},
	{0x0c,	TRUE,	CARTRIDGE_MBC_ROM_ONLY,	TRUE,	FALSE,	FALSE,	FALSE,	"ROM+MMM01+SRAM"},
	{0x0d,	TRUE,	CARTRIDGE_MBC_ROM_ONLY,	TRUE,	TRUE,	FALSE,	FALSE,	"ROM+MMM01+SRAM+BATTERY"},
	{0x0f,	FALSE,	CARTRIDGE_MBC_MBC3,		FALSE,	TRUE,	TRUE,	FALSE,	"ROM+MBC3+TIMER+BATTERY"},
	{0x10,	TRUE,	CARTRIDGE_MBC_MBC3,		FALSE,	TRUE,	TRUE,	FALSE,	"ROM+MBC3+TIMER+RAM+BATTERY"},
	{0x11,	FALSE,	CARTRIDGE_MBC_MBC3,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC3"},
	{0x12,	TRUE,	CARTRIDGE_MBC_MBC3,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC3+RAM"},
	{0x13,	TRUE,	CARTRIDGE_MBC_MBC3,		FALSE,	TRUE,	FALSE,	FALSE,	"ROM+MBC3+RAM+BATTERY"},
	{0x19,	FALSE,	CARTRIDGE_MBC_MBC5,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC5"},
	{0x1a,	TRUE,	CARTRIDGE_MBC_MBC5,		FALSE,	FALSE,	FALSE,	FALSE,	"ROM+MBC5+RAM"},
	{0x1b,	TRUE,	CARTRIDGE_MBC_MBC5,		FALSE,	TRUE,	FALSE,	FALSE,	"ROM+MBC5+RAM+BATTERY"},
	{0x1c,	FALSE,	CARTRIDGE_MBC_MBC5,		FALSE,	FALSE,	FALSE,	TRUE,	"ROM+MBC5+RUMBLE"},
	{0x1d,	TRUE,	CARTRIDGE_MBC_MBC5,		FALSE,	FALSE,	FALSE,	TRUE,	"ROM+MBC5+RUMBLE+SRAM"},
	{0x1e,	TRUE,	CARTRIDGE_MBC_MBC5,		FALSE,	TRUE,	FALSE,	TRUE,	"ROM+MBC5+RUMBLE+SRAM+BATTERY"},
	{0xfc,	FALSE,	CARTRIDGE_MBC_ROM_ONLY,	FALSE,	FALSE,	FALSE,	FALSE,	"Pocket Camera"},
	{0xfd,	FALSE,	CARTRIDGE_MBC_ROM_ONLY,	FALSE,	FALSE,	FALSE,	FALSE,	"Bandai TAMA5"},
	{0xfe,	FALSE,	CARTRIDGE_MBC_ROM_ONLY, FALSE,	FALSE,	FALSE,	FALSE,	"Hudson HuC-3"},
	{0xff,	FALSE,	CARTRIDGE_MBC_ROM_ONLY,	FALSE,	FALSE,	FALSE,	FALSE,	"Hudson HuC-1"},
	{0,0,0,0,0,0,0,NULL}
};

struct
{
	int code;
	char *text;
} cartridge_old_license_table[] =
{
	{0x01,	"Nintendo"},
	{0x08,	"Capcom"},
	{0x33,	"Nintendo"},
	{0x51,	"Aklaim"},
	{0x5a,	"Mindscape"},
	{0x79,	"Accolade"},
	{0xa4,	"Konami"},
	{0xb1,	"Nexoft"},
	{0,NULL}
};

struct
{
	int code;
	char *text;
} cartridge_license_table[] =
{
	{0x3130,	"Nintendo"},
	{0x3301,	"Nintendo"},
	{0x7901,	"Accolade"},
	{0xa400,	"Konami"},
	{0xc0de,	"MegaMan_X"},
	{0,NULL}
};

int cartridge_rom_table[256] =
{
	2,4,8,16,32,64,128,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,
	72,80,96
};

/* cartridges with no RAM get 1 bank anyway */
int cartridge_ram_table[256] =
{
	0,1,1,4,16
};

/*
 * Read and calculates various things about the current ROM.
 */
int cartridge_init()
{
	unsigned long int i;
	unsigned short j;
	unsigned char k;
	char *unknown = "Unknown";
	
	// Game title
	memcpy(cartridge_title,memory_rom+CARTRIDGE_GAME_TITLE,16);
	cartridge_title[15] = '\0';
	
	// Cartidge type
	cartridge_current_type = &cartridge_type_table[0]; // default
	for(i=0;cartridge_type_table[i].text;i++)
		if(cartridge_type_table[i].type == *(memory_rom+CARTRIDGE_TYPE))
			cartridge_current_type = &cartridge_type_table[i];
	
	// Old license
	cartridge_old_license_value = *(memory_rom+CARTRIDGE_LICENSE_OLD);
	cartridge_old_license_name = unknown;
	for(i=0;cartridge_old_license_table[i].text;i++)
		if(cartridge_old_license_table[i].code == cartridge_old_license_value)
			cartridge_old_license_name = cartridge_old_license_table[i].text;
	
	// License
	cartridge_license_value = (*(memory_rom+CARTRIDGE_LICENSE+1)<<8)+*(memory_rom+CARTRIDGE_LICENSE);
	cartridge_license_name = unknown;
	for(i=0;cartridge_license_table[i].text;i++)
		if(cartridge_license_table[i].code == cartridge_license_value)
			cartridge_license_name = cartridge_license_table[i].text;

	// Checksum
	cartridge_checksum_value = (*(memory_rom+CARTRIDGE_CHECKSUM)<<8)+*(memory_rom+CARTRIDGE_CHECKSUM+1);
	for(j=0,i=0;i < cartridge_size;i++)
		if(i != CARTRIDGE_CHECKSUM && i != CARTRIDGE_CHECKSUM+1)
			j += *(memory_rom+i);
	cartridge_checksum_calculated = j;

	// Complement
	cartridge_complement_value = *(memory_rom+CARTRIDGE_COMPLEMENT);
	for(k=0,i=0x134;i < 0x14d;i++)
		k += *(memory_rom+i);
	k = ~(k+0x18); // should be 0x19?
	cartridge_complement_calculated = k;
	
	// Other
	cartridge_banks_rom = cartridge_rom_table[*(memory_rom+CARTRIDGE_ROM_BANKS)];
	cartridge_banks_ram = cartridge_ram_table[*(memory_rom+CARTRIDGE_RAM_BANKS)];
	cartridge_gameboy_color = (*(memory_rom+CARTRIDGE_GB_COLOR) == 0x80 ? TRUE : FALSE);
	cartridge_super_gameboy = (*(memory_rom+CARTRIDGE_GB_SUPER) == 0x03 ? TRUE : FALSE);
	cartridge_rom_version = *(memory_rom+CARTRIDGE_ROM_VERSION);
	cartridge_destination = *(memory_rom+CARTRIDGE_DESTINATION);

	return TRUE;
}

void cartridge_show_info()
{
	printf("          Size: %ld\n",cartridge_size);
	printf("         Title: %s\n",cartridge_title);
	printf("          Type: %s\n",cartridge_current_type->text);
	printf("     ROM banks: %d (%d bytes)\n",cartridge_banks_rom,cartridge_banks_rom*0x4000);
	printf("     RAM banks: %d (%d bytes)\n",cartridge_banks_ram,cartridge_banks_ram*0x2000);
	printf(" Gameboy color: %s\n",(cartridge_gameboy_color ? "Yes" : "No"));
	printf(" Super Gameboy: %s\n",(cartridge_super_gameboy ? "Yes" : "No"));
	printf("      Checksum: %s (0x%.4x)\n",(cartridge_checksum_value == cartridge_checksum_calculated ? "Valid" : "Invalid"),cartridge_checksum_value);
	printf("    Complement: %s (0x%.2x)\n",(cartridge_complement_value == cartridge_complement_calculated ? "Valid" : "Invalid"),cartridge_complement_value);
	printf("       License: %s (0x%.4x)\n",cartridge_license_name,cartridge_license_value);
	printf("   Old license: %s (0x%.2x)\n",cartridge_old_license_name,cartridge_old_license_value);
	printf("   ROM version: 0x%.2x\n",cartridge_rom_version);
	printf("   Destination: %s\n",(cartridge_destination == 0x00 ? "Japanese" : "Other"));
}

