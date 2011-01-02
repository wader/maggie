/*
 * maggie.h
 * 	Functions prototypes for main loop, video interface and input interface
 * 
 * (c)2001 Mattias Wadman
 */

#ifndef __MAGGIE_H__
#define __MAGGIE_H__

#ifndef FALSE
	#define FALSE 0
	#define TRUE (!FALSE)
#endif

#define MAGGIE_VERSION "0.14"

int video_init();
int input_init();
void video_uninit();
void input_uninit();

int input_update();
void video_update();

int maggie_error(const char *format, ...);
void maggie_run();

int maggie_running;
int maggie_option_debugger;
int maggie_option_verbose;

#endif
