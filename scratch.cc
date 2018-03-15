#include <iostream>
#include "include/BasicRelation.h"
#include "include/GenScatterHierarchy.h"

int main(int argc, char *argv[])
{
    vapid_db::BasicRelation<double, std::string, int> myRelation;
    myRelation.insert(5.02, "helloooo", 3);
    myRelation.insert(6.23, "goodbye", 2);
    myRelation.insert(3.14, "goodbye", 1000);
    myRelation.swap_indices(0,1);
    myRelation.overwrite(0, std::make_tuple(4.099, "fuck", 1));
    myRelation.swap_matches_to_back(
        [](const std::tuple<double, std::string, int>& t) {
          return std::get<1>(t) == "fuck";
        });
    myRelation.resize(2);
    for (int i = 0; i < myRelation.size(); ++i) {
      double a; std::string b; int c;
      std::tie(a, b, c) = myRelation[i];
      std::cout << a << ", " << b << ", " << c << std::endl;
    }
    return 0;
}
