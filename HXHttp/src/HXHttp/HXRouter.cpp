#include <HXHttp/HXRouter.h>

#include <HXHttp/HXRequest.h>
#include <HXSTL/HXMagicEnum.h>

namespace HXHttp {

HXRouter::HXRouter() : _routerMap() {
    // 注册请求类型, 只注册常见的几种
    _routerMap[HXSTL::MagicEnum::getEnumName(HXRequest::RequestType::GET)];
    _routerMap[HXSTL::MagicEnum::getEnumName(HXRequest::RequestType::POST)];
    _routerMap[HXSTL::MagicEnum::getEnumName(HXRequest::RequestType::PUT)];
    _routerMap[HXSTL::MagicEnum::getEnumName(HXRequest::RequestType::DELETE)];
}

} // namespace HXHttp
