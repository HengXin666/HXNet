#include <HXHttp/HXHttp.h>

namespace HXHttp {

} // namespace HXHttp

#define PORT 8080
#define MAX_EVENTS 10

std::vector<std::string> split(const std::string &str, char delimiter) {
    std::vector<std::string> res;
    if (str == "")
        return res;
	// 在字符串末尾也加入分隔符，方便截取最后一段
	std::string strs = str + delimiter;
	size_t pos = strs.find(delimiter);
 
	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos) {
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		// 去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(delimiter);
	}
    return res;
}

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);

    std::string request(buffer);
    std::vector<std::string> lines = split(request, '\n');

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

#include <iostream>
#include <HXHttp/HXEpoll.h>
#include <csignal>

void printClickableLink(const std::string& text, const std::string& url) {
    // ANSI escape sequence for clickable hyperlink
    std::cout << "\033]8;;" << url << "\033\\" << text << "\033]8;;\033\\" << std::endl;
}

// 全局变量: 是否退出
bool isAllowServerRun = true;

int main() {
    printClickableLink("OpenAI", "https://www.openai.com");
    printClickableLink("Google", "https://www.google.com");

    // 绑定交互信号监听（Ctrl + C）
    signal(SIGINT, [](int signum) {
		isAllowServerRun = false;
	});

    HXHttp::HXEpoll epoll{};
    
    epoll.setNewConnectCallback([](int fd) {

    }).setNewMsgCallback([](int fd, char *str, std::size_t strLen) {

    }).setNewUserBreakCallback([](int fd){

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
