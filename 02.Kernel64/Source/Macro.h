#ifndef MINTOS_MACRO_H_
#define MINTOS_MACRO_H_

#define BIT(x) (1u << x)
#define GET_BIT(x, bit) (x & BIT(bit))
#define IS_BIT_SET(x, bit) (GET_BIT(x, bit) != 0)
#define SET_BIT(x, bit) (x | BIT(bit))
#define CLEAR_BIT(x, bit) (x ^ GET_BIT(x, bit))

#endif