#ifndef MINTOS_MACRO_H_
#define MINTOS_MACRO_H_

#define BIT(x) (1u << (x))
#define GET_BIT(x, bit) ((x) & BIT(bit))
#define IS_BIT_SET(x, bit) (GET_BIT((x), bit) != 0)
#define SET_BIT(x, bit) ((x) | BIT(bit))
#define CLEAR_BIT(x, bit) ((x) ^ GET_BIT(x, bit))

#define KB_FROM_BYTE(x) ((size_t)(x) >> 10u)
#define MB_FROM_BYTE(x) ((size_t)KB_FROM_BYTE((x)) >> 10u)
#define GB_FROM_BYTE(x) ((size_t)MB_FROM_BYTE((x)) >> 10u)
#define TB_FROM_BYTE(x) ((size_t)GB_FROM_BYTE((x)) >> 10u)
#define BYTE_FROM_KB(x) ((size_t)(x) << 10u)
#define BYTE_FROM_MB(x) ((size_t)BYTE_FROM_KB((x)) << 10u)
#define BYTE_FROM_GB(x) ((size_t)BYTE_FROM_MB((x)) << 10u)
#define BYTE_FROM_TB(x) ((size_t)BYTE_FROM_GB((x)) << 10u)

#endif