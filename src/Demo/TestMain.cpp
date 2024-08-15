#include <iostream>

/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-31 12:13:49
 * @brief: 测试程序, 用于测试代码!
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * */

#include <functional>
#include <HXSTL/container/RadixTree.h>
/// @brief 压缩前缀树测试
void testRadixTree() {
    using func = std::function<void()>;
    HX::STL::container::RadixTree<func> tree;
    tree.insert({"awa", "qwq", "*"}, [](){
        printf("awa");
    });
    tree.insert({"awa", "0.0", "*"}, [](){
        printf("qwq");
    });
    (*tree.find({"awa", "0.0", "*"}))();
    if (!tree.find({"qwq", "0.0"}).has_value())
        printf("\n没有这个\n");
}

#include <HXSTL/utils/MagicEnum.h>
/// @brief 魔法枚举测试
void testMagicEnum() {
    enum MyEnum {
        LoLi = 1,
        imouto = 8,
    };
    std::cout << HX::STL::utils::MagicEnum::getEnumName<MyEnum>(MyEnum::imouto) << '\n';
    std::cout << HX::STL::utils::MagicEnum::nameFromEnum<MyEnum>("imouto") << '\n';
}

// int main() {
//     testServer();
//     return 0;
// }