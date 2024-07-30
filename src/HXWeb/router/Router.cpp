#include <HXWeb/router/Router.h>

#include <HXWeb/protocol/http/Request.h>
#include <HXSTL/tools/MagicEnum.h>
#include <HXSTL/tools/StringTools.h>

namespace HX { namespace web { namespace router {

void Router::addController(
    const std::string& requestType, 
    const std::string& path, 
    const Router::EndpointFunc& func
    ) {
    _routerRadixTree.insert(HX::STL::tools::StringUtil::split(path, "/", {requestType}), func);
}

Router::EndpointFunc Router::getEndpointFunc(const std::string& requestType, const std::string& path) {
    // 如果path是`/home/?loli=imouto`这种, 先不要解析参数, 应该只做?之前的映射(没有必要, 因为用户可以乱传输后面的参数)
    std::size_t pos = path.find('?');
    if (auto res = _routerRadixTree.find(HX::STL::tools::StringUtil::split(pos == std::string::npos ? path : path.substr(0, pos), "/", {requestType})))
        return *res;
    return nullptr;
}

}}} // namespace HX::web::router
