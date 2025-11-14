char* nexttok(char* start, const char* split) {
  while (*start != '\0' && strchr(split, *start) == NULL) {
    start++;
  }

  if (*start != '\0') {
    *start = '\0';
    start++;
    return start;
  }
  else {
    return NULL;
  }
}

typedef struct {
  const char* file;
  const char* label;
  char* clabel;
  int linenum;
  int word;
  int depth;
} trace_t;

trace_t buildTrace(const char* file, int linenum, int word) {
  trace_t trace = { file, "", NULL, linenum, word, 0 };
  return trace;
}

trace_t buildLabeledTrace(const char* file, const char* label, char* clabel, int depth) {
  trace_t trace = { file, label, clabel, 0, 0, depth };
  return trace;
}

#define TRACE printf("%s line %d word %d: ", trace.file, trace.linenum, trace.word)

#undef INST
#undef INPUT

int current_inst = 1;
int current_keybd = 1;

#define INST instrument[current_inst]
#define KEYB keyboard[current_keybd]
#define NODE INST.node[node_index]
#define INPUT(j) NODE.input[j]

#define MAX_DEPTH 8
#define LABEL_SIZE 32

#define IS_LABEL (trace.clabel == NULL || strncmp(trace.label, trace.clabel, LABEL_SIZE) == 0)

char function_preset_file[256] = { 0 };

void doConfigFile(const char* file, const char* label, trace_t trace);

void doConfigClause(char* clause, trace_t trace) {
  char* key = clause;
  char* value = nexttok(clause, "=");

  if (value == NULL) {
    TRACE; printf("Expected config clause '%s' to contain an '=' between key and value.\n", clause);
    return;
  }

  // set current label
  if (strncmp(key, "lab", 3) == 0) {
    if (trace.clabel == NULL) {
      TRACE; printf("Cannot set a label in this context.\n");
    }
    else if (strncmp(trace.label, "?", 2) == 0) {
      printf("%s\n", value);
    }
    else {
      strncpy(trace.clabel, value, LABEL_SIZE);
    }
  }
  // skip this clause if called label is different from current label
  else if (!IS_LABEL) {
    return;
  }

  // set function key presets file
  else if (strncmp(key, "fun", 3) == 0) {
    if (strlen(value)) {
      strncpy(function_preset_file, value, 256);
    }
    else {
      // if filename is empty set to the current file
      strncpy(function_preset_file, trace.file, 256);
    }
  }
  
  // read another config file recursively
  else if (strncmp(key, "fil", 3) == 0) {
    if (!strlen(value)) {
      TRACE; printf("Expected filename or label for file instruction.\n");
    }
    else if (trace.depth > MAX_DEPTH) {
      TRACE; printf("Maximum file recursion depth (%d) exceeded, not reading file '%s'\n", MAX_DEPTH, value);
    }
    else  {
      char* val2 = nexttok(value, ":");
      if (val2 == NULL) { val2 = value + strlen(value); }
      if (!strlen(value)) {
	if (trace.clabel == NULL) {
	  if (strlen(function_preset_file)) {
	    doConfigFile(function_preset_file, val2, trace);
	  }
	  else {
	    TRACE; printf("No function key preset file configured.\n");
	  }
	}
	else {
	  doConfigFile(trace.file, val2, trace);
	}
      }
      else {
	doConfigFile(value, val2, trace);
      }
    }
  }

  // select instrument (1 <= int <= NUM_INSTS)
  else if (strncmp(key, "ins", 3) == 0) {
    int val = atoi(value);
    if (val < 1 || val > NUM_INSTS) {
      TRACE; printf("Invalid value for instrument number: %s (Should be between 1 and %d)\n", value, NUM_INSTS);
    } else {
      current_inst = val;
    }
  }

  // select keyboard (1 <= int <= NUM_KEYBDS) : set instrument (0 <= int <= NUM_INSTS)
  // instrument 0 means "same as previous keyboard"
  else if (strncmp(key, "key", 3) == 0) {
    char* val2 = nexttok(value, ":");
    if (strlen(value)) {
      int val = atoi(value);
      if (val < 1 || val > NUM_KEYBDS) {
	TRACE; printf("Invalid value for keyboard number: %s (Should be between 1 and %d)\n", value, NUM_KEYBDS);
      } else {
	current_keybd = val;
      }
    }
    if (val2 != NULL && strlen(val2)) {
      int val = atoi(val2);
      if (val < 0 || val > NUM_INSTS) {
	TRACE; printf("Invalid value for instrument number: %s (Should be between 0 and %d)\n", value, NUM_INSTS);
      } else {
	KEYB.instrument = val;
      }
    }
  }

  // copy instrument (0 <= int <= NUM_INSTS)
  else if (strncmp(key, "cop", 3) == 0) {
    int val = atoi(value);
    if (val < 0 || val > NUM_INSTS) {
      TRACE; printf("Invalid value for source instrument number: %s (Should be between 0 and %d)\n", value, NUM_INSTS);
    } else {
      INST = instrument[val];
    }
  }
  
  // set color
  else if (strncmp(key, "col", 3) == 0) {
    if (strcmp(value, "gray") == 0) { KEYB.color = COL_GRAY; }
    else if (strcmp(value, "red") == 0) { KEYB.color = COL_RED; }
    else if (strcmp(value, "green") == 0) { KEYB.color = COL_GREEN; }
    else if (strcmp(value, "yellow") == 0) { KEYB.color = COL_YELLOW; }
    else if (strcmp(value, "blue") == 0) { KEYB.color = COL_BLUE; }
    else if (strcmp(value, "magenta") == 0) { KEYB.color = COL_MAGENTA; }
    else if (strcmp(value, "cyan") == 0) { KEYB.color = COL_CYAN; }
    else if (strcmp(value, "brown") == 0) { KEYB.color = COL_BROWN; }

    else {
      TRACE; printf("Unknown keyboard color: %s (Should be gray/red/green/yellow/blue/magenta/cyan/brown)\n", value);
    }
  }
    
  // set split (grid : [ / | \ ` ' , . ])
  else if (strncmp(key, "spl", 3) == 0) {
    char* val2 = nexttok(value, ":");
    if (strlen(value)) {
      char val = -1;
      if (strlen(value) == 1 && qwerty[value[0]] >= 0) {
	val = qwerty[value[0]];
      }
      else if (strlen(value) > 2 && value[0] == '0' && value[1] == 'x') {
	val = (char)strtol(value, NULL, 0);
      }

      if (val == -1) {
	TRACE; printf("Invalid key grid value: %s (Expected key char or else hex pair starting with '0x')\n", value);
      }
      else {
	KEYB.split = val;
      }
    }
    if (val2 != NULL && strlen(val2)) {
      char val = val2[0];
      if (strchr("/|\\`',.", val) == NULL) {
	TRACE; printf("Invalid value for keyboard split slant: %s (Should be one of / | \\ ` ' , .)\n", value);
      } else {
	KEYB.slant = val;
      }
    }
  }
  
  // set transpose (grid : note)
  else if (strncmp(key, "tra", 3) == 0) {
    char* val2 = nexttok(value, ":");
    if (strlen(value)) {
      char val = -1;
      if (strlen(value) == 1 && qwerty[value[0]] >= 0) {
	val = qwerty[value[0]];
      }
      else if (strlen(value) > 2 && value[0] == '0' && value[1] == 'x') {
	val = (char)strtol(value, NULL, 0);
      }

      if (val == -1) {
	TRACE; printf("Invalid key grid value: %s (Expected key char or else hex pair starting with '0x')\n", value);
      }
      else {
	KEYB.origin = val;
      }
    }
    if (val2 != NULL && strlen(val2)) {
      char val = -1;
      if (strchr("0123456789", val2[0]) != NULL) {
	// MIDI note number
	val = (char)atoi(val2);
      }
      else if (strchr("ABCDEFG", val2[0]) != NULL) {
	// note name like C4, Bb6, F#2 (C4 is middle C)
	char* scale = "C_D_EF_G_A_B";
	int note = strchr(scale, val2[0]) - scale;
	val2++;
	if (strchr("b#", val2[0]) != NULL) {
	  char* accs = "b_#";
	  note += strchr(accs, val2[0]) - accs - 1;
	  val2++;
	}
	int octave = atoi(val2);
	note += 12 * (octave + 1);

	val = note;
      }

      if (val == -1) {
	TRACE; printf("Invalid transpose note: %s (Expected MIDI note number 0 <= n <= 128 or note name like C4 / Bb6 / F#2 etc.)\n", val2);
      }
      else {
	KEYB.transpose = val;
      }
    }
  }

  // set layout (grid)
  else if (strncmp(key, "lay", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    KEYB.layout = val;
  }

  // set volume (float)
  else if (strncmp(key, "vol", 3) == 0) {
    float val = atof(value);
    if (val < 0 || val > 1) {
      TRACE; printf("Invalid value for instrument volume: %s (Should be between 0 and 1)\n", value);
    } else {
      INST.volume = val;
    }
  }

  // set pan (float)
  else if (strncmp(key, "pan", 3) == 0) {
    float val = atof(value);
    if (val < -1 || val > 1) {
      TRACE; printf("Invalid value for instrument pan: %s (Should be between -1 and 1)\n", value);
    } else {
      INST.pan = val;
    }
  }

  // synthesis node definition
  else if (strlen(key) == 1 && key[0] >= 'A' && key[0] <= 'Z') {
    char node_index = key[0] - 'A' + 1;
    char* node_key = value;
    value = nexttok(value, ":");
    char is_set = (strncmp(node_key, "set", 3) == 0);

    if (!is_set && NODE.type != NO_NODE) {
      TRACE; printf("Node %s of instrument %d redefined\n", key, current_inst);
    }

    if (is_set && NODE.type == NO_NODE) {
      TRACE; printf("Node %s of instrument %d not defined\n", key, current_inst);
    }

    if (!is_set && node_index > INST.max_node) {
      INST.max_node = node_index;
    }

    int expected_inputs = 0;

    if (is_set) { expected_inputs = NUM_INPUTS; }

#define ENV_DEFAULT(t) NODE.type = t; \
    expected_inputs = 5;	      \
    INPUT(0).val = 0.05;	      \
    INPUT(1).val = 0.05;	      \
    INPUT(2).val = 1.0;		      \
    INPUT(3).val = 0.1;		      \
    INPUT(4).val = 0.0;
    
    else if (strncmp(node_key, "env", 3) == 0) { ENV_DEFAULT(ENVELOPE) }
    else if (strncmp(node_key, "els", 3) == 0) { ENV_DEFAULT(ENV_LSHIFT) }
    else if (strncmp(node_key, "ers", 3) == 0) { ENV_DEFAULT(ENV_RSHIFT) }
    else if (strncmp(node_key, "esp", 3) == 0) { ENV_DEFAULT(ENV_SPACE) }

#define WF_DEFAULT(t,d) NODE.type = t; expected_inputs = 2; INPUT(0).val = d; INPUT(1).addr = PITCH_IN;

    else if (strncmp(node_key, "sin", 3) == 0) { WF_DEFAULT(SINE,1) }
    else if (strncmp(node_key, "squ", 3) == 0) { WF_DEFAULT(SQUARE,1) }
    else if (strncmp(node_key, "tri", 3) == 0) { WF_DEFAULT(TRIANGLE,1) }
    else if (strncmp(node_key, "saw", 3) == 0) { WF_DEFAULT(SAWTOOTH,1) }
    else if (strncmp(node_key, "noi", 3) == 0) { WF_DEFAULT(NOISE,1) }

    else if (strncmp(node_key, "low", 3) == 0) { WF_DEFAULT(LOWPASS,0) }
    else if (strncmp(node_key, "high", 3) == 0) { WF_DEFAULT(HIGHPASS,0) }

    else if (strncmp(node_key, "mix", 3) == 0) {
      NODE.type = MIX;
      expected_inputs = 4;
      INPUT(0).val = 0.0;
      INPUT(1).val = 1.0;
      INPUT(2).val = 0.0;
      INPUT(3).val = 1.0;
    }
    else if (strncmp(node_key, "exp", 3) == 0) {
      NODE.type = EXP;
      expected_inputs = 4;
      INPUT(0).val = 1.0;
      INPUT(1).val = M_E;
      INPUT(2).val = 0.0;
      INPUT(3).val = 1.0;
    }

    else {
      TRACE; printf("Unknown synthesis node type: %s\n", node_key);
    }

    char* val;
    for (int i = 0; i < expected_inputs; i++) {
      if (value == NULL) { break; }
      val = value;
      value = nexttok(value, ",/");

      if (strlen(val) == 0) { }
      else if (strlen(val) == 1 && val[0] >= 'A' && val[0] <= 'Z') {
	char input_addr = val[0] - 'A' + 1;
	if (input_addr >= node_index) {
	  TRACE; printf("Node %s references later node %s\n", key, val);
	}
	INPUT(i).addr = input_addr;
      }
      else if (val[0] == '_') {
	if (strcmp(val, "_s") == 0) { INPUT(i).val = pow(2, 1.0/12); }
	else if (strcmp(val, "_e") == 0) { INPUT(i).val = M_E; }
	else if (strcmp(val, "_p") == 0) { INPUT(i).addr = PITCH_IN; }
	else {
	  TRACE; printf("Unknown special value: %s\n", val);
	}
      }
      else {
	INPUT(i).addr = CONST_IN;
	INPUT(i).val = atof(val);
      }
    }

    if (value != NULL) {
      TRACE; printf("Too many inputs for %s node: %s\n", node_key, value);
    }
  }

  else {
    TRACE; printf("Unknown config key: %s\n", key);
  }
}

char doConfigLine(char* line, trace_t trace) {
  if (line[0] == '>') {
    line++;
    if (IS_LABEL) {
      printf("%s", line);
    }
  }

  if (line[0] == '\n') { return 0; }
  if (line[0] == '#') { return strlen(line) > 1 && line[strlen(line)-2] == '\\'; }

  char cont = 0;
  int word = 0;
  do {
    char* current = line;
    do { line = nexttok(line, " \t\n"); } while (line != NULL && strchr(" \t\n", line[0]) != NULL);
    if (strcmp(current, "\\") == 0) {
      cont = 1;
    } else {
      if (strlen(current)) {
	trace.word++;
	doConfigClause(current, trace);
      }
      cont = 0;
    }
  } while (line != NULL);

  return cont;
}

void doConfigFile(const char* file, const char* label, trace_t trace) {
  char clabel[LABEL_SIZE+1] = { 0 };
  char line[256] = { 0 };
  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    TRACE; printf("Unable to read conf file '%s'\n", file);
    return;
  }

  trace = buildLabeledTrace(file, label, clabel, trace.depth + 1);
  while (!feof(fp)) {
    memset(line, 0, 256);
    fgets(line, 256, fp);
    trace.linenum++;
    if (strlen(line)) { doConfigLine(line, trace); }
  }
  fclose(fp);
}

void setDefaultInstrumentIfZero() {
  char buffer[] = "inst=N A=env B=sin:A\n";
  if (keyboard[1].instrument == 0) {
    keyboard[1].instrument = 1;
  }

  for (int i = 1; i <= NUM_KEYBDS; i++) {
    if (keyboard[i].instrument != 0 && instrument[keyboard[i].instrument].max_node == 0) {
      sprintf(buffer, "inst=%d A=env B=sin:A\n", keyboard[i].instrument);
      doConfigLine(buffer, buildTrace("INTERNAL", i, 0));
    }
  }
}
