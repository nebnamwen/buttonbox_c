SDL_Window *sdl_win;
SDL_Renderer *sdl_ren;
SDL_Texture *sdl_tex;

int BytesPerSample = sizeof(int16_t) * 2;
int TargetQueueBytes;

unsigned char screenpixels[1][1][4];

void setupSDL() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  sdl_win = SDL_CreateWindow("ButtonBox", 0, 0, 100, 50, 0);
  sdl_ren = SDL_CreateRenderer(sdl_win, -1, 0);
  sdl_tex = SDL_CreateTexture(sdl_ren, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, 100, 50);

  memset(screenpixels, 0, 1 * 1 * 4);

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
