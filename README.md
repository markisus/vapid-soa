# vapid soa
A simple header only library that implements structure of arrays data structure backed by std::vector.  
These are the most useful operations.  
- `.quick_sort_by_field<col_idx>()` sort all columns in tandem based on particular column 
- `.as_tuple(row_idx)` read data out as tuple
- `.get_column<col_idx>()` direct access to underlying std::vector column

Code Example (scratch.cpp)
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

    std::cout << "Accessing a particular entry as tuple:" << std::endl;
    std::tuple<double, std::string, int> entry0 = mySoa.as_tuple(0);
    std::cout << "\t" << std::get<0>(entry0) << ", " << std::get<1>(entry0) << ", " << std::get<2>(entry0) << "\n";


    std::cout << "Overwriting first entry:" << std::endl;
    mySoa.overwrite(0, std::make_tuple(4.099, "fudge", 1));
    std::cout << mySoa << std::endl;

    std::cout << "Resizing:" << std::endl;
    mySoa.resize(mySoa.size()-1);
    std::cout << mySoa << std::endl;

    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());

    std::cout << "Sorting:" << std::endl;
    mySoa.quick_sort(rng);
    std::cout << mySoa << std::endl;
    
    std::cout << "Sorting by field:" << std::endl;
    mySoa.quick_sort_by_field<1>(rng);
    std::cout << mySoa << std::endl;

    // direct access to particular column
    std::cout << "Multiplying last column by half" << std::endl;
    for (auto& d : mySoa.get_column<2>()) {
        d *= 0.5;
    }
    std::cout << mySoa << std::endl;
    
    return 0;
}
```

Installation
-----------
Manual: Copy the vapid folder into your project and #include "vapid/soa.h"
Bazel:
```starlark
# WORKSPACE
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_markisus_vapid-soa",
    url = "https://github.com/markisus/vapid-soa/archive/9c4d431e7cc02d09ffb073960c85aa090bf5938e.zip",
    sha256 = "4a57761c50d868cb25f9910b03ce0a2e32ac5009b019c2684c43b411663952c7",
    strip_prefix = "vapid-soa-9c4d431e7cc02d09ffb073960c85aa090bf5938e")
```
```starlark
# BUILD
cc_binary(
    ...
    deps = [
        "@com_github_markisus_vapid-soa//:soa",
        ...
    ]
)
```

