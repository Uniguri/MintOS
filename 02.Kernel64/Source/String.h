#ifndef MINTOS_STRING_H
#define MINTOS_STRING_H

#include "Types.h"

// Print string on (x, y).
// This function does not change attrubutes.
// @param x: 0 means left. (0 <= x < 80)
// @param y: 0 means top. (0 <= y < 25)
void kPrintString(const int x, const int y, const char* string);

int strcmp(const char* s1, const char* s2);
char* strcpy(char* s1, const char* s2);

#endif