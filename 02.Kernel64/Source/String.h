#ifndef MINTOS_STRING_H
#define MINTOS_STRING_H

#include <stdarg.h>

#include "Types.h"

int sprintf(char* buffer, const char* format, ...);
int snprintf(char* buffer, size_t n, const char* format, ...);

int vsprintf(char* buffer, const char* format, va_list va);
int vsnprintf(char* buffer, size_t n, const char* format, va_list va);

int tolower(int C);
int toupper(int c);

char* strchr(const char* s, char c);
char* strstr(const char* s1, const char* s2);
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t count);
char* strcpy(char* s1, const char* s2);
char* strncpy(char* s1, const char* s2, size_t count);
char* strcat(char* s1, const char* s2);
char* strncat(char* s1, const char* s2, size_t count);

char* ReverseString(char* s);
// Reverse s[start:end].
// @param s: string to reverse.
// @param start: index to start reverse.
// @param end: index to end reverse. s[end] is not changed.
// @return the pointer to reversed string.
char* ReverseStringWithIdx(char* s, size_t start, size_t end);

int32 Int32FromHexString(const char* hex);
int32 Int32FromDecimalString(const char* decimal);
int64 Int64FromHexString(const char* hex);
int64 Int64FromDecimalString(const char* decimal);
uint64 Uint64FromHexString(const char* hex);
uint64 Uint64FromDecimalString(const char* decimal);
size_t HexStringFromInt32(int32 decimal, char* buffer);
size_t DecimalStringFromInt32(int32 decimal, char* buffer);
size_t HexStringFromInt64(int64 decimal, char* buffer);
size_t DecimalStringFromInt64(int64 decimal, char* buffer);
size_t HexStringFromUint64(uint64 decimal, char* buffer);
size_t DecimalStringFromUint64(uint64 decimal, char* buffer);

size_t itoa(int32 i, char* buffer);
size_t itoa_with_radix(int32 i, char* buffer, int radix);
size_t ltoa(int64 l, char* buffer);
size_t ltoa_with_radix(int64 l, char* buffer, int radix);

int32 atoi(const char* s);
int32 atoi_with_radix(const char* s, int radix);
int64 atol(const char* buffer);
int64 atol_with_radix(const char* s, int radix);

#endif