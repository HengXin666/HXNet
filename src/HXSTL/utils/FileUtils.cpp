#include <HXSTL/utils/FileUtils.h>

#include <fcntl.h>
#include <fstream>

#include <HXSTL/tools/ErrorHandlingTools.h>
#include <HXSTL/coroutine/loop/IoUringLoop.h>

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
    int fd = co_await HX::STL::coroutine::loop::IoUringTask().prepOpenat(
        AT_FDCWD, path.c_str(), O_RDONLY, 0644
    );
    std::string res;
    std::vector<char> buf(1024);
    std::size_t len = 0;
    uint64_t offset = 0;
    while ((len = static_cast<std::size_t>(
            co_await HX::STL::coroutine::loop::IoUringTask().prepRead(fd, buf, offset)
        )) == buf.size()) {
        res += std::string_view {buf.data(), buf.size()};
        offset += buf.size();
    }
    co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
    co_return (res.append(std::string_view {buf.data(), len}));
}

}}} // namespace HX::STL::utils