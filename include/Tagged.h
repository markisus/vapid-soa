#ifndef _TAGGED_H_
#define _TAGGED_H_

namespace vapid {

// The Tagged helper allows you to glom arbitrary
// types onto some actual type.
//
// Eg, suppose we have these two tags.
// struct MyTagA {};
// struct MyTagB {};
// 
// Tagged<std::string, MyTagA, MyTagB> is a type that
// inherits from std::string, and MyTagA and MyTagB
// have no runtime effects.

template <typename T, typename...>
struct Tagged : public T {};

}

#endif /* _TAGGED_H_ */
