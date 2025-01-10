#ifdef _HX_REFLECTION_TEST_H_

#include <iostream>

#include <HXSTL/reflection/MemberCount.hpp>
#include <HXSTL/reflection/MemberName.hpp>

struct Cat {
    int id;
    char name;
};

int main() {
    // auto x = std::make_index_sequence<2>();
    constexpr auto arr = HX::STL::reflection::getMembersNames<Cat>();
    for (auto it : arr)
        std::cout << it << '\n';
    std::cout << HX::STL::reflection::membersCountVal<Cat> << '\n';
    return 0;
}

#endif // !_HX_REFLECTIONTEST_H_