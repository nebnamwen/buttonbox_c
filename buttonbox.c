#include "SDL.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "time.h"
#include "math.h"

// keyboard

typedef struct {
  char origin;
  char transpose;
  char layout;
} keyboard_t;

keyboard_t keyboard = { 0x00, 29, 0x72 };
signed char keygrid[128];

void initKeygrid() {

  for (int i = 0; i < 128; i++) {
    keygrid[i] = -1;
  }

  keygrid[SDLK_z] = 0x00;
  keygrid[SDLK_x] = 0x01;
  keygrid[SDLK_c] = 0x02;
  keygrid[SDLK_v] = 0x03;
  keygrid[SDLK_b] = 0x04;
  keygrid[SDLK_n] = 0x05;
  keygrid[SDLK_m] = 0x06;
  keygrid[SDLK_COMMA] = 0x07;
  keygrid[SDLK_PERIOD] = 0x08;
  keygrid[SDLK_SLASH] = 0x09;

  keygrid[SDLK_a] = 0x10;
  keygrid[SDLK_s] = 0x11;
  keygrid[SDLK_d] = 0x12;
  keygrid[SDLK_f] = 0x13;
  keygrid[SDLK_g] = 0x14;
  keygrid[SDLK_h] = 0x15;
  keygrid[SDLK_j] = 0x16;
  keygrid[SDLK_k] = 0x17;
  keygrid[SDLK_l] = 0x18;
  keygrid[SDLK_SEMICOLON] = 0x19;
  keygrid[SDLK_QUOTE] = 0x1A;

  keygrid[SDLK_q] = 0x20;
  keygrid[SDLK_w] = 0x21;
  keygrid[SDLK_e] = 0x22;
  keygrid[SDLK_r] = 0x23;
  keygrid[SDLK_t] = 0x24;
  keygrid[SDLK_y] = 0x25;
  keygrid[SDLK_u] = 0x26;
  keygrid[SDLK_i] = 0x27;
  keygrid[SDLK_o] = 0x28;
  keygrid[SDLK_p] = 0x29;
  keygrid[SDLK_LEFTBRACKET] = 0x2A;
  keygrid[SDLK_RIGHTBRACKET] = 0x2B;

  keygrid[SDLK_1] = 0x30;
  keygrid[SDLK_2] = 0x31;
  keygrid[SDLK_3] = 0x32;
  keygrid[SDLK_4] = 0x33;
  keygrid[SDLK_5] = 0x34;
  keygrid[SDLK_6] = 0x35;
  keygrid[SDLK_7] = 0x36;
  keygrid[SDLK_8] = 0x37;
  keygrid[SDLK_9] = 0x38;
  keygrid[SDLK_0] = 0x39;
  keygrid[SDLK_MINUS] = 0x3A;
  keygrid[SDLK_EQUALS] = 0x3B;
}

int noteForKey(SDL_Keycode keysym) {
  int row = keygrid[keysym] / 16 - keyboard.origin / 16;
  int col = keygrid[keysym] % 16 - keyboard.origin % 16;
  int note = row * (keyboard.layout / 16) + (col - row) * (keyboard.layout % 16) + keyboard.transpose;
  return note;
}

float frequencyForNote(int note_number) {
  return 440 * pow(2, (1.0 * note_number - 69) / 12);
}

// synthesis

int SamplesPerSecond = 44100;

#define SINE 0
#define SQUARE 1
#define TRIANGLE 2
#define SAWTOOTH 3

float waveformValue(char waveform, float frequency, uint32_t index) {
  double phase = fmod(frequency*index/SamplesPerSecond,1.0);

  switch (waveform) {
  case SINE:
    return sin(phase*M_PI*2);
    break;

  case SQUARE:
    return (phase < 0.5) ? 1 : -1;
    break;

  case TRIANGLE:
    return (phase < 0.5) ? (phase * 4 - 1) : (phase * -4 + 3);
    break;

  case SAWTOOTH:
    return phase * 2 - 1;
    break;

  default:
    return 0;
    break;
  }
   
}

typedef struct {
  float attack;
  float decay;
  float sustain;
  float release;
} envelope_t;

typedef struct {
  float volume;
  float pan;
  envelope_t main;
  envelope_t waveform[4];
} instrument_t;

instrument_t default_instrument = {
  0.1,
  0.0,
  { 0.01, 0.2, 0.6, 0.25 },
  { { 0, 0, 0, 999 },
    { 0, 0, 1.0, 999 },
    { 0, 0, 0, 999 },
    { 0, 0, 0, 999 } },
};

typedef struct {
  instrument_t instrument;
  float frequency;
  uint32_t onset;
  uint32_t offset;
} note_t;

float envelopeValue(envelope_t envelope, int32_t index, int32_t held) {
  float value = 0.0;
  float released = 0.0;

  if (index < 0) {
    return 0.0;
  }

  int32_t lookup = index;
  if (held > 0 && index > held) {
    lookup = held;
    released = 1.0 * (index - held) / SamplesPerSecond;
  }

  if (released >= envelope.release) {
    return 0.0;
  }

  float seconds = 1.0 * lookup / SamplesPerSecond;
  if (seconds < envelope.attack) {
    value = seconds / envelope.attack;
  }
  else if (seconds < envelope.attack + envelope.decay) {
    value = 1.0 - ((seconds - envelope.attack) / envelope.decay) * (1.0 - envelope.sustain);
  }
  else {
    value = envelope.sustain;
  }

  if (released > 0) {
    value *= 1.0 - released / envelope.release;
  }

  return value;
}

int16_t sampleValue(note_t note, uint32_t index) {
  int16_t value = 0;

  int32_t rel_index = index - note.onset;
  int32_t held = note.offset - note.onset;

  float main_env_val = envelopeValue(note.instrument.main, rel_index, held);

  if (main_env_val > 0) {
    for (int waveform = 0; waveform <= 3; waveform++) {
      envelope_t wf_env = note.instrument.waveform[waveform];
      if (wf_env.attack + wf_env.decay + wf_env.sustain > 0) {
	value += 32700 * note.instrument.volume * main_env_val * envelopeValue(wf_env, rel_index, held) * waveformValue(waveform, note.frequency, rel_index);
      }
    }
  }

  return value;
}

char isNoteFinished(note_t note, uint32_t index) {
  return note.offset && index - note.offset > note.instrument.main.release * SamplesPerSecond;
}

int main(int argc, char *argv[]) {

  uint32_t RunningSampleIndex = 1;
  int BytesPerSample = sizeof(int16_t) * 2;

  int BufferSamples = 512;

  int16_t SampleOut[4096];
  void *SoundBuffer = (void *)SampleOut;

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  SDL_Window *sdl_win;
  sdl_win = SDL_CreateWindow("Beep", 0, 0, 100, 50, 0);

  SDL_AudioSpec AudioSettings = {0};

  AudioSettings.freq = SamplesPerSecond;
  AudioSettings.format = AUDIO_S16;
  AudioSettings.channels = 2;
  AudioSettings.samples = BufferSamples;

  SDL_OpenAudio(&AudioSettings, 0);

  int TargetQueueBytes = AudioSettings.samples * BytesPerSample;

  SDL_PauseAudio(0);

  initKeygrid();

  note_t notes[128];
  for (int i = 0; i < 128; i++) {
    notes[i].onset = 0;
    notes[i].offset = 0;
  }

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

  SDL_CloseAudio();
  SDL_DestroyWindow(sdl_win);
  SDL_Quit();
}
