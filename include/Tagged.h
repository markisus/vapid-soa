#ifndef _TAGGED_H_
#define _TAGGED_H_

namespace vapid_db {

template <typename T, typename...>
struct Tagged : public T {};

}

#endif /* _TAGGED_H_ */
