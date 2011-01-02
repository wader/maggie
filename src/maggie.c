/*
 * maggie.c
 * 	includes the main function and main emulation loop
 * 
 * (c)2001 Mattias Wadman
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>

#include "maggie.h"
#include "cpu.h"
#include "lcdc.h"
#include "timer.h"
#include "memory.h"
#include "lcd.h"
#include "cartridge.h"
#include "sync.h"

#ifdef WITH_DEBUG
#include "debug.h"
#endif

int maggie_error(const char *format, ...)
{
#ifdef WITH_DEBUG
	char b[512];
	va_list list;

	va_start(list,format);
	vsprintf(b,format,list);
	va_end(list);

	fprintf(stderr,b);
#endif

	return FALSE;
}

void maggie_run()
{
	int c;

	maggie_running = TRUE;
	
	if(maggie_option_debugger)
		debug_interactive();

	while(maggie_running)
	{
		cpu_do_interrupt();
		c = cpu_step();
		lcdc_step(c);
		timer_step(c);
		sync_step(c);
	}
}

int main(int argc, char **argv)
{
	int c;
	
	maggie_option_debugger = FALSE;
	maggie_option_verbose = 0;
	
	fprintf(stderr,
		"Maggie, a GNU GameBoy Emulator - version " MAGGIE_VERSION "\n"
		"Copyright 2001,2002,2003 Mattias Wadman <mattias@sudac.org>\n"
		"License under GNU GPL, see COPYING for more information.\n\n"
		);
	
	while((c = getopt(argc,argv,"hvdr:")) != EOF)
		switch(c)
		{
			case 'h':
				fprintf(stderr,
					"  -h\tThis help\n"
					"  -v\tIncrease verbosity, can be used multiple times\n"
					"  -d\tStart in debugger\n"
					"  You can also use \"-\" as romfile to read from stdin\n\n"
					  );
				return EXIT_SUCCESS;
				break;
			case 'v':
				maggie_option_verbose++;
				break;
			case 'd':
#ifdef WITH_DEBUG
				if(isatty(STDIN_FILENO))
					maggie_option_debugger = TRUE;
				else
					fprintf(stderr,"Must have a terminal on stdin to run debugger, try not to read rom from stdin\n");
#else
				fprintf(stderr,"This binary is not compiled with debug functionality.\n");
				return EXIT_FAILURE;
#endif
				break;
                        case 'r':
                                memory_rom_internal_load(optarg);
                                break;
			default:
				return EXIT_FAILURE;
				break;
		}
	
	if(argv[optind] == NULL)
	{
		printf("Usage: %s [-hvd] romfile\n",argv[0]);
		return EXIT_FAILURE;
	}

	// Init our way out to the world
	if(!video_init())
		return EXIT_FAILURE;
	if(!input_init())
		return EXIT_FAILURE;
	
	// Init things that need memory allocation
	if(!memory_init())
		return EXIT_FAILURE;
	if(!lcd_init())
		return EXIT_FAILURE;

	// Put in the cartridge
	if(!memory_rom_load(argv[optind]))
		return EXIT_FAILURE;
	
	if(!cartridge_init())
		return EXIT_FAILURE;
		
#ifdef WITH_DEBUG
	if(!debug_init())
		return EXIT_FAILURE;
#endif

	// Show cartridge information
	cartridge_show_info();
	
	// Reset emulation system
	if(!cpu_reset())
		return EXIT_FAILURE;
	if(!timer_reset())
		return EXIT_FAILURE;
	if(!memory_reset())
		return EXIT_FAILURE;
	if(!lcdc_reset())
		return EXIT_FAILURE;
	if(!sync_reset())
		return EXIT_FAILURE;

	// Call the main emulation loop
	maggie_run();

#ifdef WITH_DEBUG
	debug_uninit();
#endif
	
	// Free memory
	memory_uninit();
	lcd_uninit();
	
	// Close our way out to the world
	input_uninit();
	video_uninit();
	
	// Everything went fine
	return EXIT_SUCCESS;
}
