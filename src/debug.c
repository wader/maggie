/*
 * debug.c
 * 	The debugger
 * 
 * (c)2001 Mattias Wadman
 */

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "debug.h"
#include "debug_op_table.h"
#include "cpu.h"
#include "cpu_op.h"
#include "memory.h"
#include "lcdc.h"
#include "lcd.h"
#include "maggie.h"
#include "cartridge.h"
#include "timer.h"


int debug_step;
char *debug_watch_symbol;
char *debug_until_symbol;
int debug_until_value;
int debug_until_what;

#define UNTIL_EQUAL	0
#define UNTIL_NOTEQUAL	1
#define UNTIL_LESS	2
#define UNTIL_MORE	3

#define SYMBOL_CONSTANT	0
#define SYMBOL_UBYTE	1
#define SYMBOL_USHORT	2
#define SYMBOL_INT	3

struct
{
	char *name;
	int type;
	void *p;
	int v;
} debug_symbols[] =
{
	{"cpu_reg_pc",		SYMBOL_USHORT,	&cpu_reg_pc,		0},
	{"cpu_reg_sp",		SYMBOL_USHORT,	&cpu_reg_sp,		0},
	{"cpu_reg_a",		SYMBOL_UBYTE,	&cpu_reg[REG_A],	0},
	{"cpu_reg_f",		SYMBOL_UBYTE,	&cpu_reg[REG_F],	0},
	{"cpu_reg_b",		SYMBOL_UBYTE,	&cpu_reg[REG_B],	0},
	{"cpu_reg_c",		SYMBOL_UBYTE,	&cpu_reg[REG_C],	0},
	{"cpu_reg_d",		SYMBOL_UBYTE,	&cpu_reg[REG_D],	0},
	{"cpu_reg_e",		SYMBOL_UBYTE,	&cpu_reg[REG_E],	0},
	{"cpu_reg_h",		SYMBOL_UBYTE,	&cpu_reg[REG_H],	0},
	{"cpu_reg_l",		SYMBOL_UBYTE,	&cpu_reg[REG_L],	0},
	{"lcdc_stat_count",	SYMBOL_INT,	&lcdc_stat_count,	0},
	{"cpu_ime",		SYMBOL_INT,	&cpu_ime,		0},
	{"cpu_halt",		SYMBOL_INT,	&cpu_halt,		0},
	{"cpu_stop",		SYMBOL_INT,	&cpu_stop,		0},
	{"cpu_op_cycles",	SYMBOL_INT,	&cpu_op_cycles,		0},
	{"io_p1",		SYMBOL_UBYTE,	&io_p1,			0},
	{"io_sb",		SYMBOL_UBYTE,	&io_sb,			0},
	{"io_sc",		SYMBOL_UBYTE,	&io_sc,			0},
	{"io_div",		SYMBOL_UBYTE,	&io_div,		0},
	{"io_tima",		SYMBOL_UBYTE,	&io_tima,		0},
	{"io_tma",		SYMBOL_UBYTE,	&io_tma,		0},
	{"io_tac",		SYMBOL_UBYTE,	&io_tac,		0},
	{"io_if",		SYMBOL_UBYTE,	&io_if,			0},
	{"io_lcdc",		SYMBOL_UBYTE,	&io_lcdc,		0},
	{"io_stat",		SYMBOL_UBYTE,	&io_stat,		0},
	{"io_scx",		SYMBOL_UBYTE,	&io_scx,		0},
	{"io_scy",		SYMBOL_UBYTE,	&io_scy,		0},
	{"io_ly",		SYMBOL_UBYTE,	&io_ly,			0},
	{"io_lyc",		SYMBOL_UBYTE,	&io_lyc,		0},
	{"io_bgp",		SYMBOL_UBYTE,	&io_bgp,		0},
	{"io_obp0",		SYMBOL_UBYTE,	&io_obp0,		0},
	{"io_obp1",		SYMBOL_UBYTE,	&io_obp1,		0},
	{"io_wx",		SYMBOL_UBYTE,	&io_wx,			0},
	{"io_wy",		SYMBOL_UBYTE,	&io_wy,			0},	
	{NULL,0,NULL,0}
};

char *debug_commands[] =
{
	"quit","help","interrupt","show","set","dump","watch",
	"tile","systeminfo","memoryinfo","characterdata","until",
	"videoupdate","renderlcd","continue","all",
	NULL
};

char *debug_io_map[256] =
{
	"P1 (Joypad)",			/*0x00*/
	"SB (Serial Buffer)",		/*0x01*/
	"SC (Serial Control)",		/*0x02*/
	NULL,				/*0x03*/
	"DIV",				/*0x04*/
	"TIMA (Timer Counter)",		/*0x05*/
	"TMA (Timer Modulo)",		/*0x06*/
	"TAC (Timer Control)",		/*0x07*/
	NULL,				/*0x08*/
	NULL,				/*0x09*/
	NULL,				/*0x0a*/
	NULL,				/*0x0b*/
	NULL,				/*0x0c*/
	NULL,				/*0x0d*/
	NULL,				/*0x0e*/
	"IF (Interrupt Flag)",		/*0x0f*/
	
	// TODO: sound I/O register names
	NULL,				/*0x10*/
	NULL,				/*0x11*/
	NULL,				/*0x12*/
	NULL,				/*0x13*/
	NULL,				/*0x14*/
	NULL,				/*0x15*/
	NULL,				/*0x16*/
	NULL,				/*0x17*/
	NULL,				/*0x18*/
	NULL,				/*0x19*/
	NULL,				/*0x1a*/
	NULL,				/*0x1b*/
	NULL,				/*0x1c*/
	NULL,				/*0x1d*/
	NULL,				/*0x1e*/
	NULL,				/*0x1f*/
	NULL,				/*0x20*/
	NULL,				/*0x21*/
	NULL,				/*0x22*/
	NULL,				/*0x23*/
	NULL,				/*0x24*/
	NULL,				/*0x25*/
	NULL,				/*0x26*/
	NULL,				/*0x27*/
	NULL,				/*0x28*/
	NULL,				/*0x29*/
	NULL,				/*0x2a*/
	NULL,				/*0x2b*/
	NULL,				/*0x2c*/
	NULL,				/*0x2d*/
	NULL,				/*0x2e*/
	NULL,				/*0x2f*/
	NULL,				/*0x30*/
	NULL,				/*0x31*/
	NULL,				/*0x32*/
	NULL,				/*0x33*/
	NULL,				/*0x34*/
	NULL,				/*0x35*/
	NULL,				/*0x36*/
	NULL,				/*0x37*/
	NULL,				/*0x38*/
	NULL,				/*0x39*/
	NULL,				/*0x3a*/
	NULL,				/*0x3b*/
	NULL,				/*0x3c*/
	NULL,				/*0x3d*/
	NULL,				/*0x3e*/
	NULL,				/*0x3f*/

	"LCDC (LCD Control)",		/*0x40*/
	"STAT (LCD Status)",		/*0x41*/
	"SCY (Screen scroll Y)",	/*0x42*/
	"SCX (Screen scroll X)",	/*0x43*/
	"LY (LCD Line)",		/*0x44*/
	"LYC (LY compare)",		/*0x45*/
	"DMA (DMA Transfer)",		/*0x46*/
	"BGP (Background palette)",	/*0x47*/
	"OBP0 (Object palette 0)",	/*0x48*/
	"OBP1 (Object palette 1)",	/*0x49*/
	"WY (Window Y position)",	/*0x4a*/
	"WX (Window X position)",	/*0x4b*/
	NULL,				/*0x4c*/

	// TODO: GBC I/O register names
	NULL,				/*0x4d*/
	NULL,				/*0x4e*/
	NULL,				/*0x4f*/
	NULL,				/*0x50*/
	NULL,				/*0x51*/
	NULL,				/*0x52*/
	NULL,				/*0x53*/
	NULL,				/*0x54*/
	NULL,				/*0x55*/
	NULL,				/*0x56*/
	NULL,				/*0x57*/
	NULL,				/*0x58*/
	NULL,				/*0x59*/
	NULL,				/*0x5a*/
	NULL,				/*0x5b*/
	NULL,				/*0x5c*/
	NULL,				/*0x5d*/
	NULL,				/*0x5e*/
	NULL,				/*0x5f*/
	NULL,				/*0x60*/
	NULL,				/*0x61*/
	NULL,				/*0x62*/
	NULL,				/*0x63*/
	NULL,				/*0x64*/
	NULL,				/*0x65*/
	NULL,				/*0x66*/
	NULL,				/*0x67*/
	NULL,				/*0x68*/
	NULL,				/*0x69*/
	NULL,				/*0x6a*/
	NULL,				/*0x6b*/
	NULL,				/*0x6c*/
	NULL,				/*0x6d*/
	NULL,				/*0x6e*/
	NULL,				/*0x6f*/
	NULL,				/*0x70*/
	
	// High RAM
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0x80*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0x90*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xa0*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xb0*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xc0*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xd0*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xe0*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xf0*/
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /*0xfe*/
	
	"IE (Interrupt Enable)"		/*0xff*/
};

int debug_symbol_get(char *name)
{
	int i;

	for(i=0;debug_symbols[i].name;i++)
		if(strcmp(debug_symbols[i].name,name) == 0)
		{
			if(debug_symbols[i].type == SYMBOL_CONSTANT)
				return debug_symbols[i].v;
			else if(debug_symbols[i].type == SYMBOL_UBYTE)
				return *((unsigned char*)debug_symbols[i].p);
			else if(debug_symbols[i].type == SYMBOL_USHORT)
				return *((unsigned short*)debug_symbols[i].p);
			else
				return *((int*)debug_symbols[i].p);
		}
	printf("debug_variable_get: \"%s\" no such symbol\n",name);
	return 0;
}

void debug_symbol_set(char *name, int value)
{
	int i;

	for(i=0;debug_symbols[i].name;i++)
		if(strcmp(debug_symbols[i].name,name) == 0)
		{
			if(debug_symbols[i].type == SYMBOL_CONSTANT)
				printf("debug_variable_set: can't set constant\n");
			else if(debug_symbols[i].type == SYMBOL_UBYTE)
				*((unsigned char*)debug_symbols[i].p) = (unsigned char)value;
			else if(debug_symbols[i].type == SYMBOL_USHORT)
				*((unsigned short*)debug_symbols[i].p) = (unsigned short)value;
			else
				*((int*)debug_symbols[i].p) = value;
			return;
		}
	printf("debug_variable_set: \"%s\" no such symbol\n",name);
}

void debug_symbol_show_all()
{
	int i;

	for(i=0;debug_symbols[i].name;i++)
	{
		printf("%-15s = ",debug_symbols[i].name);
		if(debug_symbols[i].type == SYMBOL_CONSTANT)
			printf("%6d (0x%x)",debug_symbols[i].v,debug_symbols[i].v);
		else if(debug_symbols[i].type == SYMBOL_UBYTE)
			printf("%6d (0x%.2x)",*((unsigned char*)debug_symbols[i].p),*((unsigned char*)debug_symbols[i].p));
		else if(debug_symbols[i].type == SYMBOL_USHORT)
			printf("%6d (0x%.4x)",*((unsigned short*)debug_symbols[i].p),*((unsigned short*)debug_symbols[i].p));
		else
			printf("%6d (0x%x)",*((int*)debug_symbols[i].p),*((int*)debug_symbols[i].p));
		printf("\n");
	}
}

char *debug_completion_match(const char *text, int state)
{
	static int i,j;

	if(!state)
		i = j = 0;

	if(debug_commands[i])
		for(;debug_commands[i];i++)
			if(strncmp(debug_commands[i],text,strlen(text)) == 0)
				return strdup(debug_commands[i++]);

	if(debug_symbols[j].name)
		for(;debug_symbols[j].name;j++)
			if(strncmp(debug_symbols[j].name,text,strlen(text)) == 0)
				return strdup(debug_symbols[j++].name);
	
	return NULL;
}

char **debug_completion(const char *text, int i, int j)
{
	return rl_completion_matches(text,debug_completion_match);
}

char *debug_completion_null(const char *text, int i)
{
	return (char*)NULL;
}

int debug_init()
{
	rl_attempted_completion_function = debug_completion;
	rl_completion_entry_function = debug_completion_null;

	debug_step = FALSE;
	debug_until_symbol = NULL;
	debug_until_value = 0;
	debug_until_what = UNTIL_EQUAL;
	debug_watch_symbol = NULL;
	
	return TRUE;
}

void debug_uninit()
{
	clear_history();

	if(debug_watch_symbol)
		free(debug_watch_symbol);
	if(debug_until_symbol)
		free(debug_until_symbol);
}

void debug_trace()
{
	unsigned short pc = cpu_reg_pc;
	unsigned char op;
	char b[64];
	int t;

	if(debug_until_symbol)
	{
		int i = debug_symbol_get(debug_until_symbol);
		if(
			(debug_until_what == UNTIL_EQUAL && i == debug_until_value) ||
			(debug_until_what == UNTIL_NOTEQUAL && i != debug_until_value) ||
			(debug_until_what == UNTIL_LESS && i < debug_until_value) ||
			(debug_until_what == UNTIL_MORE && i > debug_until_value)
		)
		{
			printf("breaking, until condition met\n");
			free(debug_until_symbol);
			debug_until_symbol = NULL;
			
			debug_step = TRUE;
		}
	}
	
	if(!debug_step)
		return;

	op = MEMORY_READ(pc);
	printf("0x%.4x: %.2x ",pc,op);
	pc++;
	if(op == 0xcb)
	{
		op = MEMORY_READ(pc);
		printf("%.2x   :",op);
		snprintf(b,64,debug_op_table_cb[op].string);
	}
	else
		if(debug_op_table[op].size == 1)
		{
			printf("%.2x   :",MEMORY_READ(pc));
			snprintf(b,64,debug_op_table[op].string,MEMORY_READ(pc));
		}
		else if(debug_op_table[op].size == 2)
		{
			printf("%.2x %.2x:",MEMORY_READ(pc),MEMORY_READ(pc+1));
			snprintf(b,64,debug_op_table[op].string,MEMORY_READ(pc+1),MEMORY_READ(pc));
		}
		else
		{
			printf("     :");
			snprintf(b,64,debug_op_table[op].string);
		}

	printf(" %-15s ",b);

	printf(" AF=%.4x (%c%c%c%c) BC=%.4x DE=%.4x HL=%.4x SP=%.4x",
		REG_GET_16(REG_AF),
		(FLAG_VALUE_Z?'Z':'z'),
		(FLAG_VALUE_N?'N':'n'),
		(FLAG_VALUE_H?'H':'h'),
		(FLAG_VALUE_C?'C':'c'),
		REG_GET_16(REG_BC),
		REG_GET_16(REG_DE),
		REG_GET_16(REG_HL),
		cpu_reg_sp
		);
	
	if(strchr(debug_op_table[op].string,'(') && debug_op_table[op].size > 0) // ugly
	{
		t = (debug_op_table[op].size == 2 ? (MEMORY_READ(pc)>>8)+MEMORY_READ(pc+1) : 0xff00+MEMORY_READ(pc));
		if(t>>8 == 0xff && debug_io_map[t&0x00ff])
			printf(" %-10s",debug_io_map[t&0x00ff]);
	}

	printf("\n");
		
	debug_interactive();
}

void debug_interactive()
{
	int i,c,l;
	char *p,*t,*o;
	char *arguments[4];
		
	for(;;)
	{
		if(debug_watch_symbol)
			printf("%s=%d (0x%x) ",debug_watch_symbol,debug_symbol_get(debug_watch_symbol),debug_symbol_get(debug_watch_symbol));
		
		p = readline("debug>");
		c = 0;
		o = p;
		while(*o)
		{
			for(;*o && *o == ' ';o++); // strip spacees before argument
			if(!*o)
				break; // no reason to continue
			t = o;
			for(;*t && *t != ' ';t++); // count chars in argument
			l = t-o;
			arguments[c] = (char*)malloc(l+1);
			strncpy(arguments[c],o,l);
			arguments[c][l] = '\0';
			c++;
			o = t;
			if(c == 4) // currenly only need 3 arguments
				break;
		}

		if(c > 0)
		{
			add_history(p);
			if(strcmp(arguments[0],"help") == 0)
			{
				printf(
					"Command line Help (use tab for auto-completion)\n"
					"help			help\n"
					"quit			quit\n"
					"show arg		show value of argument\n"
					"set arg value		set argument to value\n"
					"systeminfo		show various system info\n"
					"memoryinfo		show memory map\n"
					"watch symbol		set symbol to watch\n"
					"until symbol !=<> value execute until condition is met\n"
					"tile size addr		render tile at addr\n"
					"dump page		dump memory page\n"
					"characterdata		render nintendo character data from chartridge\n"
					"videoupdate		update screen\n"
					"renderlcd		render whole lcd screen and update screen\n"
					"continue		continue execution\n"
					);
			}
			else if(strcmp(arguments[0],"quit") == 0)
			{
				maggie_running = FALSE;
				break;
			}
			else if(strcmp(arguments[0],"show") == 0)
			{
				if(c < 2)
					printf("usage: show arg\n");
				else if(c > 1 && strcmp(arguments[1],"all") == 0)
					debug_symbol_show_all();
				else
				{
					int i = debug_symbol_get(arguments[1]);
					printf("%s=%d (0x%x)\n",arguments[1],i,i);
				}
			}
			else if(strcmp(arguments[0],"set") == 0)
			{
				if(c < 3)
					printf("usage: set arg value\n");
				else
					debug_symbol_set(arguments[1],strtol(arguments[2],NULL,0));
					
			}
			else if(strcmp(arguments[0],"systeminfo") ==0)
			{
				char *modes[4] = {"HBLANK","VBLANK","OAM","TRANSFER"};
				int timer[4] = {4096,262144,65536,16384};
			
				printf(" IME: %s\n",(cpu_ime ? "ON" : "OFF"));
				
				printf("  IE: ");
				if(cpu_ie & CPU_INTERRUPT_VBLANK) printf("VBLANK ");
				if(cpu_ie & CPU_INTERRUPT_LCDC) printf("LCDC ");
				if(cpu_ie & CPU_INTERRUPT_TIMER) printf("TIMER ");
				if(cpu_ie & CPU_INTERRUPT_SERIAL) printf("SERIAL ");
				if(cpu_ie & CPU_INTERRUPT_JOYPAD) printf("JOYPAD ");
				printf("\n");
				
				printf("  IF: ");
				if(io_if & CPU_INTERRUPT_VBLANK) printf("VBLANK ");
				if(io_if & CPU_INTERRUPT_LCDC) printf("LCDC ");
				if(io_if & CPU_INTERRUPT_TIMER) printf("TIMER ");
				if(io_if & CPU_INTERRUPT_SERIAL) printf("SERIAL ");
				if(io_if & CPU_INTERRUPT_JOYPAD) printf("JOYPAD ");
				printf("\n");

				printf("LCDC: ");
				printf("%s ",(io_lcdc&LCDC_OPERATION ? "ON" : "OFF"));
				printf("winmap=%s ",(io_lcdc&LCDC_WIN_TILE_MAP ? "0x9c00" : "0x9800"));
				printf("win=%s ",(io_lcdc&LCDC_WIN_DISPLAY ? "ON" : "OFF"));
				printf("tiledata=%s ",(io_lcdc&LCDC_TILE_DATA ? "0x8800" : "0x8000"));
				printf("bgmap=%s ",(io_lcdc&LCDC_WIN_TILE_MAP ? "0x9c00" : "0x9800"));
				printf("objsize=%s ",(io_lcdc&LCDC_OBJ_SIZE ? "8x16" : "8x8"));
				printf("obj=%s ",(io_lcdc&LCDC_OBJ_DISPLAY ? "ON" : "OFF"));
				printf("bg=%s",(io_lcdc&LCDC_BG_DISPLAY ? "ON" : "OFF"));
				printf("\n");
				
				printf("STAT: ");
				if(io_stat&LCDC_STAT_LYC_FLAG) printf("LYC=LY ");
				if(io_stat&LCDC_STAT_LYC_INT) printf("LYC_INT ");
				if(io_stat&LCDC_STAT_OAM_INT) printf("OAM_INT ");
				if(io_stat&LCDC_STAT_VBLANK_INT) printf("VBLANK_INT ");
				if(io_stat&LCDC_STAT_HBLANK_INT) printf("HBLANK_INT ");
				printf("%s",modes[io_stat&0x03]);
				printf("\n");
				
				printf("TIMA: %x\n",io_tima);
				printf(" TMA: %x\n",io_tma);	
				printf(" TAC: ");
				printf("%s ",(io_tac&TIMER_CONTROL ? "ON" : "OFF"));
				printf("%d Hz",timer[io_tac&0x03]);
				printf("\n");	
			}
			else if(strcmp(arguments[0],"memoryinfo") ==0)
			{
				int i;

				printf("memory map: read / write\n");
				for(i=0;i <16;i++)
					printf("memory[%x] = %p / %p\n",i,memory_map_read[i],memory_map_write[i]);
			}
			else if(strcmp(arguments[0],"dump") ==0)
			{
				if(c < 2)
					printf("usage: dump page\n");
				else
				{
					int i,j,c;
					int page = strtol(arguments[1],NULL,0);
				
					page %= 0x100;
					page <<= 8;
					for(i=0;i < 16;i++)
					{
						printf("0x%.4x:",page+(i*16));
						for(j=0;j < 16;j++)
							printf(" %.2x",MEMORY_READ(page+(i*16)+j));
						printf(" ");
						for(j=0;j < 16;j++)
						{
							c = MEMORY_READ(page+(i*16)+j);
							printf("%c",(isprint(c) ? c : ' '));
						}
						printf("\n");
					}
				}
			}
			else if(strcmp(arguments[0],"tile") == 0)
			{
				if(c < 3)
					printf("usage: tile size addr\n");
				else
				{
					int i,j,k,size,addr;
					size = strtol(arguments[1],NULL,0);
					addr = strtol(arguments[2],NULL,0);
					
					for(k=0;k < size;k++)
					{
						for(i=7;i > -1;i--)
						{
							j = (((MEMORY_READ(addr+k*2)&(1<<i)&&1))<<1) + ((MEMORY_READ(addr+1+k*2)&(1<<i)&&1));
							if(j) printf("%d",j);
							else printf(" ");
						}
						printf("\n");
					}
				}
			}
			else if(strcmp(arguments[0],"characterdata") == 0)
			{
				int i,j,k,l,m;
				for(m=0;m < 2;m++)
					for(i=0;i < 2;i++)
						for(j=0;j < 2;j++)
							for(l=0;l < 12;l++)
								for(k=0;k < 4;k++)
									printf("%s%s",((1<<((j?3:7)-k))&MEMORY_READ(CARTRIDGE_CHARACTER_DATA+(l*2)+i+(m?24:0))?"*":" "),(k==3&&l==11?"\n":""));
			}
			else if(strcmp(arguments[0],"watch") == 0)
			{
				if(c == 1 || c == 2)
				{
					if(debug_watch_symbol)
						free(debug_watch_symbol);
					debug_watch_symbol = NULL;
					if(c ==  1)
						printf("watch cleared\n");
					else
					{
						debug_watch_symbol = strdup(arguments[1]);
						printf("watch set to: %s\n",debug_watch_symbol);
					}
				}
				else
					printf("usage: watch symbol\n");
			}
			else if(strcmp(arguments[0],"until") == 0)
			{
				if(c == 1 || c == 4)
				{
					if(debug_until_symbol)
						free(debug_until_symbol);
					debug_until_symbol = NULL;
					if(c == 1)
						printf("until condition cleared\n");
					else
					{
						debug_until_symbol = strdup(arguments[1]);
						switch(arguments[2][0])
						{
							case '!': debug_until_what = UNTIL_NOTEQUAL; break;
							case '=': debug_until_what = UNTIL_EQUAL; break;
							case '<': debug_until_what = UNTIL_LESS; break;
							case '>': debug_until_what = UNTIL_MORE; break;
							default: printf("unknown condition check\n"); break;
						}
						debug_until_value = strtol(arguments[3],NULL,0);
						printf("until condition set\n");
					}
				}
				else
					printf("usage: until symbol =|!|<|> value\n");
			}
			else if(strcmp(arguments[0],"videoupdate") == 0)
				video_update();
			else if(strcmp(arguments[0],"renderlcd") == 0)
			{
				int oly = io_ly;
				
				for(io_ly=0;io_ly < 144;io_ly++)
					lcd_render_line();

				io_ly = oly;
				video_update();
			}
			else if(strcmp(arguments[0],"continue") == 0)
			{
				debug_step = FALSE;
				break;
			}
			else
				printf("unknown command: %s\n",arguments[0]);
		}
		else
		{
			debug_step = TRUE;
			break; // no command, exit debugger
		}
		
		for(i=0;i < c;i++)
			free(arguments[i]);
	}
	
	for(i=0;i < c;i++)
		free(arguments[i]);
}

