#ifndef MINTOS_KEYBOARD_H_
#define MINTOS_KEYBOARD_H_

#include "Types.h"

#define KEY_SKIP_COUNT_FOR_PAUSE 2
enum KeyFlag { kFlagUp = 0, kFlagDown, kFlagExtendedKey };
#define KEY_MAPPING_TABLE_MAX_COUNT 89
enum Key {
  kNone = 0,
  kEnter = '\n',
  kTab = '\t',
  kEsc = 0x1B,
  kBacksapce = 0x08,
  kCtrl = 0x81,
  kLshift,
  kRshift,
  kPrintscreen,
  kLalt,
  kCapslock,
  kF1,
  kF2,
  kF3,
  kF4,
  kF5,
  kF6,
  kF7,
  kF8,
  kF9,
  kF10,
  kNumlock,
  kScrolllock,
  kHome,
  kUp,
  kPageup,
  kLeft,
  kCenter,
  kRight,
  kEnd,
  kDown,
  kPagedown,
  kIns,
  kDel,
  kF11,
  kF12,
  kPause
};
#pragma pack(push, 1)
// Represent entry for key mapping table(key_mapping_table).
typedef struct kKeyMappingEntryStruct {
  // Key without CTL or SFT.
  uint8 normal_code;
  // Key with CTL or SFT.
  uint8 combined_code;
} KeyMappingEntry;

typedef struct kKeyboardManagerStruct {
  bool shift_down;
  bool caps_lock_on;
  bool num_lock_on;
  bool scroll_lock_on;

  bool extened_code_in;
  int skip_count_for_pause;
} KeyboardManager;

typedef struct kKeyDataStruct {
  uint8 scan_code;
  uint8 ascii_code;
  enum KeyFlag flag;
} KeyData;
#pragma pack(pop)

// Initialize keyboard. things initialized:
// - queue
// - PS/2
// @return True if success.
bool kInitializeKeyboard(void);

// Convert scan code to ascii and push it to queue.
// @param scan_code: scan code to convert and push.
// @return True if success.
bool kConvertScanCodeAndPushToQueue(uint8 scan_code);

bool kWaitForACKAndPutOtherScanCode(void);

// Get key from queue.
// @param data: the pointer of key data to store.
// @return True if success.
bool kGetKeyFromKeyQueue(KeyData* key_data);

// Wait and get scan code.
// @return scan code
uint8 kGetKeyboardScanCode(void);

// Check whether there is an element on PS/2's output buffer(port 0x64 : bit
// 0).
// @return true when output buffer full
bool kIsOutputBufferFull(void);

// Check whether there is an element on PS/2's input buffer(port 0x64 : bit 1).
// @return true when input buffer full
bool kIsInputBufferFull(void);

// Clear output port byte.
void kClearOutputPortByte(void);

// Activate Keyboard device and keyboard.
// @return true if activating is success.
bool kActivateKeyboard(void);

// Reboot PS/2.
void kRebootPS2(void);

// @param scan_code: scan code
// @return true if we need to use combined code.
bool kIsUseCombinedCode(const uint8 scan_code);

// @param scan_code: scan code
// @return true if scan code is alphabet.
bool kIsAlphabetScanCode(const uint8 scan_code);

// @param scan_code: scan code
// @return true if scan code is number of symbol.
bool kIsNumberOrSymbolScanCode(const uint8 scan_code);

// @param scan_code: scan code
// @return true if scan code is numpad number.
bool kIsNumberPadScanCode(const uint8 scan_code);

// Update keyboard_manager depending on scan_code. This allow us to use
// combination code.
// @param scan_code: scan_code.
// @return true when scan code make update.
bool kUpdateCombinationKeyStatus(const uint8 scan_code);

// Try to convert scan code to ascii code.
// @param scan_code: scan_code to convert.
// @param ascii_code: pointer to store ascii code.
// @param flag: pointer to key flag: down, up, extended.
// @return true when convertion successes.
bool kConvertScanCodeToAsciiCode(const uint8 scan_code, uint8* ascii_code,
                                 enum KeyFlag* flag);

#endif