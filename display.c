SDL_Window *sdl_win;
SDL_Renderer *sdl_ren;
SDL_Texture *sdl_tex;

int BytesPerSample = sizeof(int16_t) * 2;
int TargetQueueBytes;

unsigned char screenpixels[360][1024][4];

void setupSDL() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  sdl_win = SDL_CreateWindow("ButtonBox", 0, 0, 1024, 360, 0);
  sdl_ren = SDL_CreateRenderer(sdl_win, -1, 0);
  sdl_tex = SDL_CreateTexture(sdl_ren, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, 1024, 360);

  memset(screenpixels, 0, 1024 * 360 * 4);

  int BufferSamples = 512;
  SDL_AudioSpec AudioSettings = {0};

  AudioSettings.freq = SamplesPerSecond;
  AudioSettings.format = AUDIO_S16;
  AudioSettings.channels = 2;
  AudioSettings.samples = BufferSamples;

  SDL_OpenAudio(&AudioSettings, 0);

  TargetQueueBytes = AudioSettings.samples * BytesPerSample;

  SDL_PauseAudio(0);
}

void teardownSDL() {
  SDL_CloseAudio();
  SDL_DestroyRenderer(sdl_ren);
  SDL_DestroyTexture(sdl_tex);
  SDL_DestroyWindow(sdl_win);
  SDL_Quit();
}

void drawKeyIcon(char grid, char onoff) {
  int kb = keyboardForGrid(grid);
  int is_white = isWhiteKey(noteForGrid(grid));

  int row = 3 - (grid / 16);
  int halfcol = (grid % 16) * 2 + row;

  // printf("%i %i %i\n", row, halfcol, onoff);

  int start_y = 15 + 80*row;
  int start_x = 12 + 40*halfcol;

  int center_y = start_y + 40;
  int center_x = start_x + 40;

  int outradius = 25 + 10*onoff;
  outradius *= outradius; // square for distance calculation

  int inradius = 20 + 5*onoff;
  inradius *= inradius; // square for distance calculation
  
  for (int dy = 0; dy < 80; dy++) {
    for (int dx = 0; dx < 80; dx++) {
      int target_y = start_y + dy;
      int target_x = start_x + dx;

      int disp_y = target_y - center_y;
      int disp_x = target_x - center_x;

      int pixradius = disp_y * disp_y + disp_x * disp_x;
      
      unsigned char red = 0;
      unsigned char green = 0;
      unsigned char blue = 0;
      
      if (pixradius < outradius) {
	if ((pixradius > inradius) || (disp_x + disp_y) % 2) {
	  switch (keyboard[kb].color) {
	  case COL_GRAY:
	    red = 128;
	    green = 128;
	    blue = 128;
	    break;

	  case COL_RED:
	    red = 200;
	    break;

	  case COL_GREEN:
	    green = 200;
	    break;

	  case COL_YELLOW:
	    red = 200;
	    green = 180;
	    break;

	  case COL_BLUE:
	    green = 75;
	    blue = 255;
	    break;

	  case COL_MAGENTA:
	    red = 200;
	    blue = 255;
	    break;

	  case COL_CYAN:
	    green = 200;
	    blue = 255;
	    break;

	  case COL_BROWN:
	    red = 128;
	    green = 72;
	    break;
	  }
	}
	else {
	  if (is_white) {
	    red = 255;
	    green = 255;
	    blue = 255;
	  }
	}
      }

      screenpixels[target_y][target_x][0] = blue;
      screenpixels[target_y][target_x][1] = green;
      screenpixels[target_y][target_x][2] = red;
    }
  }
}

void initDisplay() {
  for (int keysym = 0; keysym < 128; keysym++) {
    if (keygrid[keysym] != -1) {
      drawKeyIcon(keygrid[keysym], 0);
    }
  }
}

void updateDisplay() {
  SDL_UpdateTexture(sdl_tex, NULL, screenpixels, 1024*4);
  SDL_RenderClear(sdl_ren);
  SDL_RenderCopy(sdl_ren, sdl_tex, NULL, NULL);
  SDL_RenderPresent(sdl_ren);
}
