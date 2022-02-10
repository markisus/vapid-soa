# vapid soa
A simple c++17 header only library that implements structure of arrays data structure backed by std::vector.  
These are the most useful operations.  
- `.insert(field1, field2, ...)` field_n inserts into the nth array
- `.sort_by_field<col_idx>()` sort all columns in tandem based on particular column 
- `.operator[](row_idx)` read data out as tuple of references
- `.get_column<col_idx>()` direct access to underlying std::vector column
- `.view<col_idx1, col_idx2, ...>(row_idx)` read subset of the fields out as a tuple of references

Code Example (scratch.cpp)
------------------------

```c++
#include <iostream>
#include "vapid/soa.h"

int main(int argc, char *argv[])
{
    // presidents will be a soa representing
    // order, first name, last name

    constexpr int ORDER = 0;
    constexpr int FIRST_NAME = 1;
    constexpr int LAST_NAME = 2;
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
    presidents.sort_by_field<ORDER>();
    std::cout << "Presidents sorted by temporal order" << "\n";
    std::cout << presidents << "\n";

    // sort by first name (second column)
    presidents.sort_by_field<FIRST_NAME>();
    std::cout << "Presidents sorted by first name" << "\n";
    std::cout << presidents << "\n";

    // sort by last name (third column)
    presidents.sort_by_field<LAST_NAME>();
    std::cout << "Presidents sorted by last name" << "\n";
    std::cout << presidents << "\n";

    // operator[] returns a tuple of references
    // Let's update Joe Biden to Joseph Biden
    {
        std::cout << "Editing the first row to update Joe => Joseph" << "\n";
        auto [order, fname, lname] = presidents[0];
        fname = "Joseph"; // Joe => Joseph
        std::cout << presidents << "\n";
    }

    // view is templated to return a subset of fields 
    // Let's update Abraham Lincoln to George Washington
    {
        std::cout << "Editing the third row to update Abraham Lincoln => George Washington" << "\n";
        auto [fname, lname] = presidents.view<FIRST_NAME,LAST_NAME>(3);
        fname = "George";
        lname = "Washington";
        std::cout << presidents << "\n";
    }

    // get_column<idx> returns direct access
    // to the underlying std::vector.
    // Let's sum the characters of the first names.
    std::cout << "Summing first name lengths\n";
    int length_sum = 0;
    for (const auto& fname : presidents.get_column<FIRST_NAME>()) {
        length_sum += fname.length();
    }
    std::cout << "Total characters used in first names = " << length_sum << "\n\n";

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
This program outputs the following.
```
Presidents in order of insertion
soa {
	0, Abraham, Lincoln
	3, Barack, Obama
	2, George, Bush
	1, Bill, Clinton
	4, Donald, Trump
	5, Joe, Biden
}

Presidents sorted by temporal order
soa {
	0, Abraham, Lincoln
	1, Bill, Clinton
	2, George, Bush
	3, Barack, Obama
	4, Donald, Trump
	5, Joe, Biden
}

Presidents sorted by first name
soa {
	0, Abraham, Lincoln
	3, Barack, Obama
	1, Bill, Clinton
	4, Donald, Trump
	2, George, Bush
	5, Joe, Biden
}

Presidents sorted by last name
soa {
	5, Joe, Biden
	2, George, Bush
	1, Bill, Clinton
	0, Abraham, Lincoln
	3, Barack, Obama
	4, Donald, Trump
}

Editing the first row to update Joe => Joseph
soa {
	5, Joseph, Biden
	2, George, Bush
	1, Bill, Clinton
	0, Abraham, Lincoln
	3, Barack, Obama
	4, Donald, Trump
}

Editing the third row to update Abraham Lincoln => George Washington
soa {
	5, Joseph, Biden
	2, George, Bush
	1, Bill, Clinton
	0, George, Washington
	3, Barack, Obama
	4, Donald, Trump
}

Summing first name lengths
Total characters used in first names = 34

Sorting by number of characters in the last name.
soa {
	2, George, Bush
	5, Joseph, Biden
	3, Barack, Obama
	4, Donald, Trump
	1, Bill, Clinton
	0, George, Washington
}

```

Benchmark
-------
We can observe speed ups for structure of arrays (soa=vapid::soa) vs array of structs (vec=std::vector) with the benchmarks.cc.
Here are the results using Visual Studio 2022 on Release mode on my laptop.

```
# bazel run -c opt //:benchmarks
Run on (8 X 2995 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 1280 KiB (x4)
  L3 Unified 12288 KiB (x1)
---------------------------------------------------------------
Benchmark                     Time             CPU   Iterations
---------------------------------------------------------------
BM_SoaSortBySensorId   10599404 ns      9151786 ns           70
BM_VecSortBySensorId   24412036 ns     25111607 ns           28
BM_SoaSumTimestamps      106078 ns       106027 ns         5600
BM_VecSumTimestamps      264606 ns       266841 ns         2635
```

The benchmark contains a small program concerning simulated sensor measurements. With a straightforward array of structs, we store metadata together with the actual sensor data together and then just push_back() these onto an std::vector.
```c++
struct SensorData {
    std::array<double, 18> xyz;

struct Measurement {
    Id sensor_id;
    Id object_id;
    double timestamp;
    SensorData data;
};

...

std::vector<Measurement> measurements_vec;
measurements_vec.push_back(m);
```
Alternatively, we could split the metadata and sensor data apart into a structure of arrays.
```c++
vapid::soa<Id, Id, double, SensorData> measurements_soa;
soa.insert(m.sensor_id, m.object_id, m.timestamp, m.data);
```

The benchmark times the cost of sorting by sensor_id, and then the cost of finding the average measurement timestamp using an std::vector vs a vapid::soa.

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
    url = "https://github.com/markisus/vapid-soa/archive/d3c303a3fe15ebc4c8a7e3049bd21a3f663a42d0.zip",
    strip_prefix = "vapid-soa-d3c303a3fe15ebc4c8a7e3049bd21a3f663a42d0")
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

