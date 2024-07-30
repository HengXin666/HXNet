#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-7-16 13:51:13
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
#ifndef _HX_HXJSON_H_
#define _HX_HXJSON_H_

#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <regex>
#include <charconv>

// 下期目标: 实现一个将Json数据转存为Json文件的Code

namespace HX::Json {

struct JsonObject;

using JsonList = std::vector<JsonObject>;
using JsonDict = std::unordered_map<std::string, JsonObject>;

struct JsonObject {
    using JsonData = std::variant
    < std::nullptr_t  // null
    , bool            // true
    , int             // 42
    , long long       // 12321312321312312LL
    , double          // 3.14
    , std::string     // "hello"
    , JsonList        // [42, "hello"]
    , JsonDict        // {"hello": 985, "world": 211}
    >;

    JsonData _inner;

    explicit JsonObject() : _inner(std::nullptr_t{})
    {}

    explicit JsonObject(JsonData&& data) : _inner(data) 
    {}

    // 鸭子
    void print() const;

    template<class T>
    const T& get() const {
        return std::get<T>(_inner);
    }

    template <class T>
    T &get() {
        return std::get<T>(_inner);
    }
};

template <class T>
std::optional<T> try_parse_num(std::string_view str) {
    T value;
    // std::from_chars 尝试将 str 转为 T(数字)类型的值, 返回值是一个 tuple<指针 , T>
    // 值得注意的是 from_chars 不识别指数外的正号(在起始位置只允许出现负号)
    // 具体请见: https://zh.cppreference.com/w/cpp/utility/from_chars
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    if (res.ec == std::errc() && 
        res.ptr == str.data() + str.size()) { // 必需保证整个str都是数字
        return value;
    }
    return std::nullopt;
}

char unescaped_char(char c);


// 跳过末尾的空白字符 如: [1      , 2]
std::size_t skipTail(std::string_view json, std::size_t i, char ch);

// 更优性能应该使用栈实现的非递归

template<bool analysisKey = false>
std::pair<JsonObject, std::size_t> parse(std::string_view json) {
    if (json.empty()) { // 如果没有内容则返回空
        return {JsonObject{std::nullptr_t{}}, 0};
    } else if (std::size_t off = json.find_first_not_of(" \n\r\t\v\f\0"); off != 0 && off != json.npos) { // 去除空行
        auto [obj, eaten] = parse<analysisKey>(json.substr(off));
        return {std::move(obj), eaten + off};
    } else if (json[0] >= '0' && json[0] <= '9' || json[0] == '+' || json[0] == '-') { // 如果为数字
        // std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
        std::regex num_re{"[-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"}; // 一个支持识别内容是否为数字的: 1e-12, 114.514, -666
        std::cmatch match; // 匹配结果
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_re)) { // re解析成功
            std::string str = match.str();
            // 支持识别为 int 或者 double
            if (auto num = try_parse_num<int>(str)) {
                return {JsonObject{*num}, str.size()};
            } else if (auto num = try_parse_num<long long>(str)) {
                return {JsonObject{*num}, str.size()};
            } else if (auto num = try_parse_num<double>(str)) {
                return {JsonObject{*num}, str.size()};
            }
        }
    } else if (json[0] == '"') { // 识别字符串, 注意, 如果有 \", 那么 这个"是不识别的
        std::string str;
        enum {
            Raw,     // 前面不是'\'
            Escaped, // 前面是个'\'
        } phase = Raw;
        std::size_t i = 1;
        for (; i < json.size(); ++i) {
            char ch = json[i];
            if (phase == Raw) {
                if (ch == '\\') {
                    phase = Escaped;
                } else if (ch == '"') {
                    i += 1;
                    break;
                } else {
                    str += ch;
                }
            } else if (phase == Escaped) {
                str += unescaped_char(ch); // 处理转义字符
                phase = Raw;
            }
        }
        return {JsonObject{std::move(str)}, i};
    } else if (json[0] == '[') { // 解析列表
        JsonList res;
        std::size_t i = 1;
        for (; i < json.size(); ) {
            if (json[i] == ']') {
                i += 1;
                break;
            }
            auto [obj, eaten] = parse(json.substr(i)); // 递归调用
            if (eaten == 0) {
                i = 0;
                break;
            }
            i += eaten;
            res.push_back(std::move(obj));

            i += skipTail(json, i, ',');
        }
        return {JsonObject{std::move(res)}, i};
    } else if (json[0] == '{') { // 解析字典, 如果Key重复, 则使用最新的Key的Val
        JsonDict res;
        std::size_t i = 1;
        for (; i < json.size(); ) {
            if (json[i] == '}') {
                i += 1;
                break;
            }

            // 需要支持解析 不带双引号的 Key
            auto [key, keyEaten] = parse<true>(json.substr(i));
            
            if (keyEaten == 0) {
                i = 0;
                break;
            }
            i += keyEaten;
            if (!std::holds_alternative<std::string>(key._inner)) {
                i = 0;
                break;
            }

            i += skipTail(json, i, ':');

            auto [val, valEaten] = parse(json.substr(i));
            if (valEaten == 0) {
                i = 0;
                break;
            }
            i += valEaten;

            res.insert({std::move(key.get<std::string>()), std::move(val)});

            i += skipTail(json, i, ',');
        }
        return {JsonObject{std::move(res)}, i};
    } else if constexpr (analysisKey) { // 解析Key不带 ""
        if (std::size_t off = json.find_first_of(": \n\r\t\v\f\0"); off != json.npos)
            return {JsonObject{std::string{json.substr(0, off)}}, off};
    } else if (json.size() > 3) { // 解析 null, false, true
        switch (json[0]) {
        case 'n':
            if (json[1] == 'u' && json[2] == 'l' && json[3] == 'l')
                return {JsonObject{std::nullptr_t{}}, 4};
        case 't':
            if (json[1] == 'r' && json[2] == 'u' && json[3] == 'e')
                return {JsonObject{true}, 4};
        case 'f':
            if (json.size() > 4 && json[1] == 'a' && json[2] == 'l' && json[3] == 's' && json[4] == 'e')
                return {JsonObject{false}, 5};
        default:
            break;
        }
    }
    return {JsonObject{std::nullptr_t{}}, 0};
}

} // namespace HXJson

#endif // _HX_HXJSON_H_