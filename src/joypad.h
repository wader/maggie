/*
 * joypad.h
 * 	joypad constants and functions prototypes
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __JOYPAD_H__
#define __JOYPAD_H__

// this is non-standard key codes
#define JOYPAD_KEY_BACKSPACE	0x08
#define JOYPAD_KEY_TAB		0x09
#define JOYPAD_KEY_RETURN	0x0d
#define JOYPAD_KEY_ESC		0x1b
#define JOYPAD_KEY_SPACE	0x20
#define JOYPAD_KEY_ALT		0x100
#define JOYPAD_KEY_CTRL		0x101
#define JOYPAD_KEY_SHIFT	0x102
#define JOYPAD_KEY_UP		0x200
#define JOYPAD_KEY_DOWN		0x201
#define JOYPAD_KEY_LEFT		0x202
#define JOYPAD_KEY_RIGHT	0x203
#define JOYPAD_KEY_STATE_DOWN	0x1000
#define JOYPAD_KEY_STATE_UP	0x2000

#define JOYPAD_RIGHT	0x01
#define JOYPAD_LEFT	0x02
#define JOYPAD_UP	0x04
#define JOYPAD_DOWN	0x08
#define JOYPAD_A	0x10
#define JOYPAD_B	0x20
#define JOYPAD_SELECT	0x40
#define JOYPAD_START	0x80
#define JOYPAD_P14	0x10
#define JOYPAD_P15	0x20

void joypad_write(int key,int state);
int joypad_read();
int joypad_reset();
void joypad_change(int data);
void joypad_update();

unsigned char joypad_state;

#endif
