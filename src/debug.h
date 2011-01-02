/*
 * debug.h
 * 	Functions prototypes for the debugger 
 * 
 * (c)2001 Mattias Wadman
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

int debug_symbol_get(char *name);
void debug_symbol_set(char *name, int value);
void debug_variable_show_all();
char *debug_completion_match(const char *text, int state);
char **debug_completion(const char *text, int i, int j);
char *debug_completion_null(const char *text, int i);
int debug_init();
void debug_uninit();
void debug_trace();
void debug_interactive();

int debug_step;

#endif
