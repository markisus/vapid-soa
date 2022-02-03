#pragma once

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

// Pop pops off elements from the head of a TypeList.
// Examples:
//  Pop<TypeList<A, B, C, D>, 0>::type = TypeList<A, B, C, D>
//  Pop<TypeList<A, B, C, D>, 1>::type = TypeList<B, C, D>
//  Pop<TypeList<A, B, C, D>, 2>::type = TypeList<C, D>
template <uint16_t count, typename T>
struct Pop : Pop<count - 1, typename Tail<T>::type> {};

template <typename T>
struct Pop<0, T> {
  using type = T;
};

}
