#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <iostream>

#include <HXSTL/reflection/MemberCount.hpp>
#include <HXSTL/reflection/MemberName.hpp>

struct Cat {
    int id;
    char name;
};

TEST_CASE("测试: 获取类的成员变量个数") {
    constexpr int cnt = HX::STL::reflection::membersCount<Cat>();
    CHECK(cnt == 2);
}

TEST_CASE("测试: 获取类的所有成员变量的名称") {
    constexpr auto arr = HX::STL::reflection::getMembersNames<Cat>();
    for (auto it : arr)
        std::cout << it << '\n';
    CHECK(arr[0] == "id");
    CHECK(arr[1] == "name");
}