typedef struct {
  char is_active;
  char split;
  char slant;
  char color;
  char origin;
  unsigned char transpose;
  unsigned char layout;
} keyboard_t;

#define COL_GRAY 0
#define COL_RED 1
#define COL_GREEN 2
#define COL_YELLOW 3
#define COL_BLUE 4
#define COL_MAGENTA 5
#define COL_CYAN 6
#define COL_BROWN 7

keyboard_t default_keyboard = { 0, 0x09, '/', 0, 0x00, 36, 0x72 };

#define NUM_INSTS 8
keyboard_t keyboard[NUM_INSTS];

signed char keygrid[128];

void initKeygrid() {

  for (int i = 0; i < NUM_INSTS; i++) {
    keyboard[i] = default_keyboard;
  }

  keyboard[0].is_active = 1;
  
  for (int i = 0; i < 128; i++) {
    keygrid[i] = -1;
  }

  keygrid[SDL_SCANCODE_Z] = 0x00;
  keygrid[SDL_SCANCODE_X] = 0x01;
  keygrid[SDL_SCANCODE_C] = 0x02;
  keygrid[SDL_SCANCODE_V] = 0x03;
  keygrid[SDL_SCANCODE_B] = 0x04;
  keygrid[SDL_SCANCODE_N] = 0x05;
  keygrid[SDL_SCANCODE_M] = 0x06;
  keygrid[SDL_SCANCODE_COMMA] = 0x07;
  keygrid[SDL_SCANCODE_PERIOD] = 0x08;
  keygrid[SDL_SCANCODE_SLASH] = 0x09;

  keygrid[SDL_SCANCODE_A] = 0x10;
  keygrid[SDL_SCANCODE_S] = 0x11;
  keygrid[SDL_SCANCODE_D] = 0x12;
  keygrid[SDL_SCANCODE_F] = 0x13;
  keygrid[SDL_SCANCODE_G] = 0x14;
  keygrid[SDL_SCANCODE_H] = 0x15;
  keygrid[SDL_SCANCODE_J] = 0x16;
  keygrid[SDL_SCANCODE_K] = 0x17;
  keygrid[SDL_SCANCODE_L] = 0x18;
  keygrid[SDL_SCANCODE_SEMICOLON] = 0x19;
  keygrid[SDL_SCANCODE_APOSTROPHE] = 0x1A;

  keygrid[SDL_SCANCODE_Q] = 0x20;
  keygrid[SDL_SCANCODE_W] = 0x21;
  keygrid[SDL_SCANCODE_E] = 0x22;
  keygrid[SDL_SCANCODE_R] = 0x23;
  keygrid[SDL_SCANCODE_T] = 0x24;
  keygrid[SDL_SCANCODE_Y] = 0x25;
  keygrid[SDL_SCANCODE_U] = 0x26;
  keygrid[SDL_SCANCODE_I] = 0x27;
  keygrid[SDL_SCANCODE_O] = 0x28;
  keygrid[SDL_SCANCODE_P] = 0x29;
  keygrid[SDL_SCANCODE_LEFTBRACKET] = 0x2A;
  keygrid[SDL_SCANCODE_RIGHTBRACKET] = 0x2B;

  keygrid[SDL_SCANCODE_1] = 0x30;
  keygrid[SDL_SCANCODE_2] = 0x31;
  keygrid[SDL_SCANCODE_3] = 0x32;
  keygrid[SDL_SCANCODE_4] = 0x33;
  keygrid[SDL_SCANCODE_5] = 0x34;
  keygrid[SDL_SCANCODE_6] = 0x35;
  keygrid[SDL_SCANCODE_7] = 0x36;
  keygrid[SDL_SCANCODE_8] = 0x37;
  keygrid[SDL_SCANCODE_9] = 0x38;
  keygrid[SDL_SCANCODE_0] = 0x39;
  keygrid[SDL_SCANCODE_MINUS] = 0x3A;
  keygrid[SDL_SCANCODE_EQUALS] = 0x3B;
}

int keyboardForGrid(char grid) {
  for (int i = 0; i < NUM_INSTS; i++) {
    int du, dv;

    switch (keyboard[i].slant) {
    case '\\':
      du = 0;
      dv = 1;
      break;

    case '/':
      du = -1;
      dv = 1;
      break;

    case '|':
      du = -1;
      dv = 2;
      break;

    case '`':
      du = -16;
      dv = 1;
      break;
      
    case '\'':
      du = -16;
      dv = -1;
      break;
      
    case ',':
      du = 16;
      dv = 1;
      break;
      
    case '.':
      du = 16;
      dv = -1;
      break;
      
    default:
      du = 0;
      dv = 1;
    }

    int split = du * (keyboard[i].split / 16) + dv * (keyboard[i].split % 16);
    int key = du * (grid / 16) + dv * (grid % 16);
    if (key <= split) {
      return i;
    }
  }

  return NUM_INSTS - 1;
}

int noteForGrid(char grid) {
  int kb = keyboardForGrid(grid);
  int row = grid / 16 - keyboard[kb].origin / 16;
  int col = grid % 16 - keyboard[kb].origin % 16;
  int note = row * (keyboard[kb].layout / 16) + (col - row) * (keyboard[kb].layout % 16) + keyboard[kb].transpose;
  return note;
}

float frequencyForNote(int note_number) {
  return 440 * pow(2, (1.0 * note_number - 69) / 12);
}

char isWhiteKey(int note_number) {
  char mod = note_number % 12;
  return (mod % 2) == (mod >= 5);  
}
