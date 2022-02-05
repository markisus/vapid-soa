# vapid soa
A simple header only library that implements structure of arrays data structure backed by std::vector.  
These are the most useful operations.  
- `.sort_by_field<col_idx>()` sort all columns in tandem based on particular column 
- `operator[row_idx]` read data out as tuple of references
- `.get_column<col_idx>()` direct access to underlying std::vector column

Code Example (scratch.cpp)
------------------------

```c++
#include <iostream>
#include "vapid/soa.h"

int main(int argc, char *argv[])
{
    // presidents will be a soa representing
    // order, first name, last name
    vapid::soa<int, std::string, std::string> presidents;

    presidents.insert(0, "Abraham", "Lincoln");
    presidents.insert(3, "Barack", "Obama");
    presidents.insert(2, "George", "Bush");
    presidents.insert(1, "Bill", "Clinton");
    presidents.insert(4, "Donald", "Trump");
    presidents.insert(5, "Joe", "Biden");

    std::cout << "Presidents in order of insertion" << "\n";
    std::cout << presidents << "\n";

    // sort by time (first column)
    presidents.sort_by_field<0>();
    std::cout << "Presidents sorted by temporal order" << "\n";
    std::cout << presidents << "\n";

    // sort by first name (second column)
    presidents.sort_by_field<1>();
    std::cout << "Presidents sorted by first name" << "\n";
    std::cout << presidents << "\n";

    // sort by last name (third column)
    presidents.sort_by_field<2>();
    std::cout << "Presidents sorted by last name" << "\n";
    std::cout << presidents << "\n";

    // operator[] returns a tuple of references
    // Let's update Joe Biden to Joseph Biden
    auto [order, fname, lname] = presidents[0];
    fname = "Joseph"; // Joe => Joseph
    std::cout << "Editing the first row to update Joe=>Joseph" << "\n";
    std::cout << presidents << "\n";

    // get_column<idx> returns direct access
    // to the underlying std::vector.
    // Let's update the first name column, changing
    // the entry George to George W.
    std::cout << "Editing the second row to update George=>George W." << "\n";
    auto& first_names = presidents.get_column<1>();
    first_names[1] = "George W.";
    std::cout << presidents << "\n";

    // We can pass a custom comparator when sorting
    // Let's sort based on length of last name
    std::cout << "Sorting by number of characters in the last name." << "\n";
    presidents.sort_by_field<2>([](auto& lname_a, auto& lname_b){ 
        return lname_a.size() < lname_b.size();
    });
    std::cout << presidents << "\n";

    return 0;
}

```

Manual Installation
-----------
Copy the vapid folder into your project and #include "vapid/soa.h"  
  
Bazel Installation
------
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

