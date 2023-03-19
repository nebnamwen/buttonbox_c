buttonbox: buttonbox.c keyboard.c synthesis.c main.c
	gcc `sdl2-config --cflags --libs` -o buttonbox buttonbox.c
