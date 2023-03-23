int SamplesPerSecond = 44100;

#define MAIN 0

#define SINE 1
#define SQUARE 2
#define TRIANGLE 3
#define SAWTOOTH 4
#define NOISE 5

#define NUM_WAVEFORMS 5

#define NOISE_RES 32768

// from http://burtleburtle.net/bob/hash/integer.html
int32_t noiseHash(uint32_t a) {
  a = (a ^ 61) ^ (a >> 16);
  a = a + (a << 3);
  a = a ^ (a >> 4);
  a = a * 0x27d4eb2d;
  a = a ^ (a >> 15);
  return (int32_t)a;
}

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

  case NOISE:
    phase = 0;
    double period = 1.0 * SamplesPerSecond / (frequency * 12);
    uint32_t seed = index / period;
    phase = (index - seed * period) * 1.0 / period;
    float result = ((noiseHash(seed) % NOISE_RES) * (1.0 - phase) / NOISE_RES +
		    (noiseHash(seed + 1) % NOISE_RES) * phase / NOISE_RES);
    return result;
    break;

  default:
    return 0;
    break;
  }
   
}

typedef struct {
  float attack;
  float peak;
  float decay;
  float sustain;
  float release;
} envelope_t;

typedef struct {
  float volume;
  float pan;
  envelope_t envelope[NUM_WAVEFORMS + 1];
} instrument_t;

instrument_t default_instrument = {
  0.2,
  0.0,
  {
    { 0, 1, 0, 1, 0.25 }, // MAIN ENVELOPE
    { 0.02, 1, 0.7, 0.6, 999 }, // SINE
    { 0, 0, 0, 0, 999 }, // SQUARE
    { 0, 0, 0, 0, 999 }, // TRIANGLE
    { 0.3, 0.1, 0.5, 0.6, 999 }, // SAWTOOTH
    { 0, 1, 0.1, 0, 0.1 }, // NOISE
  },
};

typedef struct {
  instrument_t instrument;
  float frequency;
  uint32_t onset;
  uint32_t offset;
} note_t;

note_t notes[128];

void initNotes() {
  for (int i = 0; i < 128; i++) {
    notes[i].onset = 0;
    notes[i].offset = 0;
  }
}

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
    value = envelope.peak * seconds / envelope.attack;
  }
  else if (seconds < envelope.attack + envelope.decay) {
    value = envelope.peak - ((seconds - envelope.attack) / envelope.decay) * (envelope.peak - envelope.sustain);
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

  float main_env_val = envelopeValue(note.instrument.envelope[MAIN], rel_index, held);

  if (main_env_val > 0) {
    for (int waveform = 1; waveform <= NUM_WAVEFORMS; waveform++) {
      envelope_t wf_env = note.instrument.envelope[waveform];
      if ((wf_env.attack + wf_env.decay) * wf_env.peak + wf_env.sustain > 0) {
	float wf_env_val = envelopeValue(wf_env, rel_index, held);
	if (wf_env_val > 0) {
	  value += 32700 * note.instrument.volume * main_env_val * wf_env_val * waveformValue(waveform, note.frequency, rel_index);
	}
      }
    }
  }

  return value;
}

char isNoteFinished(note_t note, uint32_t index) {
  return note.offset && index - note.offset > note.instrument.envelope[MAIN].release * SamplesPerSecond;
}
