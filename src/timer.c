/*
 * timer.c
 *	 Handle the timer that makes periodic interrupts
 *	 Also hanle the increment of the DIV register
 * 
 * (c)2001 Mattias Wadman
 */

#include "timer.h"
#include "memory.h"
#include "cpu.h"
#include "maggie.h"

void timer_change(int data)
{
	io_tac = data|0xf8;
}

int timer_reset()
{
	timer_count = 0;
	timer_count_div = 0;
	io_div = 0x23;
	io_tima = 0x00;
	io_tma = 0x00;
	io_tac = 0xf8;	

	return TRUE;
}

/*
 * Will increase the TIMA register at different frequencies and will do
 * TIMER interrupt when it overflows.
 *
 * Also increse the DIV register at 16384 Hz
 */
void timer_step(int cycles)
{
	static int timer_table[4] = {256,4,16,64};

	if(io_tac&TIMER_CONTROL)
	{
		timer_count += cycles;
		if(timer_count >= timer_table[io_tac&0x03])
		{
			timer_count -= timer_table[io_tac&0x03];
			if(++io_tima == 0)
			{
				io_tima = io_tma;
				cpu_interrupt(CPU_INTERRUPT_TIMER);
			}
		}
	}
	
	timer_count_div += cycles;
	if(timer_count_div >= timer_table[3]) // 16384 Hz
	{
		timer_count_div -= timer_table[3];
		io_div++;
	}
}

