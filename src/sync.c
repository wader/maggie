/*
 * sync.h
 * 	Time synchronization
 *
 * (c)2001 Mattias Wadman
 */

#include <unistd.h>
#include <sys/time.h>

#include "maggie.h"
#include "sync.h"


long sync_getdelta()
{
	static struct timeval c = {0};
	struct timeval t;
	int s,us;
	
	gettimeofday(&t,NULL);
	us = t.tv_usec-c.tv_usec;
	s = t.tv_sec-c.tv_sec;
	c = t;
	
	return (s ? 1000000+us : us);
}

int sync_reset()
{
	sync_count = 0;
	
	return TRUE;
}

void sync_step(int cycles)
{
	sync_count += cycles;

	if(sync_count >= (1<<14)) // every 16384 cycle (64 Hz)
	{
		int d;
		sync_count = 0;
		d = sync_getdelta();
		if(d < 15625)
			usleep(15625-d);
		sync_getdelta(); // do a "reset"
	}
}

