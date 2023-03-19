int main(int argc, char *argv[]) {

  uint32_t RunningSampleIndex = 1;

  int BufferSamples = 512;
  int16_t SampleOut[4096];
  void *SoundBuffer = (void *)SampleOut;

  setupSDL();

  SDL_PauseAudio(0);

  initKeygrid();
  initNotes();

  int quit = 0;
  SDL_Event e;
  SDL_Keycode keysym = 0;

  while (! quit) {
    while (SDL_PollEvent(&e)){
      switch (e.type) {
      case SDL_QUIT:
        quit = 1;
        break;

      case SDL_KEYDOWN:
	if (e.key.keysym.sym == SDLK_ESCAPE) {
	  quit = 1;
	}

	else if (e.key.repeat == 0) {
	  keysym = e.key.keysym.sym;
	  if (keysym > 0 && keysym < 128 && keygrid[keysym] != -1) {
	    int note_number = (keygrid[keysym] / 16) * 5 + (keygrid[keysym] % 16) * 2;
	    notes[keysym].instrument = default_instrument;
	    notes[keysym].frequency = frequencyForNote(noteForKey(keysym));
	    notes[keysym].onset = RunningSampleIndex;
	    notes[keysym].offset = 0;
	  }
	}
	break;

      case SDL_KEYUP:
	keysym = e.key.keysym.sym;
	if (keysym > 0 && keysym < 128 && keygrid[keysym] != -1) {
	  notes[keysym].offset = RunningSampleIndex;
	}
	break;
	
      }

    }

    int BytesToWrite = TargetQueueBytes - SDL_GetQueuedAudioSize(1);
    // printf("%i\n", BytesToWrite);

    for (int SampleIndex = 0;
	 SampleIndex < BytesToWrite / BytesPerSample;
	 ++SampleIndex) {
      SampleOut[SampleIndex*2] = 0;
      SampleOut[SampleIndex*2 + 1] = 0;
    }

    for (int n = 0; n < 128; n++) {
      if (notes[n].onset) {
	for (int SampleIndex = 0;
	     SampleIndex < BytesToWrite / BytesPerSample;
	     ++SampleIndex)
	  {
	    int16_t SampleValue = sampleValue(notes[n], RunningSampleIndex + SampleIndex);

	    SampleOut[SampleIndex*2] += SampleValue;
	    SampleOut[SampleIndex*2 + 1] += SampleValue;
	  }
      }
    }

    RunningSampleIndex += BytesToWrite / BytesPerSample;

    for (int n = 0; n < 128; n++) {
      if (isNoteFinished(notes[n], RunningSampleIndex)) {
	notes[n].onset = 0;
      }
    }

    SDL_QueueAudio(1, SoundBuffer, BytesToWrite);

    SDL_Delay(500 * TargetQueueBytes / (BytesPerSample * SamplesPerSecond));
  }

  teardownSDL();
}
