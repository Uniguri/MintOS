#include "Keyboard.h"

#include "Hardware.h"
#include "Interrupt.h"
#include "Macro.h"
#include "Queue.h"
#include "Types.h"

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

#define KEY_MAX_QUEUE_COUNT 100
static Queue key_queue;
static KeyData key_queue_buffer[KEY_MAX_QUEUE_COUNT];

inline bool kInitializeKeyboard(void) {
  kInitializeQueue(&key_queue, key_queue_buffer, KEY_MAX_QUEUE_COUNT,
                   sizeof(KeyData));

  return kActivateKeyboard();
}

bool kConvertScanCodeAndPushToQueue(uint8 scan_code) {
  KeyData key_data = {
      0,
  };
  key_data.scan_code = scan_code;

  bool ret = false;
  if (kConvertScanCodeToAsciiCode(scan_code, &key_data.ascii_code,
                                  &key_data.flag)) {
    bool previouse_interrupt_status = kIsInterruptEnabled();
    kSetInterruptFlag(false);
    ret = kPushQueue(&key_queue, &key_data);
    kSetInterruptFlag(previouse_interrupt_status);
  }

  return ret;
}
#undef KEY_MAX_QUEUE_COUNT

bool kGetKeyFromKeyQueue(KeyData* key_data) {
  if (kIsQueueEmpty(&key_queue)) {
    return false;
  }

  bool previouse_interrupt_status = kIsInterruptEnabled();
  bool ret = true;
  kSetInterruptFlag(false);
  ret &= kGetFrontFromQueue(&key_queue, key_data);
  ret &= kPopQueue(&key_queue);
  kSetInterruptFlag(previouse_interrupt_status);

  return ret;
}

#define GET_PORT_DATA() kGetPortByte(0x60)
bool kWaitForACKAndPutOtherScanCode(void) {
  bool ret = false;
  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 0xFFFF; ++j) {
      if (kIsOutputBufferFull()) {
        break;
      }
    }

    uint8 data = GET_PORT_DATA();
    // When data is ACK(0xFA)
    if (data == 0xFA) {
      ret = true;
      break;
    } else {
      kConvertScanCodeAndPushToQueue(data);
    }
  }

  return ret;
}

#define GET_PORT_STATUS() kGetPortByte(0x64)
inline bool kIsOutputBufferFull(void) {
  return IS_BIT_SET(GET_PORT_STATUS(), 0);
}

inline bool kIsInputBufferFull(void) {
  return IS_BIT_SET(GET_PORT_STATUS(), 1);
}

void kClearOutputPortByte(void) {
  while (!kIsOutputBufferFull())
    ;
}

#define SET_PORT_DATA(data) kSetPortByte(0x60, (data))
#define SEND_COMMAND(cmd) kSetPortByte(0x64, (cmd))
bool kActivateKeyboard(void) {
  bool previous_interrupt_status = kIsInterruptEnabled();
  kSetInterruptFlag(false);

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
  bool ret = kWaitForACKAndPutOtherScanCode();

  kSetInterruptFlag(previous_interrupt_status);

  return ret;
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

inline uint8 kGetKeyboardScanCode(void) {
  kClearOutputPortByte();
  //  Get scan code.
  const uint8 code = GET_PORT_DATA();
  return code;
}

#define GET_DOWN_CODE(code) (code & 0x7F)
bool kIsUseCombinedCode(const uint8 scan_code) {
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

inline bool kIsAlphabetScanCode(const uint8 scan_code) {
  uint8 nomal_code = key_mapping_table[scan_code].normal_code;
  return ('a' <= nomal_code && nomal_code <= 'z');
}

inline bool kIsNumberOrSymbolScanCode(const uint8 scan_code) {
  // scan code 2 ~ 53 are number + alphabet + symbols.
  // Thus, scan code is number or symbol when in [2, 53] and is not alphabet.
  return (2 <= scan_code && scan_code <= 53 && !kIsAlphabetScanCode(scan_code));
}

inline bool kIsNumberPadScanCode(const uint8 scan_code) {
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