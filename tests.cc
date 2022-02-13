#include <gtest/gtest.h>
#include <iostream>
#include "vapid/soa.h"

template <typename T>
bool is_sorted(const std::vector<T>& l) {
    for (size_t idx = 1; idx < l.size(); ++idx) {
        if (l[idx] < l[idx-1]) {
            return false;
        }
    }
    return true;
}

TEST(SortA, Sorting) {
    vapid::soa<int, int> soa;
    soa.insert(0, 1);
    soa.insert(2, 5);
    soa.insert(4, 9);
    soa.insert(1, 0);
    soa.sort_by_field<0>();
    EXPECT_TRUE(is_sorted(soa.get_column<0>()));
}
