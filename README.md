# vapid soa
A simple header only library that implements structure of arrays data structure backed by std::vector.  
These are the most useful operations.  
- `.quick_sort_by_field<field_idx>()` sort all columns in tandem based on particular column 
- `.as_tuple(row_idx)` read data out as tuple
- `.get_column<field_idx>()` direct access to underlying columns
- `.dump(std::cout)` pretty the printing contents 

Code Example (main.cpp)
------------------------

```c++
#include <iostream>
#include "include/soa.h"

int main(int argc, char *argv[])
{
    vapid::soa<double, std::string, int> mySoa;

    std::cout << "Insertion:" << std::endl;
    mySoa.insert(5.02, "helloooo", 3);
    mySoa.insert(6.23, "gobye", 2);
    mySoa.insert(3.14, "godbye", 1000);
    mySoa.insert(444.14, "eh", 1000);
    mySoa.insert(5.04, "hellooo", 3);
    mySoa.insert(6.25, "goodby", 2);
    mySoa.insert(3.18, "godye", 1000);
    mySoa.insert(444.19, "ehh", 1000);
    mySoa.insert(-2.134, "h", 1000);
    mySoa.dump(std::cout);

    std::cout << "Overwriting first entry:" << std::endl;
    mySoa.overwrite(0, std::make_tuple(4.099, "fudge", 1));
    mySoa.dump(std::cout);

    std::cout << "Resizing:" << std::endl;
    mySoa.resize(mySoa.size()-1);
    mySoa.dump(std::cout);

    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());

    std::cout << "Sorting:" << std::endl;
    mySoa.quick_sort(rng);
    mySoa.dump(std::cout);

    std::cout << "Sorting by field:" << std::endl;
    mySoa.quick_sort_by_field<1>(rng);
    mySoa.dump(std::cout);

    // direct access to partcular column
    std::cout << "Multiplying last column by half" << std::endl;
    for (auto& d : mySoa.get_column<2>()) {
        std::cout << "\t" << d << "\n";
        d *= 0.5;
    }
    mySoa.dump(std::cout);
    
    return 0;
}

```
