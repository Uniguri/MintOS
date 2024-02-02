#ifndef __MINTOS_TYPES_H_
#define __MINTOS_TYPES_H_

#define int8 char
#define int16 short
#define int32 int
#define int64 long
#define uint8 unsigned int8
#define uint16 unsigned int16
#define uint32 unsigned int32
#define uint64 unsigned int64
#define bool uint8
#define BYTE uint16
#define WORD uint32
#define DWORD uint64
#define QWORD uint64
#define BOOL uint8

#define true (1)
#define false (0)
#define null (0)
#define nullptr ((void*)0)
#define TRUE (1)
#define FALSE (0)
#define NULL (0)
#define NULLPTR ((void*)0)

#pragma pack(push, 1)
typedef struct kCharactorStruct {
  BYTE charactor;
  BYTE attribute;
} Character;
#pragma pack(pop)

#endif