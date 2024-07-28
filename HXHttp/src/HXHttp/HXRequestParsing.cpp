#include <HXHttp/HXRequestParsing.h>

#include <HXSTL/HXStringTools.h>

namespace HXHttp {

std::vector<int> HXRequestParsing::pathWildcardAnalysis(std::string_view path) {
    auto arr = HXSTL::HXStringUtil::split(path, "/");
    std::vector<int> res;
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i][0] == '{') {
            res.push_back(i);
        }
    }
    if (res.empty())
        throw "The path does not have wildcard characters";
    return res;
}

} // namespace HXHttp
