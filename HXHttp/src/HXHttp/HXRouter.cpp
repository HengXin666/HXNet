#include <HXHttp/HXRouter.h>

#include <HXHttp/HXRequest.h>
#include <HXSTL/HXMagicEnum.h>
#include <HXSTL/HXStringTools.h>

namespace HXHttp {

void HXRouter::addController(
    const std::string& requestType, 
    const std::string& path, 
    const HXRouter::EndpointFunc& func
    ) {
    _routerRadixTree.insert(HXSTL::HXStringUtil::split(path, "/", {requestType}), func);
}

HXRouter::EndpointFunc HXRouter::getEndpointFunc(const std::string& requestType, const std::string& path) {
    // 如果path是`/home/?loli=imouto`这种, 先不要解析参数, 应该只做?之前的映射(没有必要, 因为用户可以乱传输后面的参数)
    std::size_t pos = path.find('?');
    if (auto res = _routerRadixTree.find(HXSTL::HXStringUtil::split(pos == std::string::npos ? path : path.substr(0, pos), "/", {requestType})))
        return *res;
    return nullptr;
}

} // namespace HXHttp
