#pragma once

#include <type_traits>
#include "NoneType.h"

namespace vapid_db {

template <typename... Ts> class TypeList {};

template <typename... Ts>
struct Head;

template <typename T, typename... Ts>
struct Head<TypeList<T, Ts...>> {
  using type = T;
};

template <typename... Ts>
struct Tail;

template <typename T, typename... Ts>
struct Tail<TypeList<T, Ts...>> {
  using type = TypeList<Ts...>;
};

template <uint16_t count, typename T>
struct Pop : Pop<count - 1, typename Tail<T>::type> {};

template <typename T>
struct Pop<0, T> {
  using type = T;
};

template <typename... Ts>
struct LengthOf;

template <>
struct LengthOf<> {
  static constexpr uint16_t value = 0;
};

template <typename T, typename... Ts>
struct LengthOf<T, Ts...> {
  static constexpr uint16_t value = LengthOf<Ts...>::value + 1;
};

}
