/*
 * disassembler.c
 *	Simple GameBoy ROM disassembler
 * 
 * (c)2001 Mattias Wadman
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "debug_op_table.h"

#define SIGNED_8(a) ((a)&0x80?-((~a+1)&0xff):(a))

unsigned char *rom;
unsigned long int pc;

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

void disassemble()
{
	struct debug_op *op;
	int cb=0;

	printf("%.2lx: %.4lx: %.2x ",pc>>14,pc%0x4000,*(rom+pc));
	
	if(*(rom+pc) == 0xcb)
	{
		pc++;
		op = &debug_op_table_cb[*(rom+pc)];
		cb = 1;
	}
	else
		op = &debug_op_table[*(rom+pc)];
	
	pc++;

	if(!op->size)
	{
		if(!cb)
			printf("     : ");
		else
			printf("%.2x   : ",*(rom+pc-1));
		printf(op->string);
	}
	else if(op->size == 1)
	{
		printf("%.2x   : ",*(rom+pc));
		printf(op->string,*(rom+pc));
		
		// relativ jumps and increments
		if(*(rom+pc-1) == 0x20 || *(rom+pc-1) == 0x18 || *(rom+pc-1) == 0x28 || *(rom+pc-1) == 0x30 || *(rom+pc-1) == 0x38)
			printf(" ; %+d (%.4lx)",SIGNED_8(*(rom+pc)),pc+1+SIGNED_8(*(rom+pc)));
		if(*(rom+pc-1) == 0xE8 || *(rom+pc-1) == 0xF8)
			printf(" ; %+d",SIGNED_8(*(rom+pc)));

		if(strchr(op->string,'(') && op->size > 0) // ugly
		{
			int i;
			i = (op->size == 2 ? (*(rom+pc)>>8)+*(rom+pc+1) : 0xff00+*(rom+pc));
			
			if(i>>8  == 0xff && debug_io_map[i&0x00ff])
				printf(" ; %s",debug_io_map[i&0x00ff]);
		}
	}
	else
	{
		printf("%.2x %.2x: ",*(rom+pc),*(rom+pc+1));
		printf(op->string,*(rom+pc+1),*(rom+pc));
	}
	printf("\n");

	pc+=op->size;
}

int main(int argc,char *argv[])
{
	FILE *stream;
	struct stat s;
	int bank;
	
	if(argc < 2)
	{
		fprintf(stderr,"Usage: %s file [offset(hex)] [bank]\n",argv[0]);
		return EXIT_FAILURE;
	}

	if((stream = fopen(argv[1],"rb")) == NULL)
	{
		fprintf(stderr,"can't open %s\n",argv[1]);
		return EXIT_FAILURE;
	}

	pc = 0;
	
	if(argc > 2)
		sscanf(argv[2],"%lx",&pc);
	if(argc > 3)
	{
		sscanf(argv[3],"%d",&bank);
		pc += bank*0x4000;
	}
	
	fstat(fileno(stream),&s);
	rom = malloc(s.st_size);
	fread(rom,1,s.st_size,stream);
	fclose(stream);
			
	while(pc < s.st_size)
		disassemble();
		
	free(rom);

	return EXIT_SUCCESS;
}

