int SamplesPerSecond = 44100;

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

instrument_t instrument[NUM_INSTS];

typedef struct {
  char instrument;
  float frequency;
  uint32_t onset;
  uint32_t offset;
  node_state_t state[NUM_NODES + 1];
  uint32_t zeros;
} note_t;

note_t notes[128];

#define NO_NODE 0

#define SINE 1
#define SQUARE 2
#define TRIANGLE 3
#define SAWTOOTH 4
#define NOISE 5

#define ENVELOPE 10

#define MIX 11
#define EXP 12

#define LOWPASS 13
#define HIGHPASS 14

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

float envelopeValue(float attack, float decay, float sustain, float release, int32_t index, int32_t held) {
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

  if (released > release) {
    return 0.0;
  }

  float seconds = 1.0 * lookup / SamplesPerSecond;
  if (seconds < attack) {
    value = seconds / attack;
  }
  else if (seconds < attack + decay) {
    float lerp = (seconds - attack) / decay;
    value = (1.0 - lerp) + lerp * sustain;
  }
  else {
    value = sustain;
  }

  if (released > 0) {
    value *= 1.0 - released / release;
  }

  return value;
}

#define NOTE notes[n]

#define INST instrument[NOTE.instrument]

#define INPUT(j) (node.input[j].addr == CONST_IN ? node.input[j].val : \
		  node.input[j].addr == PITCH_IN ? NOTE.frequency :    \
		  NOTE.state[node.input[j].addr].out)

#define STATE NOTE.state[i]

int16_t sampleValue(char n, uint32_t index) {
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
      {
	int32_t rel_index = index - NOTE.onset;
	int32_t held = NOTE.offset - NOTE.onset;
	STATE.out = envelopeValue(INPUT(0), INPUT(1), INPUT(2), INPUT(3), rel_index, held);
	if (rel_index == 0 || rel_index == held) {
	  STATE.f = STATE.out;
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
  int32_t value = 32700 * INST.volume * NOTE.state[INST.max_node].out;
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
  return index - NOTE.onset > NOTE_OFF_THRESHOLD && NOTE.zeros > NOTE_OFF_THRESHOLD;
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
  for (int i = 0; i < NUM_INSTS; i++) {
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

  for (int n = 0; n < 128; n++) {
    clearNote(n);
  }
}
