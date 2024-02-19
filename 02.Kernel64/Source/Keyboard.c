#include "Keyboard.h"

#include "Macro.h"
#include "Types.h"

uint8 kGetPortByte(uint8 port) {
  uint64 data = 0;
  asm volatile(
      // Read data from port
      "in al, dx\n"
      : [data] "=a"(data)
      : [port] "d"(port));
  return (uint8)(data & 0xFF);
}

void kSetPortByte(uint8 port, uint8 data) {
  uint8 p = port, d = data;
  asm volatile(
      // Set data on port.
      "out dx, al\n"
      :
      : [port] "d"((uint64)port), [data] "a"(data));
}

bool kIsOutputBufferFull(void) { return IS_BIT_SET(kGetPortByte(0x64), 0); }

bool kIsInputBufferFull(void) { return IS_BIT_SET(kGetPortByte(0x64), 1); }

void kClearOutputPortByte(void) {
  while (!kIsOutputBufferFull())
    ;
}

#define GET_PORT_DATA() kGetPortByte(0x60)
#define SET_PORT_DATA(data) kSetPortByte(0x60, data)
#define GET_PORT_STATUS() kGetPortByte(0x64)
#define SEND_COMMAND(cmd) kSetPortByte(0x64, cmd)

bool kActivateKeyboard(void) {
  // Send enable command(0xAE) to command register(0x64).
  SEND_COMMAND(0xAE);
  // Wait until input buffer is claen.
  for (int i = 0; i < 0xFFFF; ++i) {
    if (!kIsInputBufferFull()) {
      break;
    }
  }

  // Send enable scanning(0xF4) to data port.
  SET_PORT_DATA(0xF4);
  // Wait until enabled.
  for (int i = 0; i < 100; ++i) {
    // Wait until output buffer is claen.
    for (int i = 0; i < 0xFFFF; ++i) {
      if (!kIsOutputBufferFull()) {
        break;
      }
    }
    // Check a response(0xFA) of the enable command(0xF4).
    if (GET_PORT_DATA() == 0xFA) {
      return true;
    }
  }

  kClearOutputPortByte();

  // Fail to activate keyboard.
  return false;
}

void kRebootPS2(void) {
  for (int i = 0; i < 0xFF; ++i) {
    if (!kIsInputBufferFull()) {
      break;
    }
  }

  // 0xD1: Write next byte to Controller Output Port.
  SEND_COMMAND(0xD1);
  // 0x00: System reset (output).
  SET_PORT_DATA(0x00);

  while (1)
    ;
}

uint8 kGetKeyboardScanCode(void) {
  kClearOutputPortByte();
  // Get scan code.
  const uint8 code = GET_PORT_DATA();
  kClearOutputPortByte();
  return code;
}

static KeyboardManager keyboard_manager = {
    0,
};
static KeyMappingEntry key_mapping_table[KEY_MAPPING_TABLE_MAX_COUNT] = {
    {kNone, kNone},
    {kEsc, kEsc},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {kBacksapce, kBacksapce},
    {kTab, kTab},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {'\n', '\n'},
    {kCtrl, kCtrl},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '\"'},
    {'`', '~'},
    {kLshift, kLshift},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {kRshift, kRshift},
    {'*', '*'},
    {kLalt, kLalt},
    {' ', ' '},
    {kCapslock, kCapslock},
    {kF1, kF1},
    {kF2, kF2},
    {kF3, kF3},
    {kF4, kF4},
    {kF5, kF5},
    {kF6, kF6},
    {kF7, kF7},
    {kF8, kF8},
    {kF9, kF9},
    {kF10, kF10},
    {kNumlock, kNumlock},
    {kScrolllock, kScrolllock},
    {kHome, '7'},
    {kUp, '8'},
    {kPageup, '9'},
    {'-', '-'},
    {kLeft, '4'},
    {kCenter, '5'},
    {kRight, '6'},
    {'+', '+'},
    {kEnd, '1'},
    {kDown, '2'},
    {kPagedown, '3'},
    {kIns, '0'},
    {kDel, '.'},
    {kNone, kNone},
    {kNone, kNone},
    {kNone, kNone},
    {kF11, kF11},
    {kF12, kF12}};

#define GET_DOWN_CODE(code) (code & 0x7F)
bool kIsUseCombinedCode(const uint8 scan_code) {
  const uint8 down_scan_code = GET_DOWN_CODE(scan_code);

  if (kIsAlphabetScanCode(scan_code)) {
    // Alphabet is affected by SHT, CAPS.
    return keyboard_manager.shift_down ^ keyboard_manager.caps_lock_on;
  }
  if (kIsNumberOrSymbolScanCode(scan_code)) {
    // Number and symbol is affected by SHT.
    return keyboard_manager.shift_down;
  }
  // Extended code and numpad numbers has same scan code except 0xE0.
  // Use combined code when not extended code.
  if (kIsNumberPadScanCode(scan_code) && !keyboard_manager.extened_code_in) {
    // Numpad number is affected by NumLock.
    return keyboard_manager.num_lock_on;
  }

  // Unreachable.
  return false;
}

bool kIsAlphabetScanCode(const uint8 scan_code) {
  uint8 nomal_code = key_mapping_table[scan_code].normal_code;
  return ('a' <= nomal_code && nomal_code <= 'z');
}

bool kIsNumberOrSymbolScanCode(const uint8 scan_code) {
  // scan code 2 ~ 53 are number + alphabet + symbols.
  // Thus, scan code is number or symbol when in [2, 53] and is not alphabet.
  return (2 <= scan_code && scan_code <= 53 && !kIsAlphabetScanCode(scan_code));
}

bool kIsNumberPadScanCode(const uint8 scan_code) {
  // scan code 71 ~ 88 are key on numpad.
  return (71 <= scan_code && scan_code <= 83);
}

#define IS_UP_CODE(code) (code & 0x80)
bool kUpdateCombinationKeyStatus(const uint8 scan_code) {
  // is_pressed equals to is_down and is_on.
  bool is_pressed = true;
  uint8 down_scan_code = scan_code;
  // Check scan code is up code and convert it to down code.
  if (IS_UP_CODE(scan_code)) {
    is_pressed = false;
    down_scan_code = GET_DOWN_CODE(scan_code);
  }

  // When scan code is SHT(42, 54)
  if (down_scan_code == 42 || down_scan_code == 54) {
    keyboard_manager.shift_down = is_pressed;
    return true;
  }

  // When scan code is Caps Lock(58), switch its status. Only handle when
  // pressed.
  if (down_scan_code == 58 && is_pressed) {
    keyboard_manager.caps_lock_on ^= true;
    return true;
  }

  // When scan code is Num Lock(69), switch its status. Only handle when
  // pressed.
  if (down_scan_code == 69 && is_pressed) {
    keyboard_manager.num_lock_on ^= true;
    return true;
  }

  return false;
}

bool kConvertScanCodeToAsciiCode(const uint8 scan_code, uint8* ascii_code,
                                 enum KeyFlag* flag) {
  // If previous code is PAUSE, ignore current scan code.
  if (keyboard_manager.skip_count_for_pause > 0) {
    --keyboard_manager.skip_count_for_pause;
    return false;
  }

  // Handle PAUSE scan code(0xE1).
  if (scan_code == 0xE1) {
    *ascii_code = kPause;
    *flag = kFlagDown;
    keyboard_manager.skip_count_for_pause = KEY_SKIP_COUNT_FOR_PAUSE;
    return false;
  }
  // Handle extention scan code (0xE0).
  if (scan_code == 0xE0) {
    keyboard_manager.extened_code_in = true;
    return false;
  }

  if (kUpdateCombinationKeyStatus(scan_code)) {
    return false;
  }

  const bool to_use_combined_key = kIsUseCombinedCode(scan_code);
  if (to_use_combined_key) {
    *ascii_code = key_mapping_table[GET_DOWN_CODE(scan_code)].combined_code;
  } else {
    *ascii_code = key_mapping_table[GET_DOWN_CODE(scan_code)].normal_code;
  }

  if (keyboard_manager.extened_code_in) {
    *flag = kFlagExtendedKey;
    keyboard_manager.extened_code_in = false;
  } else {
    *flag = null;
  }

  if (!IS_UP_CODE(scan_code)) {
    *flag = kFlagDown;
  }

  return true;
}
#undef IS_UP_CODE
#undef GET_DOWN_CODE

#undef GET_PORT_DATA
#undef SET_PORT_DATA
#undef GET_PORT_STATUS
#undef SEND_COMMAND