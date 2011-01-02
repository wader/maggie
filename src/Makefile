# Targets
MAGGIE_SDL = maggie_sdl
#MAGGIE_ALLEGRO = maggie_allegro.exe
MAGGIE_NULL = maggie_null

# For debugger and debugging
DEBUG_CFLAGS = -g -DWITH_DEBUG
DEBUG_OBJS = debug.o
DEBUG_LIBS = -lreadline

# Compile with code profileing
#PROFILE_CFLAGS = -pg

# How to delete a file
RM = rm -f
#RM = del

CC = gcc
CFLAGS = -Wall -O3 $(DEBUG_CFLAGS) $(PROFILE_CFLAGS)
LIBS = $(DEBUG_LIBS)

OBJS = cpu.o cpu_op.o memory.o lcdc.o timer.o lcd.o joypad.o cartridge.o sync.o maggie.o $(DEBUG_OBJS)

TARGETS = $(MAGGIE_SDL) $(MAGGIE_ALLEGRO) $(MAGGIE_NULL)

all: $(TARGETS)

clean: 
	$(RM) *.o
	$(RM) $(TARGETS)

%.o : %.c
	$(CC) ${CFLAGS} -c $< -o $@

sdl.o: sdl.c joypad.h
	$(CC) $(CFLAGS) $(shell sdl-config --cflags) -c sdl.c -o sdl.o

allegro.o: allegro.o joypad.h
	$(CC) $(CFLAGS) -c allegro.c -o allegro.o

maggie_sdl: $(OBJS) sdl.o
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) sdl.o -o maggie_sdl $(shell sdl-config --libs)

maggie_allegro.exe: $(OBJS) allegro.o
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) allegro.o -o maggie_allegro.exe -lalleg

maggie_null: $(OBJS) null.o
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) null.o -o maggie_null

