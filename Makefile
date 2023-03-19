buttonbox: keyboard.c synthesis.c display.c main.c
	gcc `sdl2-config --cflags --libs` -o buttonbox main.c
