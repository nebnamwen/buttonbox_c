int SamplesPerSecond = 44100;

#define NUM_INSTS 8

#define NUM_INPUTS 5
#define NUM_NODES 26

#define CONST_IN -1
#define PITCH_IN -2

typedef struct {
  char addr;
  float val;
} input_t;

typedef struct {
  char type;
  input_t input[NUM_INPUTS];
} node_t;

typedef struct {
  long int i;
  double f;
  float out;
} node_state_t;

typedef struct {
  float volume;
  float pan;
  node_t node[NUM_NODES + 1];
  char max_node;
} instrument_t;

instrument_t instrument[NUM_INSTS + 1];

typedef struct {
  instrument_t instrument;
  float frequency;
  uint32_t onset;
  uint32_t offset;
  node_state_t state[NUM_NODES + 1];
  uint32_t zeros;
} note_t;

note_t notes[256];

#define NO_NODE 0

#define SINE 1
#define SQUARE 2
#define TRIANGLE 3
#define SAWTOOTH 4
#define NOISE 5

#define ENVELOPE 10
#define ENV_LSHIFT 11
#define ENV_RSHIFT 12
#define ENV_SPACE 13

#define MIX 16
#define EXP 17

#define LOWPASS 18
#define HIGHPASS 19

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

float waveformValue(char waveform, float phase, uint32_t index) {
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
    phase = phase * 12;
    uint32_t seed = (index * 12) + (int)phase;
    float lerp = phase - (int)phase;
    float result = ((noiseHash(seed) % NOISE_RES) * (1.0 - lerp) / NOISE_RES +
		    (noiseHash(seed + 1) % NOISE_RES) * lerp / NOISE_RES);
    return result;
    break;

  default:
    return 0;
    break;
  }
   
}

#define NOTE notes[n]
#define KEY notes[key]

#define INST NOTE.instrument

#define INPUT(j) (node.input[j].addr == CONST_IN ? node.input[j].val : \
		  node.input[j].addr == PITCH_IN ? NOTE.frequency :    \
		  NOTE.state[node.input[j].addr].out)

#define STATE NOTE.state[i]

float sampleValue(char n, uint32_t index) {
  for (int i = 1; i <= INST.max_node; i++) {
    node_t node = INST.node[i];

    switch(node.type) {

    case SINE:
    case SQUARE:
    case TRIANGLE:
    case SAWTOOTH:
    case NOISE:
      {
	STATE.f += INPUT(1)/SamplesPerSecond;
	if (STATE.f > 1.0) {
	  STATE.f -= 1.0;
	  STATE.i += 1;
	}
	STATE.out = INPUT(0) * waveformValue(node.type, STATE.f, STATE.i);
	break;
      }

    case ENVELOPE:
    case ENV_LSHIFT:
    case ENV_RSHIFT:
    case ENV_SPACE:
      {
	float attack = INPUT(0);
	float decay = INPUT(1);
	float sustain = INPUT(2);
	float release = INPUT(3);
	float delay = INPUT(4);

	int32_t delay_samples = delay * SamplesPerSecond;

	unsigned char key;
	switch(node.type) {
	case ENVELOPE:
	  key = n;
	  break;
	  
	case ENV_LSHIFT:
	  key = SDL_SCANCODE_LSHIFT;
	  break;
	  
	case ENV_RSHIFT:
	  key = SDL_SCANCODE_RSHIFT;
	  break;
	  
	case ENV_SPACE:
	  key = SDL_SCANCODE_SPACE;
	  break;
	}
	
	int32_t onset = KEY.onset + delay_samples;
	int32_t offset = KEY.offset;

	if (KEY.onset < KEY.offset && onset > offset) {
	  onset = offset;
	}
	
	if (index == onset || index == offset) {
	  STATE.i = index;
	  STATE.f = STATE.out;
	}

	if (index >= onset && onset > offset) {
	  float seconds = 1.0 * (index - onset) / SamplesPerSecond;
	  if (seconds < attack) {
	    float lerp = seconds / attack;
	    STATE.out = (1.0 - lerp)*STATE.f + lerp;
	  }
	  else if (seconds < attack + decay) {
	    float lerp = (seconds - attack) / decay;
	    STATE.out = (1.0 - lerp) + lerp * sustain;
	  }
	  else {
	    STATE.out = sustain;
	  }
	}
	else {
	  float released = 1.0 * (index - offset) / SamplesPerSecond;
	  if (released < release) {
	    float lerp = 1.0 - released / release;
	    STATE.out = lerp * STATE.f;
	  }
	  else {
	    STATE.out = 0;
	  }
	}

	break;
      }

    case MIX:
      STATE.out = INPUT(0) * INPUT(1) + INPUT(2) * INPUT(3);
      break;

    case EXP:
      STATE.out = INPUT(0) * pow(INPUT(1), INPUT(2) * INPUT(3));
      break;

    case LOWPASS:
      {
	float alpha = M_PI*2 * INPUT(1)/SamplesPerSecond;
	STATE.f *= 1.0 - alpha;
	STATE.f += alpha * INPUT(0);
	STATE.out = STATE.f;
	break;
      }

    case HIGHPASS:
      {
	float alpha = SamplesPerSecond / (M_PI*2 * INPUT(1));
	alpha = alpha/(alpha + 1.0);
	STATE.out = alpha * (STATE.out + INPUT(0) - STATE.f);
	STATE.f = INPUT(0);
	break;
      }

    case NO_NODE:
    default:
      STATE.out = 0;
    }
  }
  float value = INST.volume * NOTE.state[INST.max_node].out;
  if (value == 0) {
    NOTE.zeros += 1;
  }
  else {
    NOTE.zeros = 0;
  }

  return value;
}

#define NOTE_OFF_THRESHOLD 0.25 * SamplesPerSecond

char isNoteFinished(char n, uint32_t index) {
  return NOTE.offset > NOTE.onset &&
    index - NOTE.onset > NOTE_OFF_THRESHOLD &&
    NOTE.zeros > NOTE_OFF_THRESHOLD;
}

void clearInstrument(int i) {
  instrument[i].volume = 0.2;
  instrument[i].pan = 0;
  instrument[i].max_node = 0;
  for (int j = 0; j <= NUM_NODES; j++) {
    instrument[i].node[j].type = NO_NODE;
    for (int k = 0; k < NUM_INPUTS; k++) {
      instrument[i].node[j].input[k].addr = CONST_IN;
      instrument[i].node[j].input[k].val = 0;
    }
  }
}

void initInstruments() {
  for (int i = 0; i <= NUM_INSTS; i++) {
    clearInstrument(i);
  }
}

void clearNote(int n) {
  NOTE.onset = 0;
  NOTE.offset = 0;
  NOTE.zeros = 0;
  for (int i = 0; i <= NUM_NODES; i++) {
    STATE.i = 0;
    STATE.f = 0;
    STATE.out = 0;
  }
}

void initNotes() {
  for (int n = 0; n < 256; n++) {
    clearNote(n);
  }
}
