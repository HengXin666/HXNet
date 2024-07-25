#include <HXHttp/HXHttp.h>

#include <iostream>
#include <iosfwd>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <cstring>
#include <vector>

namespace HXHttp {

} // namespace HXHttp

#include <functional>
#include <string>
#include <unordered_map>

class EndpointInfo {
public:
    std::string name;
    std::string path;
    std::string method;
    std::unordered_map<std::string, std::string> pathParams;
};

template<typename T>
class Handler {
public:
    using HandlerFunc = std::function<void(T*, const std::unordered_map<std::string, std::string>&)>;

    Handler(T* controller, HandlerFunc func)
        : m_controller(controller), m_handlerFunc(func) {}

    void execute(const std::unordered_map<std::string, std::string>& requestParams) {
        m_handlerFunc(m_controller, requestParams);
    }

private:
    T* m_controller;
    HandlerFunc m_handlerFunc;
};

#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>

class MyController {
private:
    std::unordered_map<std::string, std::shared_ptr<Handler<MyController>>> m_endpoints;

public:
    void registerEndpoint(const std::string& endpointName, const std::string& path,
                          const std::string& method, std::function<void(MyController*, const std::unordered_map<std::string, std::string>&)> handlerFunc) {
        auto handler = std::make_shared<Handler<MyController>>(this, handlerFunc);
        m_endpoints[endpointName] = handler;
    }

    std::shared_ptr<Handler<MyController>> getEndpoint(const std::string& endpointName) {
        auto it = m_endpoints.find(endpointName);
        if (it != m_endpoints.end()) {
            return it->second;
        }
        return nullptr;
    }

    // 示例处理函数
    void handleGetRequest(const std::unordered_map<std::string, std::string>& requestParams) {
        std::cout << "Handling GET request" << std::endl;
        for (const auto& param : requestParams) {
            std::cout << param.first << " -> " << param.second << std::endl;
        }
    }
};

int _main() {
    MyController controller;

    // 注册端点
    controller.registerEndpoint("getUserById", "/users/{userId}", "GET", &MyController::handleGetRequest);

    // 模拟请求参数
    std::unordered_map<std::string, std::string> requestParams = {
        {"userId", "123"}
    };

    // 获取并调用端点处理函数
    auto endpointHandler = controller.getEndpoint("getUserById");
    if (endpointHandler) {
        endpointHandler->execute(requestParams);
    } else {
        std::cout << "Endpoint not found" << std::endl;
    }

    return 0;
}

#include <HXHttp/HXRouter.h>
#include <HXSTL/HXStringTools.h>
#include <HXHttp/HXRequest.h>
#include <HXHttp/HXResponse.h>
#include <HXprint/HXprint.h>
#include <HXHttp/HXServer.h>

#include <iostream>
#include <unordered_map>
#include <csignal>
#include <cstring>

// 全局变量: 是否退出
bool isAllowServerRun = true;

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    try {
        HXHttp::HXServer::Epoll ctx;
        // @bug: tmd 端口还只能绑定系统有的?! wcnmddsb fsl!!!
        auto ptr = HXHttp::HXServer::Acceptor::make();
        ptr->start("127.0.0.1", "28205");
        ctx.join();
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}