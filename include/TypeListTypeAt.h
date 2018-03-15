#ifndef _TYPELISTTYPEAT_H_
#define _TYPELISTTYPEAT_H_

#include "TypeAt.h"
#include "TypeList.h"

namespace vapid_db {

// specialize TypeAt for TypeList
template <uint16_t idx, typename... Ts>
struct TypeAt<idx, TypeList<Ts...>> : public VariadicTypeAt<idx, Ts...> {};

}


#endif /* _TYPELISTTYPEAT_H_ */
