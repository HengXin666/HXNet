#include <HXSTL/utils/FileUtils.h>

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

HX::STL::coroutine::task::Task<std::string> FileUtils::asyncGetFileContent(
    const std::string& path,
    OpenMode flags /*= OpenMode::Read*/,
    mode_t mode /*= 0644*/
) {
    int fd = co_await HX::STL::coroutine::loop::IoUringTask().prepOpenat(
        AT_FDCWD, path.c_str(), static_cast<int>(flags), mode
    );
    std::string res;
    std::vector<char> buf(kBufMaxSize);
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

HX::STL::coroutine::task::Task<int> FileUtils::asyncPutFileContent(
    const std::string& path,
    std::string_view content,
    OpenMode flags,
    mode_t mode /*= 0644*/
) {
    int fd = co_await HX::STL::coroutine::loop::IoUringTask().prepOpenat(
        AT_FDCWD, path.c_str(), static_cast<int>(flags), mode
    );

    // 无需设置 offset, 因为内核会根据 open 的 flags 来
    int res = co_await HX::STL::coroutine::loop::IoUringTask().prepWrite(fd, content, static_cast<std::uint64_t>(-1));
    co_await HX::STL::coroutine::loop::IoUringTask().prepClose(fd);
    co_return res;
}

}}} // namespace HX::STL::utils