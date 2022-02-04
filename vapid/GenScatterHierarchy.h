#ifndef _GENSCATTERHIERARCHY_H_
#define _GENSCATTERHIERARCHY_H_

#include "Tagged.h"
#include "TypeList.h"

namespace vapid {

template <template <typename> typename Unit, typename... Ts>
struct GenScatterHierarchy;

template <template <typename> typename Unit>
struct GenScatterHierarchy<Unit> {
  using type_list = TypeList<>;
};

template <template <typename> typename Unit, typename T, typename... Ts>
struct GenScatterHierarchy<Unit, T, Ts...> :
      public Tagged<Unit<T>, TypeList<Unit<Ts>...>>,
      public GenScatterHierarchy<Unit, Ts...> {
  using type_list = TypeList<Unit<T>, Unit<Ts>...>;
};

template <uint16_t idx, typename T>
struct TaggedFieldAt;

template <typename T, typename... Ts>
struct TaggedFieldAt<0, TypeList<T, Ts...>> {
  using type = Tagged<T, TypeList<Ts...>>;
};

template <uint16_t idx, typename T, typename... Ts>
struct TaggedFieldAt<idx, TypeList<T, Ts...>> :
      public TaggedFieldAt<idx-1, TypeList<Ts...>> {};


template <uint16_t idx, typename T>
const typename TaggedFieldAt<idx, typename T::type_list>::type& FieldAt(const T& t) {
  return t;
};


}

#endif /* _GENSCATTERHIERARCHY_H_ */
