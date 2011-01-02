/*
 * timer.h
 * 	Timer specific constants and function prototypes
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#define TIMER_CONTROL	0x04

void timer_change(int data);
int timer_reset();
void timer_step(int cycles);

int timer_count;
int timer_count_div;

#endif
