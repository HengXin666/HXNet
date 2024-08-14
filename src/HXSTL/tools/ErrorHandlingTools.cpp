#include <HXSTL/tools/ErrorHandlingTools.h>

namespace HX { namespace STL { namespace tools {

[[noreturn]] void LinuxErrorHandlingTools::_throwSystemError(const char *what) {
    auto ec = std::error_code(errno, std::system_category());
    LOG_ERROR("%s: %s %s.%d", what, ec.message().c_str(), ec.category().name(), ec.value());
    throw std::system_error(ec, what);
}

}}} // namespace HX::STL::tools
