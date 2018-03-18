#include <iostream>
#include "include/BasicRelation.h"
#include "include/GenScatterHierarchy.h"

int main(int argc, char *argv[])
{
    vapid_db::BasicRelation<double, std::string, int> myRelation;
    myRelation.insert(5.02, "helloooo", 3);
    myRelation.insert(6.23, "goodbye", 2);
    myRelation.insert(3.14, "goodbye", 1000);
    myRelation.insert(444.14, "eh", 1000);
    myRelation.insert(-2.14, "eh", 1000);
    myRelation.insert(5.04, "helloooo", 3);
    myRelation.insert(6.25, "goodbye", 2);
    myRelation.insert(3.18, "goodbye", 1000);
    myRelation.insert(444.19, "eh", 1000);
    myRelation.insert(-2.134, "eh", 1000);

    myRelation.overwrite(0, std::make_tuple(4.099, "fudge", 1));
    /*
    myRelation.swap_indices(0,5);
    size_t result = myRelation.swap_matches_to_back(
        [](const std::tuple<double, std::string, int>& t) {
          return std::get<0>(t) >= 5.04;
        },        1, myRelation.size());
    std::cout << "matches start at " << result << std::endl;
    */
    myRelation.resize(myRelation.size()-1);
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());
    myRelation.quick_sort(rng);
    myRelation.dump(std::cout);
    return 0;
}
