#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "time.h"
#include "math.h"

#include "SDL3/SDL.h"

#include "keyboard.c"
#include "synthesis.c"
#include "config.c"
#include "display.c"

int main(int argc, char *argv[]) {

  uint32_t RunningSampleIndex = 1;

  int BufferSamples = 512;
  float SampleOut[4096];
  void *SoundBuffer = (void *)SampleOut;

  setupSDL();

  initKeygrid();
  initNotes();

  // apply config from command line
  for (int i = 1; i < argc; i++) {
    char *arg = argv[i];
    char *pos = strchr(arg, '=');
    if (pos == NULL) { doConfigFile(arg); }
    else { doConfigLine(arg, ""); }
  }

  setDefaultInstrumentIfZero();

  initDisplay();

  int quit = 0;
  int keycode = 0;
  SDL_Event e;

  while (! quit) {
    while (SDL_PollEvent(&e)){
      switch (e.type) {
      case SDL_EVENT_QUIT:
        quit = 1;
        break;

      case SDL_EVENT_KEY_DOWN:
	if (e.key.key == SDLK_ESCAPE) {
	  quit = 1;
	}

	else if (e.key.repeat == 0) {
	  keycode = e.key.scancode;
	  if (keycode > 0 && keycode < 256 && keygrid[keycode] != -1) {
	    // printf("%d\n",keycode);
	    notes[keycode].onset = RunningSampleIndex;
	    if (keygrid[keycode] >= 0) {
	      notes[keycode].instrument = keyboardForGrid(keygrid[keycode]);
	      notes[keycode].frequency = frequencyForNote(noteForGrid(keygrid[keycode]));

	      drawKeyIcon(keygrid[keycode], 1);
	    }
	  }
	}
	break;

      case SDL_EVENT_KEY_UP:
	keycode = e.key.scancode;
	if (keycode > 0 && keycode < 256 && keygrid[keycode] != -1) {
	  notes[keycode].offset = RunningSampleIndex;
	  if (keygrid[keycode] >= 0) {
	    drawKeyIcon(keygrid[keycode], 0);
	  }
	}
	break;
	
      }

    }

    int BytesToWrite = TargetQueueBytes - SDL_GetAudioStreamQueued(stream);
    // printf("%i\n", BytesToWrite);

    for (int SampleIndex = 0;
	 SampleIndex < BytesToWrite / BytesPerSample;
	 ++SampleIndex) {
      SampleOut[SampleIndex*2] = 0;
      SampleOut[SampleIndex*2 + 1] = 0;
    }

    for (int n = 0; n < 64; n++) {
      if (keygrid[n] >= 0 && NOTE.onset) {
	for (int SampleIndex = 0;
	     SampleIndex < BytesToWrite / BytesPerSample;
	     ++SampleIndex)
	  {
	    float SampleValue = sampleValue(n, RunningSampleIndex + SampleIndex);

	    SampleOut[SampleIndex*2] += SampleValue * (1 - INST.pan);
	    SampleOut[SampleIndex*2 + 1] += SampleValue * (1 + INST.pan);
	  }
      }
    }

    RunningSampleIndex += BytesToWrite / BytesPerSample;

    for (int n = 0; n < 64; n++) {
      if (keygrid[n] >= 0 && isNoteFinished(n, RunningSampleIndex)) {
	clearNote(n);
      }
    }

    SDL_PutAudioStreamData(stream, SoundBuffer, BytesToWrite);

    updateDisplay();

    SDL_Delay(500 * TargetQueueBytes / (BytesPerSample * SamplesPerSecond));
  }

  teardownSDL();
}
