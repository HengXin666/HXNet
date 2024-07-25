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

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);

    std::string request(buffer);
    std::vector<std::string> lines;// = split(request, '\n');

    std::cout << "Request received:" << std::endl;
    for (const auto &line : lines) {
        std::cout << line << std::endl;
    }

    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";
    
    send(client_socket, response.c_str(), response.length(), 0);
    close(client_socket);
}

#include <HXHttp/HXEpoll.h>
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
        {
            HXHttp::HXServer::Epoll ctx;
            HXHttp::HXServer::Acceptor::make()->start("127.0.0.1", "28205");
            ctx.join();
        }
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    return 0;
}

int _hx_main() {
    // 绑定交互信号监听 (Ctrl + C)
    signal(SIGINT, [](int signum) {
		isAllowServerRun = false;
	});

    // HXHttp::HXRouter::getSingleton();

    HXHttp::HXEpoll epoll{};
    
    epoll.setNewConnectCallback([](int fd) { // 新连接

    }).setNewMsgCallback([](int fd, char *str, const std::size_t strLen) -> bool { // 处理
        // Http 第一行是请求类型和 协议版本
        // 剩下的是键值对
        do {
            // 初步解析完毕, 路由映射跳转
            HXHttp::HXRequest req;
            if (req.resolutionRequest(fd, str, strLen) != HXHttp::HXRequest::ParseStatus::ParseSuccessful)
                break;
            HXHttp::HXResponse response {HXHttp::HXResponse::Status::CODE_200};
            if (response.setContentType("text/html", "UTF-8")
                        .setBodyData("<h1>Hello, world!</h1><h2>" + HXSTL::HXDateTimeFormat::format() + "</h2>")
                        .sendResponse(fd) == -1) {
                LOG_ERROR("发送信息时出现错误: %s (errno: %d)", strerror(errno), errno);
            }
            return true; // http 是无感应的, 不是 WebSocket
            // if (auto fun = HXHttp::HXRouter::getSingleton().getEndpointFunByURL(requestLine[1])) {
            //     printf("NO!");
            //     fun();
            // }
        } while(0);  // 处理错误: 请求行不存在

        return true;
    }).setNewUserBreakCallback([](int fd){ // 请求断开
        printf("没有东西\n");
    }).run(-1, [&](){ return isAllowServerRun; });
    
    return 0;
}