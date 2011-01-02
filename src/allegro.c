/*
 * allegro.c
 *	 Video and input interface for the Allegro library
 * 
 * (c)2001 Mattias Wadman
 */

#include <allegro.h>

#include "lcd.h"
#include "maggie.h"
#include "joypad.h"

int keycode,state;

int video_init()
{
	PALETTE pal;
	
	allegro_init();
	set_color_depth(8);
	set_gfx_mode(GFX_VGA,320,200,320,200);

	pal[0].r = pal[0].g = pal[0].b = 240;
	pal[1].r = pal[1].g = pal[1].b = 160;
	pal[2].r = pal[2].g = pal[2].b = 80;
	pal[3].r = pal[3].g = pal[3].b = 0;
	set_palette(pal);
	
	return TRUE;
}

void video_uninit()
{
	allegro_exit();
}

void video_update()
{
	int x,y;

	for(y=0;y < 144;y++)
		for(x=0;x < 160;x++)
			putpixel(screen,80+x,28+y,*(lcd_bitmap+(y*160)+x));
}

void keyboard_handler(int k)
{
	state = (k&0x80 ? JOYPAD_KEY_STATE_UP : JOYPAD_KEY_STATE_DOWN );
	keycode = k&0x7f;
}

int input_init()
{
	install_keyboard();
	keyboard_lowlevel_callback = keyboard_handler;
	return TRUE;
}

void input_uninit()
{
}

int input_update()
{
	int i;
	int k;
	static int translation[][2] = 
	{
		{KEY_BACKSPACE,	JOYPAD_KEY_BACKSPACE},
		{KEY_TAB,	JOYPAD_KEY_TAB},
		{KEY_ENTER,	JOYPAD_KEY_RETURN},
		{KEY_ESC,	JOYPAD_KEY_ESC},
		{KEY_SPACE,	JOYPAD_KEY_SPACE},
		{KEY_ALT,	JOYPAD_KEY_ALT},
		{KEY_CONTROL,	JOYPAD_KEY_CTRL},
		{KEY_LSHIFT,	JOYPAD_KEY_SHIFT},
		{KEY_RSHIFT,	JOYPAD_KEY_SHIFT},
		{KEY_UP,	JOYPAD_KEY_UP},
		{KEY_DOWN,	JOYPAD_KEY_DOWN},
		{KEY_LEFT,	JOYPAD_KEY_LEFT},
		{KEY_RIGHT,	JOYPAD_KEY_RIGHT},
		{0,0}
	};

	k = keycode;
	for(i=0;translation[i][0];i++)
		if(keycode == translation[i][0])
			k = translation[i][1];

	return k | state;	
}

