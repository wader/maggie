/*
 * sdl.c
 *	 Video and input interface for the SDL library
 * 
 * (c)2001 Mattias Wadman
 */

#include <unistd.h>
#include "SDL.h"

#include "lcd.h"
#include "maggie.h"
#include "joypad.h"


SDL_Surface *screen;

int video_init()
{
	SDL_Color palette[4];
	char drivername[32];
	
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		maggie_error("ERRRO: SDL_Init: %s\n",SDL_GetError());
		return FALSE;
	}

	SDL_WM_SetCaption("Maggie",NULL);
	screen = SDL_SetVideoMode(160,144,8,SDL_SWSURFACE);

	/*palette[0].r = palette[0].g = palette[0].b = 240;
	palette[1].r = palette[1].g = palette[1].b = 160;
	palette[2].r = palette[2].g = palette[2].b = 80;
	palette[3].r = palette[3].g = palette[3].b = 0;*/
	
        palette[0].r = 240;
        palette[0].g = 140;
        palette[0].b = 0;
        
        palette[1].r = 200;
        palette[1].g = 100;
        palette[1].b = 0;
    
        palette[2].r = 160;
        palette[2].g = 60;
        palette[2].b = 0;
        
        palette[3].r = 0;
        palette[3].g = 0;
        palette[3].b = 0;
	
        SDL_SetColors(screen,palette,0,4);
	
	SDL_VideoDriverName(drivername,32);
	if(strcmp(drivername,"x11") != 0)
		SDL_ShowCursor(FALSE);

	return TRUE;
}

void video_uninit()
{
	SDL_Quit();
}

void video_update()
{
	SDL_LockSurface(screen);
	memcpy((Uint8*)screen->pixels,lcd_bitmap,0x5a00); // will noly work with 8 bit depth buffer!?
	SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen,0,0,0,0); // all zero makes it update whole screen
}

int input_init()
{
	return TRUE;
}

void input_uninit()
{
}

int input_update()
{
	static SDL_Event event;
	int key,i,state;
	static int translation[][2] = 
	{
		{SDLK_BACKSPACE,JOYPAD_KEY_BACKSPACE},
		{SDLK_TAB,	JOYPAD_KEY_TAB},
		{SDLK_RETURN,	JOYPAD_KEY_RETURN},
		{SDLK_ESCAPE,	JOYPAD_KEY_ESC},
		{SDLK_SPACE,	JOYPAD_KEY_SPACE},
		{SDLK_LALT,	JOYPAD_KEY_ALT},
		{SDLK_RALT,	JOYPAD_KEY_ALT},
		{SDLK_LCTRL,	JOYPAD_KEY_CTRL},
		{SDLK_RCTRL,	JOYPAD_KEY_CTRL},
		{SDLK_LSHIFT,	JOYPAD_KEY_SHIFT},
		{SDLK_RSHIFT,	JOYPAD_KEY_SHIFT},
		{SDLK_UP,	JOYPAD_KEY_UP},
		{SDLK_DOWN,	JOYPAD_KEY_DOWN},
		{SDLK_LEFT,	JOYPAD_KEY_LEFT},
		{SDLK_RIGHT,	JOYPAD_KEY_RIGHT},
		{0,0}
	};

	key = event.key.keysym.sym;

	if(SDL_PollEvent(&event))
	{
		if(event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
			return 0;
		for(i=0;translation[i][0];i++)
			if(translation[i][0] == event.key.keysym.sym)
			{
				key = translation[i][1];
				break;
			}
	}

	state = (event.type == SDL_KEYDOWN ? JOYPAD_KEY_STATE_DOWN : JOYPAD_KEY_STATE_UP);

	return key | state;	
}

