char* nexttok(char* start, const char* split) {
  while (*start != '\0' && strchr(split, *start) == NULL) {
    start++;
  }

  while (*start != '\0' && strchr(split, *start) != NULL) {
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

int current_inst = 0;

#define KEYB keyboard[current_inst]
#define INST instrument[current_inst]
#define NODE INST.node[node_index]
#define INPUT(j) NODE.input[j]
  
void doConfigClause(char* clause, const char* file) {
  char* key = clause;
  char* value = nexttok(clause, "=");

  // select instrument (int < NUM_INSTS)
  if (strncmp(key, "ins", 3) == 0) {
    int val = atoi(value);
    if (val < 0 || val >= NUM_INSTS) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for instrument number: %s\n", value);
      printf("(Should be between 0 and %d)\n", NUM_INSTS - 1);
      exit(1);
    }

    current_inst = val;
    KEYB.is_active = 1;
  }

  // copy instrument (int < NUM_INSTS)
  else if (strncmp(key, "cop", 3) == 0) {
    int val = atoi(value);
    if (val < 0 || val >= NUM_INSTS) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for source instrument number: %s\n", value);
      printf("(Should be between 0 and %d)\n", NUM_INSTS - 1);
      exit(1);
    }

    INST = instrument[val];
    KEYB = keyboard[val];
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
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Unknown keyboard color: %s\n", value);
      printf("(Should be gray/red/green/yellow/blue/magenta/cyan/brown)\n");
      exit(1);
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
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for keyboard split slant: %s\n", value);
      printf("(Should be one of / | \\ ` ' , .)\n");
      exit(1);      
    }
    KEYB.slant = val;
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
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for instrument volume: %s\n", value);
      printf("(Should be between 0 and 1)\n");
      exit(1);      
    }
    INST.volume = val;
  }

  // set pan (float)
  else if (strncmp(key, "pan", 3) == 0) {
    float val = atof(value);
    if (val < -1 || val > 1) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for instrument pan: %s\n", value);
      printf("(Should be between -1 and 1)\n");
      exit(1);      
    }
    INST.pan = val;
  }

  // synthesis node definition
  if (strlen(key) == 1 && key[0] >= 'A' && key[0] <= 'Z') {
    char node_index = key[0] - 'A' + 1;
    if (NODE.type != NO_NODE) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Node %s of instrument %d already defined\n", key, current_inst);
      exit(1);
    }

    if (node_index > instrument[current_inst].max_node) {
      INST.max_node = node_index;
    }
    char* node_key = value;
    value = nexttok(value, ":");

    int expected_inputs = 0;
    
    if (strncmp(node_key, "env", 3) == 0) {
      NODE.type = ENVELOPE;
      expected_inputs = 4;
      INPUT(0).val = 0.05;
      INPUT(1).val = 0.05;
      INPUT(2).val = 1.0;
      INPUT(3).val = 0.1;
    }

#define WF_DEFAULT() expected_inputs = 2; INPUT(0).addr = PITCH_IN; INPUT(1).val = 1.0;
    
    else if (strncmp(node_key, "sin", 3) == 0) { NODE.type = SINE; WF_DEFAULT() }
    else if (strncmp(node_key, "squ", 3) == 0) { NODE.type = SQUARE; WF_DEFAULT() }
    else if (strncmp(node_key, "tri", 3) == 0) { NODE.type = TRIANGLE; WF_DEFAULT() }
    else if (strncmp(node_key, "saw", 3) == 0) { NODE.type = SAWTOOTH; WF_DEFAULT() }
    else if (strncmp(node_key, "noi", 3) == 0) { NODE.type = NOISE; WF_DEFAULT() }

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

    // else if (strncmp(node_key, "fil", 3) == 0) { }

    else {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Unknown synthesis node type: %s\n", node_key);
      exit(1);
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
	  if (strlen(file)) { printf("(%s): ", file); }
	  printf("Node %s references later node %s\n", key, val);
	}
	INPUT(i).addr = input_addr;
      }
      else if (val[0] == '_') {
	if (strcmp(val, "_s") == 0) { INPUT(i).val = pow(2, 1.0/12); }
	else if (strcmp(val, "_e") == 0) { INPUT(i).val = M_E; }
	else if (strcmp(val, "_p") == 0) { INPUT(i).addr = PITCH_IN; }
	else {
	  if (strlen(file)) { printf("(%s): ", file); }
	  printf("Unknown special value: %s\n", val);
	  exit(1);
	}
      }
      else {
	INPUT(i).addr = CONST_IN;
	INPUT(i).val = atof(val);
      }
    }

    if (value != NULL) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Too many inputs for %s node: %s\n", node_key, value);
      exit(1);
    }
  }

  else {
    if (strlen(file)) { printf("(%s): ", file); }
    printf("Unknown config key: %s\n", key);
    exit(1);
  }
}

void doConfigLine(char* line, const char* file) {
  if (strchr("#\n", line[0]) != NULL) { return; }

  do {
    char* current = line;
    line = nexttok(line, " \t\n");
    doConfigClause(current, file);
  } while (line != NULL);
}

void doConfigFile(const char* file) {
  char line[256] = { 0 };
  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    printf("Unable to read conf file %s\n", file);
    exit(1);
  }

  while (!feof(fp)) {
    memset(line, 0, 256);
    fgets(line, 256, fp);
    if (strlen(line)) { doConfigLine(line, file); }
  }
  fclose(fp);
}

