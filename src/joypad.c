/*
 * joypad.c
 *	Handle changes and updates to the joypad register
 * 
 * (c)2001 Mattias Wadman
 */

#include "joypad.h"
#include "memory.h"
#include "cpu.h"
#include "maggie.h"

#ifdef WITH_DEBUG
#include "debug.h"
#endif


void joypad_set_state(int state, int key)
{
	joypad_state = (state&JOYPAD_KEY_STATE_DOWN ? joypad_state|key : joypad_state&~key);
}

/*
 * This functions calls the input interface for updates and preforms
 * various actions.
 */
void joypad_update()
{
	int state,ostate = joypad_state;

	state = input_update();

	switch(state&0x0fff)
	{
		case JOYPAD_KEY_UP: joypad_set_state(state,JOYPAD_UP); break;
		case JOYPAD_KEY_DOWN: joypad_set_state(state,JOYPAD_DOWN); break;
		case JOYPAD_KEY_LEFT: joypad_set_state(state,JOYPAD_LEFT); break;
		case JOYPAD_KEY_RIGHT: joypad_set_state(state,JOYPAD_RIGHT); break;
		case JOYPAD_KEY_SPACE: joypad_set_state(state,JOYPAD_START); break;
		case JOYPAD_KEY_RETURN: joypad_set_state(state,JOYPAD_SELECT); break;
		case JOYPAD_KEY_CTRL: joypad_set_state(state,JOYPAD_A); break;
		case JOYPAD_KEY_ALT: joypad_set_state(state,JOYPAD_B); break;
		case JOYPAD_KEY_ESC:
			      maggie_running = FALSE;
			      break;
#ifdef WITH_DEBUG
		case 'd':
			      debug_interactive();
			      break;
#endif
	}

	// do interrupt on high to low transition, but the joypad_state is
	// internal so we should interrupt on low to high insted.
	if((ostate^joypad_state)&joypad_state)
	{
		cpu_stop = FALSE;
		cpu_interrupt(CPU_INTERRUPT_JOYPAD);
	}
}

/*
 * This functions transforms the current joypad status into the
 * format used in the JOYPAD (P1) register.
 */
void joypad_change(int data)
{
	int pads;

	if(!(data&(JOYPAD_P14|JOYPAD_P15)))
		pads = 0;
	else if(!(data&JOYPAD_P14))
		pads = joypad_state&0x0f;
	else if(!(data&JOYPAD_P15))
		pads = joypad_state>>4;
	else
		pads = 0;

	io_p1 = 0xc0|(data&0x30)|(~pads&0x0f);
}

int joypad_reset()
{
	joypad_state = 0;
	memory_io_write(IO_P1,0x00);

	return TRUE;
}

