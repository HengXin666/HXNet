#include <HXHttp/HXRequestParsing.h>

#include <HXSTL/HXStringTools.h>

namespace HXHttp {

std::vector<int> HXRequestParsing::getPathWildcardAnalysisArr(std::string_view path) {
    auto arr = HXSTL::HXStringUtil::split(path, "/");
    std::vector<int> res;
    std::size_t n = arr.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (arr[i][0] == '{') {
            res.push_back(i);
        }
    }
    if (res.empty())
        throw "The path does not have wildcard characters ({?})";
    return res;
}

std::size_t HXRequestParsing::getUniversalWildcardPathBeginIndex(std::string_view path) {
    std::size_t pos = path.find("**");
    if (pos == std::string_view::npos)
        throw "The path does not have wildcard characters (**)";
    return pos;
}

} // namespace HXHttp
