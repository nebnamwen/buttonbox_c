buttonbox: buttonbox.c
	gcc `sdl2-config --cflags --libs` -o buttonbox buttonbox.c
