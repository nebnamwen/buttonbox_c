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

int current_inst = 0;
int current_wav = MAIN;

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
  }

  // set color (cymg)
  else if (strncmp(key, "col", 3) == 0) {
    if (strcmp(value, "gray") == 0) { keyboard[current_inst].color = COL_GRAY; }
    else if (strcmp(value, "red") == 0) { keyboard[current_inst].color = COL_RED; }
    else if (strcmp(value, "green") == 0) { keyboard[current_inst].color = COL_GREEN; }
    else if (strcmp(value, "yellow") == 0) { keyboard[current_inst].color = COL_YELLOW; }
    else if (strcmp(value, "blue") == 0) { keyboard[current_inst].color = COL_BLUE; }
    else if (strcmp(value, "magenta") == 0) { keyboard[current_inst].color = COL_MAGENTA; }
    else if (strcmp(value, "cyan") == 0) { keyboard[current_inst].color = COL_CYAN; }
    else if (strcmp(value, "brown") == 0) { keyboard[current_inst].color = COL_BROWN; }

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
    keyboard[current_inst].split = val;
  }
  
  // set slant (/|\^_)
  else if (strncmp(key, "sla", 3) == 0) {
    char val = value[0];
    if (strchr("/|\\", val) == NULL) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for keyboard split slant: %s\n", value);
      printf("(Should be one of / | \\)\n");
      exit(1);      
    }
    keyboard[current_inst].slant = val;
  }
  
  // set origin (grid)
  else if (strncmp(key, "ori", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    keyboard[current_inst].origin = val;
  }
  
  // set transpose (int)
  else if (strncmp(key, "tra", 3) == 0) {
    char val = (char)atoi(value);
    keyboard[current_inst].transpose = val;
  }

  // set origin (grid)
  else if (strncmp(key, "ori", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    keyboard[current_inst].origin = val;
  }  

  // set layout (grid)
  else if (strncmp(key, "lay", 3) == 0) {
    char val = (char)strtol(value, NULL, 0);
    keyboard[current_inst].layout = val;
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
    instrument[current_inst].volume = val;
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
    instrument[current_inst].pan = val;
  }

  // select waveform (main|sine|square|triangle|sawtooth|noise)
  else if (strncmp(key, "wav", 3) == 0) {
    if (strncmp(value, "mai", 3) == 0) { current_wav = MAIN; }
    else if (strncmp(value, "sin", 3) == 0) { current_wav = SINE; }
    else if (strncmp(value, "squ", 3) == 0) { current_wav = SQUARE; }
    else if (strncmp(value, "tri", 3) == 0) { current_wav = TRIANGLE; }
    else if (strncmp(value, "saw", 3) == 0) { current_wav = SAWTOOTH; }
    else if (strncmp(value, "noi", 3) == 0) { current_wav = NOISE; }

    else {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Unknown waveform: %s\n", value);
      printf("(Should be main/sine/square/triangle/sawtooth/noise)\n");
      exit(1);
    }
  }

  // set attack (float)
  else if (strncmp(key, "att", 3) == 0) {
    float val = atof(value);
    if (val < 0) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for envelope attack: %s\n", value);
      printf("(Should be > 0)\n");
      exit(1);      
    }
    instrument[current_inst].envelope[current_wav].attack = val;
  }

  // set peak (float)
  else if (strncmp(key, "pea", 3) == 0) {
    float val = atof(value);
    if (val < 0 || val > 1) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for envelope peak: %s\n", value);
      printf("(Should be between 0 and 1)\n");
      exit(1);      
    }
    instrument[current_inst].envelope[current_wav].peak = val;
  }

  // set decay (float)
  else if (strncmp(key, "dec", 3) == 0) {
    float val = atof(value);
    if (val < 0) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for envelope decay: %s\n", value);
      printf("(Should be > 0)\n");
      exit(1);      
    }
    instrument[current_inst].envelope[current_wav].decay = val;
  }

  // set sustain (float)
  else if (strncmp(key, "sus", 3) == 0) {
    float val = atof(value);
    if (val < 0 || val > 1) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for envelope sustain: %s\n", value);
      printf("(Should be between 0 and 1)\n");
      exit(1);      
    }
    instrument[current_inst].envelope[current_wav].sustain = val;
  }

  // set release (float)
  else if (strncmp(key, "rel", 3) == 0) {
    float val = atof(value);
    if (val < 0) {
      if (strlen(file)) { printf("(%s): ", file); }
      printf("Invalid value for envelope release: %s\n", value);
      printf("(Should be > 0)\n");
      exit(1);      
    }
    instrument[current_inst].envelope[current_wav].release = val;
  }

  // set envelope (all six parameters, float, split on ",/")
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

