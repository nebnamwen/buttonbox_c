char* nexttok(char* start, const char* split) {
  while (*start != '\0' && strchr(split, *start) == NULL) {
    start++;
  }

  if (*start != '\0' && strchr(split, *start) != NULL) {
    *start = '\0';
    start++;
  }

  if (*start == '\0') {
    return NULL;
  }
  else {
    return start;
  }
}

#undef INST
#undef INPUT

int current_inst = 1;

#define KEYB keyboard[current_inst]
#define INST instrument[current_inst]
#define NODE INST.node[node_index]
#define INPUT(j) NODE.input[j]

#define TRACE printf("%s line %d word %d: ", file, linenum, word)

void doConfigClause(char* clause, const char* file, int linenum, int word) {
  char* key = clause;
  char* value = nexttok(clause, "=");

  // select instrument (1 <= int <= NUM_INSTS)
  if (strncmp(key, "ins", 3) == 0) {
    int val = atoi(value);
    if (val < 1 || val > NUM_INSTS) {
      TRACE; printf("Invalid value for instrument number: %s (Should be between 1 and %d)\n", value, NUM_INSTS);
    } else {
      current_inst = val;
      KEYB.is_active = 1;
    }
  }

  // copy instrument (0 <= int <= NUM_INSTS)
  else if (strncmp(key, "cop", 3) == 0) {
    int val = atoi(value);
    if (val < 0 || val > NUM_INSTS) {
      TRACE; printf("Invalid value for source instrument number: %s (Should be between 0 and %d)\n", value, NUM_INSTS);
    } else {
      INST = instrument[val];
      KEYB.layout = keyboard[val].layout;
    }
  }
  
  // set color (cymg)
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
    
  // set split (grid)
  else if (strncmp(key, "spl", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    KEYB.split = val;
  }
  
  // set slant (/ | \ ` ' , .)
  else if (strncmp(key, "sla", 3) == 0) {
    char val = value[0];
    if (strchr("/|\\`',.", val) == NULL) {
      TRACE; printf("Invalid value for keyboard split slant: %s (Should be one of / | \\ ` ' , .)\n", value);
    } else {
      KEYB.slant = val;
    }
  }
  
  // set origin (grid)
  else if (strncmp(key, "ori", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    KEYB.origin = val;
  }
  
  // set transpose (int)
  else if (strncmp(key, "tra", 3) == 0) {
    char val = (char)atoi(value);
    KEYB.transpose = val;
  }

  // set origin (grid)
  else if (strncmp(key, "ori", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    KEYB.origin = val;
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

    if (!is_set && node_index > instrument[current_inst].max_node) {
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
      if (strlen(file)) { printf("(%s): ", file); }
      TRACE; printf("Too many inputs for %s node: %s\n", node_key, value);
    }
  }

  else {
    if (strlen(file)) { printf("(%s): ", file); }
    TRACE; printf("Unknown config key: %s\n", key);
  }
}

char doConfigLine(char* line, const char* file, int linenum) {
  if (line[0] == '>') {
    line++;
    printf("%s", line);
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
	word++;
	doConfigClause(current, file, linenum, word);
      }
      cont = 0;
    }
  } while (line != NULL);

  return cont;
}

void doConfigFile(const char* file) {
  if (strchr("#\n", file[0]) != NULL) { return; }
  char line[256] = { 0 };
  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    printf("Unable to read conf file %s\n", file);
  }

  int linenum = 0;
  while (!feof(fp)) {
    memset(line, 0, 256);
    fgets(line, 256, fp);
    linenum++;
    if (strlen(line)) { doConfigLine(line, file, linenum); }
  }
  fclose(fp);
}

void setDefaultInstrumentIfZero() {
  char buffer[] = "inst=N A=env B=sin:A\n";
  for (int i = 1; i <= NUM_INSTS; i++) {
    if (keyboard[i].is_active && instrument[i].max_node == 0) {
      sprintf(buffer, "inst=%d A=env B=sin:A\n", i);
      doConfigLine(buffer, "INTERNAL", i);
    }
  }
}
