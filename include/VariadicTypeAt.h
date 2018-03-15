#ifndef _TAILAT_H_
#define _TAILAT_H_

#include "TypeList.h"

namespace vapid_db {

template <uint16_t idx, typename... Ts>
struct VariadicTailAt;

template <typename T, typename... Ts>
struct VariadicTailAt<0, T, Ts...> : public TypeList<Ts...> {};

template <>
struct VariadicTailAt<0> : public TypeList<> {};

template <uint16_t idx, typename T, typename... Ts>
struct VariadicTailAt<idx, T, Ts...> : public VariadicTailAt<idx-1, Ts...> {};

template <uint16_t idx, typename T>
struct TailAt;

}

#endif /* _TAILAT_H_ */
