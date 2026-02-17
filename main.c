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
  initInstruments();
  initNotes();

  // apply config from command line
  for (trace_t trace = buildTrace("ARGV", 1, 1); trace.word < argc; trace.word++) {
    char *arg = argv[trace.word];
    char *pos = strchr(arg, '=');
    if (strchr("#\n", arg[0]) != NULL) { }
    else if (pos == NULL) { doConfigFile(arg, "", trace); }
    else { doConfigClause(arg, trace); }
  }

  setDefaultInstrumentIfZero();

  initDisplay();

  int quit = 0;
  int stdinline = 0;

  while (! quit) {
    SDL_Event e;
    int keycode = 0;

    char tab_continue;
    char tab_buffer[256];

    while (SDL_PollEvent(&e)){
      switch (e.type) {
      case SDL_EVENT_QUIT:
        quit = 1;
        break;

      case SDL_EVENT_KEY_DOWN:
	keycode = e.key.scancode;
	
	switch(keygrid[keycode]) {
	case KEY_NOTHING:
	  break;

	case KEY_FUNCTION:
	  switch(keycode) {
	  case SDL_SCANCODE_TAB:
	    do {
	      if (fgets(tab_buffer, sizeof(tab_buffer), stdin) != NULL) {
		stdinline++;
		tab_continue = doConfigLine(tab_buffer, buildTrace("STDIN", stdinline, 0));
	      } else if (feof(stdin)) {
		printf("STDIN line %d: End of file: No more config to read!\n", stdinline);
		tab_continue = 0;
	      } else if (ferror(stdin)) {
		printf("STDIN line %d: Error reading from STDIN!\n", stdinline);
		tab_continue = 0;
	      }
	    } while (tab_continue);

	    initNotes();
	    initDisplay();
	    setDefaultInstrumentIfZero();
	    break;

	  case SDL_SCANCODE_F1:
	  case SDL_SCANCODE_F2:
	  case SDL_SCANCODE_F3:
	  case SDL_SCANCODE_F4:
	  case SDL_SCANCODE_F5:
	  case SDL_SCANCODE_F6:
	  case SDL_SCANCODE_F7:
	  case SDL_SCANCODE_F8:
	  case SDL_SCANCODE_F9:
	  case SDL_SCANCODE_F10:
	  case SDL_SCANCODE_F11:
	  case SDL_SCANCODE_F12:
	    keycode = keycode - SDL_SCANCODE_F1 + 1;
	    char clause[] = "file=:F00";
	    snprintf(clause, strlen(clause)+1, "file=:F%d", keycode);

	    doConfigClause(clause, buildTrace("FUNCTION", keycode, 0));
	    setDefaultInstrumentIfZero();
	    initDisplay();

	    break;
	  }

	  break;

	default:
	  if (e.key.repeat == 0) {
	    if (keycode > 0 && keycode < 256 && keygrid[keycode] >= KEY_BUTTON) {
	      notes[keycode].onset = RunningSampleIndex;
	      if (keygrid[keycode] >= 0) {
	      
		notes[keycode].instrument = instrument[instForKeyboard(keyboardForGrid(keygrid[keycode]))];
		notes[keycode].frequency = frequencyForNote(noteForGrid(keygrid[keycode]));

		drawKeyIcon(keygrid[keycode], 1);
	      }
	    }
	  }
	  break;
	}
	break;

      case SDL_EVENT_KEY_UP:
	keycode = e.key.scancode;
	if (keycode > 0 && keycode < 256 && keygrid[keycode] >= KEY_BUTTON) {
	  notes[keycode].offset = RunningSampleIndex;
	  if (keygrid[keycode] >= 0) {
	    drawKeyIcon(keygrid[keycode], 0);
	  }
	}
	break;	
      }
    }

    int BytesToWrite = TargetQueueBytes - SDL_GetAudioStreamQueued(stream);
    int SamplesToWrite = BytesToWrite / BytesPerSample;
    // printf("%i\n", SamplesToWrite);

    for (int SampleIndex = 0;
	 SampleIndex < SamplesToWrite;
	 ++SampleIndex) {
      SampleOut[SampleIndex*2] = 0;
      SampleOut[SampleIndex*2 + 1] = 0;
    }

    for (int n = 0; n < 64; n++) {
      if (keygrid[n] >= 0 && NOTE.onset) {
	for (int SampleIndex = 0;
	     SampleIndex < SamplesToWrite;
	     ++SampleIndex)
	  {
	    float SampleValue = sampleValue(n, RunningSampleIndex + SampleIndex);

	    SampleOut[SampleIndex*2] += SampleValue * (1 - INST.pan);
	    SampleOut[SampleIndex*2 + 1] += SampleValue * (1 + INST.pan);
	  }
      }
    }

    RunningSampleIndex += SamplesToWrite;

    for (int n = 0; n < 64; n++) {
      if (keygrid[n] >= 0 && isNoteFinished(n, RunningSampleIndex)) {
	clearNote(n);
      }
    }

    SDL_PutAudioStreamData(stream, SoundBuffer, BytesToWrite);

    updateDisplay();

    SDL_Delay(500 * TargetQueueBytes / (BytesPerSample * SamplesPerSecond));

#define ESC_NOTE notes[SDL_SCANCODE_ESCAPE]
    if (ESC_NOTE.offset > ESC_NOTE.onset &&
	ESC_NOTE.offset - ESC_NOTE.onset > SamplesPerSecond * 0.5) {
      quit = 1;
    }
  }

  teardownSDL();
}
