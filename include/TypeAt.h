#pragma once

namespace vapid_db {

template <uint16_t idx, typename... Ts>
struct VariadicTypeAt;

template<uint16_t idx>
struct VariadicTypeAt<idx> {
  using type = NoneType;
};

template<uint16_t idx, typename T, typename... Ts>
struct VariadicTypeAt<idx, T, Ts...> : public VariadicTypeAt<idx-1, Ts...> {};

template<typename T>
struct VariadicTypeAt<0, T> {
  using type = T;
};

}

