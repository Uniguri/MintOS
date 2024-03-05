#include "String.h"

#include <stdarg.h>

#include "Types.h"

inline int sprintf(char* buffer, const char* format, ...) {
  va_list va;

  va_start(va, format);
  int ret = vsprintf(buffer, format, va);
  va_end(va);

  return ret;
}

inline int snprintf(char* buffer, size_t n, const char* format, ...) {
  va_list va;

  va_start(va, format);
  int ret = vsnprintf(buffer, n, format, va);
  va_end(va);

  return ret;
}

inline int vsprintf(char* buffer, const char* format, va_list va) {
  return vsnprintf(buffer, (size_t)-1, format, va);
}

int vsnprintf(char* buffer, size_t n, const char* format, va_list va) {
  if (!n) {
    return 0;
  }

  const size_t format_length = strlen(format);
  size_t idx = 0;
  for (size_t i = 0; i < format_length && idx < n; ++i) {
    const char now_char = format[i];
    const size_t left_count = n - idx;
    // When format string.
    if (now_char == '%') {
      ++i;
      switch (format[i]) {
        case 's': {
          const char* s = (char*)va_arg(va, char*);
          const size_t len = strlen(s);
          if (left_count >= len) {
            strncpy(&buffer[idx], s, len);
            idx += len;
          } else {
            strncpy(&buffer[idx], s, left_count);
            idx += left_count;
          }
          break;
        }

        case 'c': {
          const char c = (char)va_arg(va, int);
          buffer[idx++] = c;
          break;
        }

        case 'd':
        case 'D': {
          const int32 val = (int32)va_arg(va, int32);
          idx += DecimalStringFromInt32(val, &buffer[idx]);
          break;
        }

        case 'x':
        case 'X': {
          const int32 val = (int32)va_arg(va, int32);
          idx += HexStringFromInt64(val, &buffer[idx]);
          break;
        }

        case 'p': {
          const uint64 val = (uint64)va_arg(va, uint64);
          idx += HexStringFromUint64(val, &buffer[idx]);
          break;
        }

        case 'q':
        case 'Q': {
          const int64 val = (int64)va_arg(va, int64);
          idx += DecimalStringFromInt64(val, &buffer[idx]);
          break;
        }

        default: {
          buffer[idx++] = format[i];
          break;
        }
      }
    } else {
      buffer[idx++] = now_char;
    }
  }

  buffer[idx] = 0;
  return idx;
}

inline int tolower(int C) {
  if ('A' <= C && C <= 'Z') {
    return C + ('a' - 'A');
  }
  return C;
}

inline int toupper(int c) {
  if ('a' <= c && c <= 'z') {
    return c - ('a' - 'A');
  }
  return c;
}

char* strchr(const char* s, char c) {
  while (*s) {
    char now = *s;
    if (now == c) {
      return (char*)s;
    }
    ++s;
  }

  return nullptr;
}

char* strstr(const char* s1, const char* s2) {
  if (!*s2) {
    return (char*)s1;
  }

  for (const char* p1 = s1; *p1; ++p1) {
    if (*p1 != *s2) {
      continue;
    }

    const char* const before_p1 = p1;
    const char* p2 = s2;
    while (1) {
      if (!*p2) {
        return (char*)before_p1;
      } else if (*p1++ != *p2++) {
        break;
      }
    }

    p1 = before_p1;
  }

  return nullptr;
}

size_t strlen(const char* s) {
  size_t len = 0;
  while (*s++) {
    ++len;
  }
  return len;
}

inline int strcmp(const char* s1, const char* s2) {
  return strncmp(s1, s2, (size_t)-1);
}

int strncmp(const char* s1, const char* s2, size_t count) {
  size_t now_count = 0;
  while (now_count++ < count && *s1 && (*s1 == *s2)) {
    ++s1;
    ++s2;
  }
  return *(uint8*)s1 - *(uint8*)s2;
}

inline char* strcpy(char* s1, const char* s2) {
  return strncpy(s1, s2, (size_t)-1);
}

char* strncpy(char* s1, const char* s2, size_t count) {
  size_t now_count = 0;
  char* p = s1;
  while (now_count++ < count && *s2) {
    *p++ = *s2++;
  }
  return s1;
}

inline char* strcat(char* s1, const char* s2) {
  return strncat(s1, s2, (size_t)-1);
}

char* strncat(char* s1, const char* s2, size_t count) {
  size_t s1_length = strlen(s1), now_count = 0;
  char* p = s1 + s1_length;
  while (now_count++ < count && *p) {
    *p++ = *s2++;
  }
  return s1;
}

inline char* ReverseString(char* s) {
  return ReverseStringWithIdx(s, 0, strlen(s));
}

char* ReverseStringWithIdx(char* s, size_t start, size_t end) {
  for (size_t i = 0; i < (end - start) / 2; ++i) {
    const size_t target_idx = end - i - 1;
    const char t = s[start + i];
    s[start + i] = s[target_idx];
    s[target_idx] = t;
  }

  return s;
}

inline int32 Int32FromHexString(const char* hex) {
  return (int32)Int64FromHexString(hex);
}

inline int32 Int32FromDecimalString(const char* decimal) {
  return (int32)Int64FromDecimalString(decimal);
}

int64 Int64FromHexString(const char* hex) {
  size_t start_idx = 0, length = strlen(hex);

  int64 sign = 1;
  if (hex[start_idx] == '-') {
    sign = -1;
    ++start_idx;
  } else if (hex[start_idx] == '+') {
    ++start_idx;
  }

  if (hex[start_idx] == '0' && hex[start_idx + 1] == 'x') {
    start_idx += 2;
  } else if (hex[length - 1] == 'h') {
    --length;
  }

  int64 ret = 0;
  for (size_t i = start_idx; i < length; ++i) {
    ret *= 0x10;
    uint8 now_hex = toupper(hex[i]);
    if ('0' <= now_hex && now_hex <= '9') {
      ret += now_hex - '0';
    } else if ('A' <= now_hex && now_hex <= 'F') {
      ret += now_hex - 'A' + 0xA;
    }
  }

  return sign * ret;
}

int64 Int64FromDecimalString(const char* decimal) {
  size_t start_idx = 0, length = strlen(decimal);

  int64 sign = 1;
  if (decimal[start_idx] == '-') {
    sign = -1;
    ++start_idx;
  } else if (decimal[start_idx] == '+') {
    ++start_idx;
  }

  int64 ret = 0;
  for (size_t i = start_idx; i < length; ++i) {
    ret *= 10;
    ret += decimal[i] - '0';
  }

  return sign * ret;
}

uint64 Uint64FromHexString(const char* hex) {
  return (uint64)Int64FromHexString(hex);
}
inline uint64 Uint64FromDecimalString(const char* decimal) {
  return (uint64)Int64FromDecimalString(decimal);
}

inline size_t HexStringFromInt32(int32 decimal, char* buffer) {
  return HexStringFromInt64((int64)decimal, buffer);
}

inline size_t DecimalStringFromInt32(int32 decimal, char* buffer) {
  return DecimalStringFromInt64((int64)decimal, buffer);
}

size_t HexStringFromInt64(int64 decimal, char* buffer) {
  size_t start_idx = 0;

  if (decimal < 0) {
    buffer[start_idx++] = '-';
    decimal *= -1;
  }
  buffer[start_idx++] = '0';
  buffer[start_idx++] = 'x';

  size_t idx = start_idx;
  if (!decimal) {
    buffer[idx++] = '0';
  } else {
    while (decimal) {
      uint8 now_val = decimal % 0x10;
      if (now_val < 0xA) {
        buffer[idx++] = '0' + now_val;
      } else {
        buffer[idx++] = 'A' - 10 + now_val;
      }
      decimal /= 0x10;
    }
  }
  buffer[idx] = 0;

  ReverseStringWithIdx(buffer, start_idx, idx);

  return idx;
}

size_t DecimalStringFromInt64(int64 decimal, char* buffer) {
  size_t start_idx = 0;

  if (decimal < 0) {
    buffer[start_idx++] = '-';
    decimal *= -1;
  }

  size_t idx = start_idx;
  if (!decimal) {
    buffer[idx++] = '0';
  } else {
    while (decimal) {
      buffer[idx++] = '0' + (decimal % 10);
      decimal /= 10;
    }
  }
  buffer[idx] = 0;

  ReverseStringWithIdx(buffer, start_idx, idx);

  return idx;
}

size_t HexStringFromUint64(uint64 decimal, char* buffer) {
  size_t start_idx = 0;

  buffer[start_idx++] = '0';
  buffer[start_idx++] = 'x';

  size_t idx = start_idx;
  if (!decimal) {
    buffer[idx++] = '0';
  } else {
    while (decimal) {
      uint8 now_val = decimal % 0x10;
      if (now_val < 0xA) {
        buffer[idx++] = '0' + now_val;
      } else {
        buffer[idx++] = 'A' - 10 + now_val;
      }
      decimal /= 0x10;
    }
  }
  buffer[idx] = 0;

  ReverseStringWithIdx(buffer, start_idx, idx);

  return idx;
}
size_t DecimalStringFromUint64(uint64 decimal, char* buffer) {
  size_t start_idx = 0;

  size_t idx = start_idx;
  if (!decimal) {
    buffer[idx++] = '0';
  } else {
    while (decimal) {
      buffer[idx++] = '0' + (decimal % 10);
      decimal /= 10;
    }
  }
  buffer[idx] = 0;

  ReverseStringWithIdx(buffer, start_idx, idx);

  return idx;
}

inline size_t itoa(int32 i, char* buffer) {
  return itoa_with_radix(i, buffer, 10);
}

inline size_t itoa_with_radix(int32 i, char* buffer, int radix) {
  return ltoa_with_radix((int64)i, buffer, radix);
}

size_t ltoa(int64 l, char* buffer) { return ltoa_with_radix(l, buffer, 10); }

size_t ltoa_with_radix(int64 l, char* buffer, int radix) {
  switch (radix) {
    case 16:
      return HexStringFromInt64(l, buffer);
    default:
    case 10:
      return DecimalStringFromInt64(l, buffer);
  }
}

inline int32 atoi(const char* s) { return atoi_with_radix(s, 10); }

inline int32 atoi_with_radix(const char* s, int radix) {
  return (int32)atol_with_radix(s, radix);
}

inline int64 atol(const char* s) { return atol_with_radix(s, 10); }

int64 atol_with_radix(const char* s, int radix) {
  switch (radix) {
    case 16:
      return Int64FromHexString(s);
    default:
    case 10:
      return Int64FromDecimalString(s);
  }
}
