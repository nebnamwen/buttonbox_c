# Makefile for buttonbox

# Define the variables conventionally used for the C compiler and linker
CFLAGS = $(shell sdl2-config --cflags)
LDLIBS = $(shell sdl2-config --libs) -lm
CC     = gcc

# Rule to make buttonbox.  Note that on Linux Mint, and probably most Linuxes,
#	libraries need to come after the object files that use them.  That's
#	so that the linker can load only the functions that are actually used.
buttonbox: keyboard.c synthesis.c config.c display.c main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LDLIBS)
