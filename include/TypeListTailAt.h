#ifndef _TYPELISTTAILAT_H_
#define _TYPELISTTAILAT_H_

#include "TailAt.h"
#include "TypeList.h"

namespace vapid_db {

// specialize TypeAt for TypeList
template <uint16_t idx, typename... Ts>
struct TailAt<idx, TypeList<Ts...>> : public VariadicTailAt<idx, Ts...> {};

}

#endif /* _TYPELISTTAILAT_H_ */
