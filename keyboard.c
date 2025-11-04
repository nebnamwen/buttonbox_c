typedef struct {
  char instrument;
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

keyboard_t default_base_keyboard = { 1, 0x09, '/', COL_GRAY, 0x00, 36, 0x72 };
// instrument 0 and layout 0 mean "same as previous keyboard"
keyboard_t default_keyboard = { 0, 0x09, '/', COL_GRAY, 0x00, 36, 0 };

#define NUM_KEYBDS 8
keyboard_t keyboard[NUM_KEYBDS + 1];

signed char keygrid[256];
signed char qwerty[256];

#define KEY_BUTTON -1 // not a note key but onset and offset time are tracked using a note struct
#define KEY_FUNCTION -2 // triggers a function call on press, no action on release, state not tracked
#define KEY_NOTHING -3 // does nothing at all

void initKeygrid() {

  for (int i = 0; i <= NUM_KEYBDS; i++) {
    keyboard[i] = default_keyboard;
  }

  keyboard[1] = default_base_keyboard;
  
  for (int i = 0; i < 256; i++) {
    keygrid[i] = qwerty[i] = KEY_NOTHING;
  }

  keygrid[SDL_SCANCODE_ESCAPE] = KEY_BUTTON;

  keygrid[SDL_SCANCODE_TAB] = KEY_FUNCTION;

  for (int i = SDL_SCANCODE_F1; i <= SDL_SCANCODE_F12; i++) {
    keygrid[i] = KEY_FUNCTION;
  }

  keygrid[SDL_SCANCODE_LSHIFT] = KEY_BUTTON;
  keygrid[SDL_SCANCODE_RSHIFT] = KEY_BUTTON;
  keygrid[SDL_SCANCODE_SPACE] = KEY_BUTTON;
  
  keygrid[SDL_SCANCODE_Z] = qwerty['z'] = 0x00;
  keygrid[SDL_SCANCODE_X] = qwerty['x'] = 0x01;
  keygrid[SDL_SCANCODE_C] = qwerty['c'] = 0x02;
  keygrid[SDL_SCANCODE_V] = qwerty['v'] = 0x03;
  keygrid[SDL_SCANCODE_B] = qwerty['b'] = 0x04;
  keygrid[SDL_SCANCODE_N] = qwerty['n'] = 0x05;
  keygrid[SDL_SCANCODE_M] = qwerty['m'] = 0x06;
  keygrid[SDL_SCANCODE_COMMA] = qwerty[','] = 0x07;
  keygrid[SDL_SCANCODE_PERIOD] = qwerty['.'] = 0x08;
  keygrid[SDL_SCANCODE_SLASH] = qwerty['/'] = 0x09;

  keygrid[SDL_SCANCODE_A] = qwerty['a'] = 0x10;
  keygrid[SDL_SCANCODE_S] = qwerty['s'] = 0x11;
  keygrid[SDL_SCANCODE_D] = qwerty['d'] = 0x12;
  keygrid[SDL_SCANCODE_F] = qwerty['f'] = 0x13;
  keygrid[SDL_SCANCODE_G] = qwerty['g'] = 0x14;
  keygrid[SDL_SCANCODE_H] = qwerty['h'] = 0x15;
  keygrid[SDL_SCANCODE_J] = qwerty['j'] = 0x16;
  keygrid[SDL_SCANCODE_K] = qwerty['k'] = 0x17;
  keygrid[SDL_SCANCODE_L] = qwerty['l'] = 0x18;
  keygrid[SDL_SCANCODE_SEMICOLON] = qwerty[';'] = 0x19;
  keygrid[SDL_SCANCODE_APOSTROPHE] = qwerty['\''] = 0x1A;

  keygrid[SDL_SCANCODE_Q] = qwerty['q'] = 0x20;
  keygrid[SDL_SCANCODE_W] = qwerty['w'] = 0x21;
  keygrid[SDL_SCANCODE_E] = qwerty['e'] = 0x22;
  keygrid[SDL_SCANCODE_R] = qwerty['r'] = 0x23;
  keygrid[SDL_SCANCODE_T] = qwerty['t'] = 0x24;
  keygrid[SDL_SCANCODE_Y] = qwerty['y'] = 0x25;
  keygrid[SDL_SCANCODE_U] = qwerty['u'] = 0x26;
  keygrid[SDL_SCANCODE_I] = qwerty['i'] = 0x27;
  keygrid[SDL_SCANCODE_O] = qwerty['o'] = 0x28;
  keygrid[SDL_SCANCODE_P] = qwerty['p'] = 0x29;
  keygrid[SDL_SCANCODE_LEFTBRACKET] = qwerty['['] = 0x2A;
  keygrid[SDL_SCANCODE_RIGHTBRACKET] = qwerty[']'] = 0x2B;

  keygrid[SDL_SCANCODE_1] = qwerty['1'] = 0x30;
  keygrid[SDL_SCANCODE_2] = qwerty['2'] = 0x31;
  keygrid[SDL_SCANCODE_3] = qwerty['3'] = 0x32;
  keygrid[SDL_SCANCODE_4] = qwerty['4'] = 0x33;
  keygrid[SDL_SCANCODE_5] = qwerty['5'] = 0x34;
  keygrid[SDL_SCANCODE_6] = qwerty['6'] = 0x35;
  keygrid[SDL_SCANCODE_7] = qwerty['7'] = 0x36;
  keygrid[SDL_SCANCODE_8] = qwerty['8'] = 0x37;
  keygrid[SDL_SCANCODE_9] = qwerty['9'] = 0x38;
  keygrid[SDL_SCANCODE_0] = qwerty['0'] = 0x39;
  keygrid[SDL_SCANCODE_MINUS] = qwerty['-'] = 0x3A;
  keygrid[SDL_SCANCODE_EQUALS] = qwerty['='] = 0x3B;
}

int keyboardForGrid(char grid) {
  for (int i = 1; i < NUM_KEYBDS; i++) {
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

  return NUM_KEYBDS;
}

unsigned char layoutForKeyboard(char kb) {
  if (keyboard[kb].layout == 0 && kb > 1) {
    return layoutForKeyboard(kb - 1);
  } else {
    return keyboard[kb].layout;
  }
}

int noteForGrid(char grid) {
  int kb = keyboardForGrid(grid);
  int row = grid / 16 - keyboard[kb].origin / 16;
  int col = grid % 16 - keyboard[kb].origin % 16;
  unsigned char layout = layoutForKeyboard(kb);
  int note = row * (layout / 16) + (col - row) * (layout % 16) + keyboard[kb].transpose;
  return note;
}

char instForKeyboard(char kb) {
  if (keyboard[kb].instrument == 0 && kb > 1) {
    return instForKeyboard(kb - 1);
  } else {
    return keyboard[kb].instrument;
  }
}

float frequencyForNote(int note_number) {
  return 440 * pow(2, (1.0 * note_number - 69) / 12);
}

char isWhiteKey(int note_number) {
  char mod = note_number % 12;
  return (mod % 2) == (mod >= 5);  
}

char isRootKey(char grid) {
  int kb = keyboardForGrid(grid);
  int note = noteForGrid(grid);
  return note % 12 == keyboard[kb].transpose % 12;
}
