#include <HXCodeTest/HXCodeTest.h>
#include <HXprint/HXprint.h>
#include <HXJson/HXJson.h>

namespace HXCodeTest {

} // namespace HXCodeTest

using namespace std;
using namespace HXJson;
using namespace HXprint;

int main() {
        std::string_view str = R"Json({
    "name": "Json.CN",
    url: "http://www.json.cn",
    "page": 88,
    "isNonProfit": true,
    "address": {
        "street": "科技园路.",
        "city": "江苏苏州",
        "country": "中国"
    },
    "links": [
        {
            "name": "Google",
            "url": "http://www.google.com"
        },
        {
            "name": "Baidu",
            "url": "http://www.baidu.com"
        },
        {
            "name": "SoSo",
            "url": "http://www.SoSo.com"
        }
    ]
})Json";
    auto [obj, eaten] = parse(str);
    print(obj);
    print("\n只是字符串");
    optional<string> x;
    x = "字符串啦";
    print(x);
    print(NULL);
    print(nullptr);
    print(true);
    print('\n');
    vector<int> *p = NULL;
    print(p);
    return 0;
}