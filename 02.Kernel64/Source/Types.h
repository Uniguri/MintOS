#ifndef MINTOS_TYPES_H_
#define MINTOS_TYPES_H_

#define int8 char
#define int16 short
#define int32 int
#define int64 long long
#define uint8 unsigned int8
#define uint16 unsigned int16
#define uint32 unsigned int32
#define uint64 unsigned int64
#define size_t uint64
#define bool uint8
#define BYTE uint8
#define WORD uint16
#define DWORD uint32
#define QWORD uint64
#define BOOL uint8

#define true (1)
#define false (0)
#define null (0)
#define nullptr ((void*)0)
#define TRUE (true)
#define FALSE (false)
#define NULL (null)
#define NULLPTR (nullptr)

#define offsetof(TYPE, MEMBER) __builtin_offsetof(TYPE, MEMBER)

#pragma pack(push, 1)
// Structure for video memory. Video memory locate on 0xB8000 when in protected
// mode. This structure contains two members: charactor, attribute.
// The charactor means code; It is like 'A', 'B', ...
// The attribute means background and foreground colors.
// check out https://en.wikipedia.org/wiki/VGA_text_mode.
typedef struct kCharactorStruct {
  uint8 charactor;
  uint8 attribute;
} Character;
#pragma pack(pop)
_Static_assert(sizeof(Character) == 2, "kCharactorStruct struct size != 2");

#endif