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

int noteForGrid(char grid) {
  int row = grid / 16 - keyboard.origin / 16;
  int col = grid % 16 - keyboard.origin % 16;
  int note = row * (keyboard.layout / 16) + (col - row) * (keyboard.layout % 16) + keyboard.transpose;
  return note;
}

float frequencyForNote(int note_number) {
  return 440 * pow(2, (1.0 * note_number - 69) / 12);
}

char isWhiteKey(int note_number) {
  char mod = note_number % 12;
  return (mod % 2) == (mod >= 5);  
}
