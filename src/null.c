/*
 * null.c
 *	 Dummy video and input interface
 * 
 * (c)2001 Mattias Wadman
 */

#include "maggie.h"

int video_init()
{
	return TRUE;
}

void video_uninit()
{
}

void video_update()
{
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
	return 0;
}

