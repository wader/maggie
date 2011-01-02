/*
 * cpu.h
 * 	CPU specific constants, variables and function prototypes
 * 	Also a lot of usefull macros
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __CPU_H__
#define __CPU_H__

#define SIGNED_8(a) ((a)&BIT_7?-((~a+1)&0xff):(a))

#define BIT_0			0x01
#define BIT_1			0x02
#define BIT_2			0x04
#define BIT_3			0x08
#define BIT_4			0x10
#define BIT_5			0x20
#define BIT_6			0x40
#define BIT_7			0x80

#define REG_A			0
#define	REG_F			1
#define	REG_B			2
#define	REG_C			3
#define	REG_D			4
#define	REG_E			5
#define	REG_H			6
#define	REG_L			7

#define REG_AF			0
#define REG_BC			2
#define REG_DE			4
#define REG_HL			6

// only used in cpu_op_table.h
#define REG_SP			0x08

#define REG_GET_HL		((cpu_reg[REG_H]<<8)+cpu_reg[REG_L])
#define REG_SET_HL(v)		{cpu_reg[REG_H]=(v)>>8; cpu_reg[REG_L]=(v)&0x00ff;}
#define REG_GET_16(r)		((cpu_reg[r]<<8)+cpu_reg[r+1])
#define REG_SET_16(r,v)		{cpu_reg[r]=(unsigned char)((v)>>8); cpu_reg[r+1]=(unsigned char)(v);}

#define COND_C			0
#define	COND_NC			1
#define COND_Z			2
#define COND_NZ			3

#define COND_CHECK(c)	((c == COND_NZ && !FLAG_VALUE_Z) || (c == COND_Z && FLAG_VALUE_Z) || (c == COND_NC && !FLAG_VALUE_C) || (c == COND_C && FLAG_VALUE_C))
		

#define FLAG_BIT_Z		0x80
#define FLAG_BIT_N		0x40
#define FLAG_BIT_H		0x20
#define FLAG_BIT_C		0x10
#define FLAG_BIT_VALUE_Z	((cpu_reg[REG_F]&FLAG_BIT_Z)>>7)
#define FLAG_BIT_VALUE_N	((cpu_reg[REG_F]&FLAG_BIT_N)>>6)
#define FLAG_BIT_VALUE_H	((cpy_reg[REG_F]&FLAG_BIT_H)>>5)
#define FLAG_BIT_VALUE_C	((cpu_reg[REG_F]&FLAG_BIT_C)>>4)
#define FLAG_VALUE_Z		(cpu_reg[REG_F]&FLAG_BIT_Z)
#define FLAG_VALUE_N		(cpu_reg[REG_F]&FLAG_BIT_N)
#define FLAG_VALUE_H		(cpu_reg[REG_F]&FLAG_BIT_H)
#define FLAG_VALUE_C		(cpu_reg[REG_F]&FLAG_BIT_C)
#define FLAG_SET_Z(a)		(a?0:FLAG_BIT_Z)

#define FLAG_SET_ADD_H(a,b)	(((a&0x0f)+(b&0x0f)) > 0x0f ? FLAG_BIT_H : 0x00)
#define FLAG_SET_ADD_C(a,b)	((a+b) > 0xff ? FLAG_BIT_C : 0x00)
#define FLAG_SET_ADC_H(a,b,c)	(((a&0x0f)+(b&0x0f)+c) > 0x0f ? FLAG_BIT_H : 0x00)
#define FLAG_SET_ADC_C(a,b,c)	((a+b+c) > 0xff ? FLAG_BIT_C : 0x00)
#define FLAG_SET_SUB_H(a,b)	((b&0x0f) > (a&0x0f) ? FLAG_BIT_H : 0x00)
#define FLAG_SET_SUB_C(a,b)	(b > a ? FLAG_BIT_C : 0x00)
#define FLAG_SET_SBC_H(a,b,c)	(((b&0x0f)+c) > (a&0x0f) ? FLAG_BIT_H : 0x00)
#define FLAG_SET_SBC_C(a,b,c)	((b+c) > a ? FLAG_BIT_C : 0x00)
#define FLAG_SET_ADD_H_16(a,b)	((a&0x0fff)+(b&0x0fff) > 0x0fff ? FLAG_BIT_H : 0x00)
#define FLAG_SET_ADD_C_16(a,b)	((a+b) > 0xffff ? FLAG_BIT_C : 0x00)

#define CPU_INTERRUPT_VBLANK	0x01
#define CPU_INTERRUPT_LCDC	0x02
#define CPU_INTERRUPT_TIMER	0x04
#define CPU_INTERRUPT_SERIAL	0x08
#define CPU_INTERRUPT_JOYPAD	0x10

int cpu_reset();
int cpu_step();
void cpu_op_cb();
void cpu_op_unknown();
void cpu_interrupt(int interrupt);
void cpu_do_interrupt();

int cpu_ime;
int cpu_ima;
int cpu_ie;
int cpu_halt;
int cpu_stop;
int cpu_op_cycles;
unsigned short cpu_reg_sp;
unsigned short cpu_reg_pc;
unsigned char cpu_reg[8];

#endif
