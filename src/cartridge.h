/*
 * cartridge.h
 * 	cartridge specific constants, variables and function prototypes
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#define CARTRIDGE_START				0x100
#define CARTRIDGE_CHARACTER_DATA	0x104 // 0x104-0x133 Nintendo graphic
#define CARTRIDGE_GAME_TITLE		0x134 // 0x134-0x142 Game title
#define CARTRIDGE_GB_COLOR			0x143
#define CARTRIDGE_LICENSE			0x144 // 0x144 high, 0x145 low
#define CARTRIDGE_GB_SUPER			0x146
#define CARTRIDGE_TYPE				0x147
#define CARTRIDGE_ROM_BANKS			0x148
#define CARTRIDGE_RAM_BANKS			0x149
#define CARTRIDGE_DESTINATION		0x14a
#define CARTRIDGE_LICENSE_OLD		0x14b
#define CARTRIDGE_ROM_VERSION		0x14c
#define CARTRIDGE_COMPLEMENT		0x14d
#define CARTRIDGE_CHECKSUM			0x14e // 0x14e high, 0x14f low

#define CARTRIDGE_MBC_ROM_ONLY		0
#define CARTRIDGE_MBC_MBC1			1
#define CARTRIDGE_MBC_MBC2			2
#define CARTRIDGE_MBC_MBC3			3
#define CARTRIDGE_MBC_MBC5			4
#define CARTRIDGE_MBC_RUMBLE		5
#define CARTRIDGE_MBC_HUC1			6

struct cartridge_type
{
	int type;
	int ram;
	int mbc;
	int mmm01;
	int battery;
	int timer;
	int rumble;
	char *text;
};

int cartridge_init();
void cartridge_show_info();

unsigned long int cartridge_size;
char cartridge_title[16];
unsigned short cartridge_license_value;
char *cartridge_license_name;
unsigned char cartridge_old_license_value;
char *cartridge_old_license_name;
unsigned char cartridge_destination;
unsigned short cartridge_checksum_value;
int cartridge_checksum_calculated;
int cartridge_banks_ram;
int cartridge_banks_rom;
int cartridge_gameboy_color;
int cartridge_super_gameboy;
unsigned char cartridge_complement_value;
int cartridge_complement_calculated;
unsigned char cartridge_rom_version;

struct cartridge_type *cartridge_current_type;

#endif
