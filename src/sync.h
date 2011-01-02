/*
 * sync.h
 * 	Time synchronization
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __SYNC_H__
#define __SYNC_H__

int sync_reset();
void sync_step(int cycles);

long sync_count;

#endif
