#include <HXHttp/HXHttpTools.h>

/**
 * 注: SV开头是适配std::string_view
 */
#ifdef __linux__
#include <string.h>
#define S_CPY(de, len, sr) snprintf(de, len, "%s", sr.c_str())
#define SV_CPY(de, len, sr) snprintf(de, len, "%s", sr.data())
#define STOK strtok_r
#else
#define S_CPY(de, len, sr) strcpy_s(de, len, sr.c_str())
#define SV_CPY(de, len, sr) strcpy_s(de, len, sr.data())
#define STOK strtok_s
#endif

namespace HXHttp {

std::vector<std::string> HXStringUtil::split(std::string_view str, std::string_view delim)  {
    std::vector<std::string> res;
    // 空字符处理
    if ("" == str) 
        return res;

    // 字符串从string类型转换为char*类型
    char* source = new char[str.length() + 1];
    SV_CPY(source, str.length() + 1, str);
    char* d = new char[delim.length() + 1];
    SV_CPY(d, delim.length() + 1, delim);

    // 拆分字符串逻辑
    char* nextToken = NULL;
    char* strToken = STOK(source, d, &nextToken);
    while (strToken) {
        // 分割得到的字符串转换为string类型
        res.emplace_back(strToken);
        // 继续分隔
        strToken = STOK(NULL, d, &nextToken);
    }
    delete[] source;
    delete[] d;
    return res;
}

std::pair<std::string, std::string> HXStringUtil::splitAtFirst(std::string_view str, std::string_view delim) {
    std::pair<std::string, std::string> res;
    std::size_t pos = str.find(delim);
    if (pos != std::string_view::npos) {
        res.first = str.substr(0, pos);
        res.second = str.substr(pos + delim.size());
    } else {
        res.first = str;
        res.second = "";
    }
    return res;
}

} // namespace HXHttp
