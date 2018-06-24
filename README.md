# VapidDb
A stupid in-process relational database library

Code Example (main.cpp
------------------------

```c++
#include <iostream>

#include "include/BasicRelation.h"

#include "include/GenScatterHierarchy.h"



int main(int argc, char *argv[])

{

  std::cout << "Insertion:" << std::endl;

    vapid_db::BasicRelation<double, std::string, int> myRelation;

    myRelation.insert(5.02, "helloooo", 3);

    myRelation.insert(6.23, "gobye", 2);

    myRelation.insert(3.14, "godbye", 1000);

    myRelation.insert(444.14, "eh", 1000);

    myRelation.insert(-2.14, "e", 1000);

    myRelation.insert(5.04, "hellooo", 3);

    myRelation.insert(6.25, "goodby", 2);

    myRelation.insert(3.18, "godye", 1000);

    myRelation.insert(444.19, "ehh", 1000);

    myRelation.insert(-2.134, "h", 1000);

    myRelation.dump(std::cout);



    // overwriting

    std::cout << "Overwriting:" << std::endl;

    myRelation.overwrite(0, std::make_tuple(4.099, "fudge", 1));

    myRelation.dump(std::cout);



    std::cout << "Resizing:" << std::endl;

    myRelation.resize(myRelation.size()-1);

    myRelation.dump(std::cout);



    std::random_device rd;     // only used once to initialise (seed) engine

    std::mt19937 rng(rd());



    std::cout << "Sorting:" << std::endl;

    myRelation.quick_sort(rng);

    myRelation.dump(std::cout);



    std::cout << "Sorting by field:" << std::endl;

    myRelation.quick_sort_by_field<1>(rng);

    myRelation.dump(std::cout);



    return 0;

}
```
