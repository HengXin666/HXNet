#include <HXHttp/HXHttp.h>

namespace HXHttp {

} // namespace HXHttp

#define PORT 8080
#define MAX_EVENTS 10

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
#include <HXHttp/HXHttpTools.h>

#include <iostream>
#include <unordered_map>
#include <csignal>
#include <cstring>

// 全局变量: 是否退出
bool isAllowServerRun = true;

int main() {
    // 绑定交互信号监听 (Ctrl + C)
    signal(SIGINT, [](int signum) {
		isAllowServerRun = false;
	});

    // HXHttp::HXRouter::getSingleton();

    HXHttp::HXEpoll epoll{};
    
    epoll.setNewConnectCallback([](int fd) { // 新连接

    }).setNewMsgCallback([](int fd, char *str, std::size_t strLen) -> bool { // 处理
        // printf("%s", str);
        printf("%s", str);
        // Http 第一行是请求类型和 协议版本
        // 剩下的是键值对
        char *tmp = NULL;
        char *line = ::strtok_r(str, "\r\n", &tmp); // 线程安全
        /*
请求格式
GET /PTAH HTTP/1.1 # 可以确定, http协议的第一行, 必需是这个
Host: localhost:28205 # 从头开始, 寻找第一个`:`
         */
        if (line) { // 解析请求头
            // GET /PTAH HTTP/1.1
            auto requestLine = HXHttp::HXStringUtil::split(line, " ");
            std::unordered_map<std::string, std::string> requestHead;
            // requestLine[0] 请求类型
            // requestLine[1] 请求PTAH
            // requestLine[2] 请求协议
            printf("请求类型: [%s], 请求PTAH: [%s], 请求协议: [%s]\n",
                requestLine[0].c_str(),
                requestLine[1].c_str(),
                requestLine[2].c_str()
            );

            while ((line = ::strtok_r(NULL, "\r\n", &tmp))) { // 解析 请求行
                auto p = HXHttp::HXStringUtil::splitAtFirst(line, ": ");
                requestHead.insert(p);
                printf("%s -> %s\n", p.first.c_str(), p.second.c_str());
            }
        } else {  // 处理错误: 请求行不存在

        }
        return true; // http 是无感应的, 不是 WebSocket
    }).setNewUserBreakCallback([](int fd){ // 请求断开
        printf("没有东西\n");
    }).run(-1, [&](){ return isAllowServerRun; });
    
    return 0;
}


int _main() {
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    struct epoll_event event, events[MAX_EVENTS];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((epoll_fd = epoll_create1(0)) == -1) {
        perror("epoll_create1 failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl failed");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait failed");
            close(server_fd);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                if (new_socket == -1) {
                    perror("accept failed");
                } else {
                    event.events = EPOLLIN;
                    event.data.fd = new_socket;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
                        perror("epoll_ctl failed");
                        close(new_socket);
                    }
                }
            } else {
                handleClient(events[n].data.fd);
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
