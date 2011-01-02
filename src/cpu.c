/*
 * cpu.c
 * 	Handles fetch and executions of instructions and interrupt handling.
 * 
 * (c)2001 Mattias Wadman
 */

#include "cpu.h"
#include "cpu_op.h"
#include "cpu_op_table.h"
#include "memory.h"
#include "cartridge.h"
#include "maggie.h"

#ifdef WITH_DEBUG
#include "debug.h"
#endif

/*
 * This function fetch the next opcode and possibly immediate values and sets the cpu_op_p*
 * variables used to pass parameters to the functions executing the actual instruction.
 *
 * The functions to be executed is determined by useing the current opcode as an offset in
 * the cpu_op_table structure table.
 */
int cpu_step()
{
	if(cpu_halt || cpu_stop)
		return 1; // halt takes 1 cycle (increse later, less cpu intensive when sync stuff is added)

#ifdef WITH_DEBUG
	debug_trace();
#endif
	
	cpu_op_current = &cpu_op_table[MEMORY_READ(cpu_reg_pc)];

	// do not increse inside MEMORY_READ macro.
	cpu_reg_pc++;
	
	if(cpu_op_current->parm2_size) // parm2_size seams to be used more frequently (in cpu_op_table)
	{
		cpu_op_p1 = cpu_op_current->parm1_data;
		if(cpu_op_current->parm2_size == 1)
		{
			cpu_op_p2 = MEMORY_READ(cpu_reg_pc);
			cpu_reg_pc++;
		}
		else
		{
			cpu_op_p2_16 = (MEMORY_READ(cpu_reg_pc+1)<<8)+MEMORY_READ(cpu_reg_pc);
			cpu_reg_pc+=2;
		}
	}
	else
	{
		cpu_op_p2 = cpu_op_current->parm2_data;
		if(cpu_op_current->parm1_size)
		{
			if(cpu_op_current->parm1_size == 1)
			{
				cpu_op_p1 = MEMORY_READ(cpu_reg_pc);
				cpu_reg_pc++;
			}
			else
			{
				cpu_op_p1_16 = (MEMORY_READ(cpu_reg_pc+1)<<8)+MEMORY_READ(cpu_reg_pc);
				cpu_reg_pc+=2;
			}
		}
		else
			cpu_op_p1 = cpu_op_current->parm1_data;
	}
	
	cpu_op_cycles = cpu_op_current->cycles;
	
	cpu_op_current->function(); // cpu_op_current will be changed if 0xcb opcode

	return cpu_op_cycles;
}

void cpu_op_cb()
{	
	cpu_op_current = &cpu_op_table_cb[MEMORY_READ(cpu_reg_pc)];
	cpu_reg_pc++;
	cpu_op_p1 = cpu_op_current->parm1_data;
	cpu_op_p2 = cpu_op_current->parm2_data;
	cpu_op_cycles = cpu_op_current->cycles; // set new cycle value
	cpu_op_current->function();
}

void cpu_op_unknown()
{
	// PC have been incresed, so -1 to get opcode address (and unknown 0xcb opcode does not exist)
	maggie_error("WARNING: unknown opcode 0x%.2x at 0x%.4x\n",MEMORY_READ(cpu_reg_pc-1),cpu_reg_pc-1);
}

/*
 * Function is used by other functions to acknowledge interrupts
 */
void cpu_interrupt(int interrupt)
{	
	io_if |= interrupt;
	
	if(cpu_ime && (interrupt & cpu_ie))
		cpu_halt = FALSE;
}

/*
 * This function checks if there are any valid interrupt waiting in priority order
 */
void cpu_do_interrupt()
{
	int i;
	
	if(cpu_ime && (io_if & cpu_ie))
	{
		if((io_if&cpu_ie) & CPU_INTERRUPT_VBLANK)
			i = 0;
		else if((io_if&cpu_ie) & CPU_INTERRUPT_LCDC)
			i = 1;
		else if((io_if&cpu_ie) & CPU_INTERRUPT_TIMER)
			i = 2;
		else if((io_if&cpu_ie) & CPU_INTERRUPT_SERIAL)
			i = 3;
		else if((io_if&cpu_ie) & CPU_INTERRUPT_JOYPAD)
			i = 4;
		else
		{
			maggie_error("WARNING: cpu_do_interrupt: unknown interrupt\n");
			return;
		}
		
		cpu_ime = cpu_ima = FALSE;
		cpu_halt = FALSE;
		memory_stack_push(cpu_reg_pc);
		cpu_reg_pc = 0x40+(i<<3);
		io_if &= ~(1<<i); // reset the current interrupt bit in the IF register
	}
	
	cpu_ime = cpu_ima;
}

int cpu_reset()
{
	cpu_op_p1		= 0;
	cpu_op_p2		= 0;
	cpu_op_p1_16	= 0;
	
	cpu_ime			= FALSE;
	cpu_ima			= FALSE;
	cpu_ie			= 0x00;
	cpu_halt		= FALSE;
	cpu_stop		= FALSE;
	cpu_op_cycles	= 0;
	//cpu_reg_pc		= CARTRIDGE_START;
	cpu_reg_pc		= 0;
	cpu_reg_sp		= 0xfffe;
	cpu_reg[REG_A]	= 0x01; // 0x01 normal gameboy, 0x11 GBC, 0xff GBP
	cpu_reg[REG_F]	= 0xb0;
	REG_SET_16(REG_BC,0x0013);
	REG_SET_16(REG_DE,0x00d8);
	REG_SET_16(REG_HL,0x014d); // 0x0ca5(h) if emu..

	return TRUE;
}

