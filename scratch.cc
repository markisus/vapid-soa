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
    presidents.sort_by_field<LAST_NAME>([](auto& lname_a, auto& lname_b){ 
        return lname_a.size() < lname_b.size();
    });
    std::cout << presidents << "\n";

    // We can also sort by a view.
    std::cout << "Sorting by first name, last name." << "\n";
    presidents.sort_by_view<FIRST_NAME,LAST_NAME>();
    std::cout << presidents << "\n";

    // We can also use a comparator when sorting by view.
    // In that case, the comparator should take a tuple of references.
    // For example, we can sort by difference in length of last and first name.
    std::cout << "Sorting by length of first name - length of last name." << "\n";    
    presidents.sort_by_view<LAST_NAME, FIRST_NAME>([](auto view1, auto view2) {
        const auto& [lname_a, fname_a] = view1;
        const auto& [lname_b, fname_b] = view2;
        int view1_order = -int(lname_a.size()) + int(fname_a.size());
        int view2_order = -int(lname_b.size()) + int(fname_b.size());
        return view1_order < view2_order;
    });
    std::cout << presidents << "\n";

    return 0;
}
