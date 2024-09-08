#include <HXJson/Json.h>
#include <HXprint/print.h>
#include <HXSTL/concepts/SingleElementContainer.hpp>

#if 0
#include <variant>
#include <iostream>
#include <memory>

// 类型擦除接口
class IValue {
public:
    virtual ~IValue() = default;
    virtual void print() const = 0; // 用于展示值
};

// 类型擦除实现
template <typename T>
class ValueImpl : public IValue {
public:
    ValueImpl(T value) : value_(std::move(value)) {}
    void print() const override {
        std::cout << value_ << std::endl;
    }
private:
    T value_;
};

// 获取 std::variant 的当前值
std::unique_ptr<IValue> getVal(const std::variant<int, float, std::string>& v) {
    return std::visit([](const auto& arg) -> std::unique_ptr<IValue> {
        return std::make_unique<ValueImpl<decltype(arg)>>(arg);
    }, v);
}

int main() { // 使用类型擦除, 从而输出 variant 的实时的值
    std::variant<int, float, std::string> v = std::string("Hello World");
    
    auto valuePtr = getVal(v);
    valuePtr->print(); // 输出: Hello World

    return 0;
}
#endif

#ifdef JSON_TEST_MAIN

// JSON解析示例
void test_01() {
    auto json = HX::Json::parse(R"(
[
  {
    "name": "Molecule Man",
    "age": 29,
    "secretIdentity": "Dan Jukes",
    "powers": ["Radiation resistance", "Turning tiny", "Radiation blast"]
  },
  {
    "name": "Madame Uppercut",
    "age": 39.0000009,
    "secretIdentity": "Jane Wilson",
    "powers": [
      "Million tonne punch",
      "Damage resistance",
      "Superhuman reflexes"
    ],
    sb: null
  }
]
)").first;

    json.print();
    std::cout << '\n';
    
    json.get<HX::Json::JsonList>()[1].get<HX::Json::JsonDict>()["sb"].print();
    std::cout << '\n';

    json[1]["sb"].print();
    std::cout << '\n';

    json[0]["powers"][0].print();
    std::cout << '\n';

    auto str = json.toString();
    std::cout << str << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <HXJson/ReflectJson.hpp> // <-- 反射 宏的头文件: 对外提供的是 以`REFLECT`开头, 以`_`开头的是内部使用的宏

struct Student {
    std::string name;
    int age;
    struct Loli {
        std::string name;
        int age;
        int kawaiiCnt;

        REFLECT_CONSTRUCTOR_ALL(Loli, name, age, kawaiiCnt) // 可以嵌套, 但是也需要进行静态反射(需要实现`toString`方法)
    };
    std::vector<Loli> lolis;
    std::unordered_map<std::string, std::string> woc;
    std::unordered_map<std::string, Loli> awa;

    // 静态反射, 到时候提供`toString`方法以序列化为JSON
    // 提供 构造函数(从json字符串和json构造, 以及所有成员的默认构造函数)
    // 注: 如果不希望生成 [所有成员的默认构造函数], 可以使用 REFLECT_CONSTRUCTOR 宏
    REFLECT_CONSTRUCTOR_ALL(Student, name, age, lolis, woc, awa)
};

#include <HXJson/UnReflectJson.hpp> // <-- undef 相关的所有宏的头文件, 因为宏会污染全局命名空间

// JSON 序列化(结构体 toJsonString)示例
void test_02() {
    Student stu { // 此处使用了 宏生成的 [所有成员的默认构造函数] (方便我调试awa)
        "Heng_Xin",
        20,
        {{
            "ラストオーダー",
            13,
            100
        }, {
            "みりむ",
            14,
            100
        }},
        {
            {"hello", "word"},
            {"op", "ed"}
        },
        {
            {"hello", {
                "みりむ",
                14,
                100
            }}
        }
    };
    // 示例: 转化为json字符串(紧凑的)
    HX::print::print(stu.toString());
    auto json = HX::Json::parse(stu.toString()).first;
    json.print();
    printf("\n\n");

    // 示例: 从json对象 / json字符串转为 结构体
    Student x(json);
    HX::Json::parse(x.toString()).first.print();
}

int main () {
    HX::print::print("示例1: json解析\n");
    test_01();
    HX::print::print("\n\n示例2: json合成string || jsonString合成到结构体\n");
    test_02();
    return 0;
}
#endif // JSON_TEST_MAIN