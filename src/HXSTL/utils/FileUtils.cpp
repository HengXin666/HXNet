#include <HXSTL/utils/FileUtils.h>

#include <fcntl.h>
#include <fstream>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXWeb/server/AsyncFile.h>

namespace HX { namespace STL { namespace utils {

std::string FileUtils::getFileContent(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::system_error(errno, std::generic_category());
    }
    return std::string {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void FileUtils::putFileContent(const std::string& path, std::string_view content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::system_error(errno, std::generic_category());
    }
    std::copy(content.begin(), content.end(), std::ostreambuf_iterator<char>(file));
}

HX::STL::coroutine::awaiter::Task<std::string> FileUtils::asyncGetFileContent(
    const std::string& path
) {
    // 这个是错误的!!!, epoll 不能监测普通文件...
    auto fd = HX::STL::tools::ErrorHandlingTools::convertError<int>(
        ::open(path.c_str(), O_RDONLY)
    ).expect("open: O_RDONLY");
    std::string res;
    std::vector<char> buf(1024);
    HX::web::server::AsyncFile file {fd};
    while (co_await file.asyncRead(buf, buf.size()) != buf.size()) {
        res += std::string_view {buf.data(), buf.size()};
        buf.clear();
    }
    co_return res += std::string_view {buf.data(), buf.size()};
}

}}} // namespace HX::STL::utils