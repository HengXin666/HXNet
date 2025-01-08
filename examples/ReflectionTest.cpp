#ifdef _HX_REFLECTION_TEST_H_

#include <iostream>

#include <HXSTL/reflection/MemberCount.hpp>

struct Cat {
    int id;
    char name;
};

int main() {
    std::cout << HX::STL::reflection::membersCountVal<Cat> << '\n';
    return 0;
}

#endif // !_HX_REFLECTIONTEST_H_