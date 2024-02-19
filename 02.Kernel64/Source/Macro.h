#ifndef MINTOS_MACRO_H_
#define MINTOS_MACRO_H_

#define BITS(x) (1 << x)
#define GET_BIT(x, bit) (x & BITS(bit))
#define IS_BIT_SET(x, bit) (GET_BIT(x, bit) != 0)
#define SET_BIT(x, bit) (x | BITS(bit))
#define CLEAR_BIT(x, bit) (x ^ GET_BIT(x, bit))

#endif