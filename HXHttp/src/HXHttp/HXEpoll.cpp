#include <HXHttp/HXEpoll.h>

#include <netinet/in.h> // INADDR_ANY等宏的数据
#include <sys/socket.h> // 套接字
#include <cstring>      // 错误码 to 错误码对应原因
#include <cerrno>       // 错误码
#include <unistd.h>     // 提供了 close

#include <HXprint/HXprint.h>
#include <HXJson/HXJson.h>

namespace HXHttp {

HXEpoll::HXEpoll(int port, int maxQueue, int maxConnect) : _maxConnect(maxConnect) {
    do {
        // 建立socket套接字
        if ((_serverFd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            LOG_ERROR("socket Error: %s (errno: %d)", strerror(errno), errno);
            break;
        }

        // 初始化服务器的网络配置
        struct sockaddr_in st_sersock;                  // 声明 IPv4 地址结构体
        memset(&st_sersock, 0, sizeof(st_sersock));     // 初始化
        st_sersock.sin_family = AF_INET;                // IPv4协议
        st_sersock.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY 转换过来就是0.0.0.0，泛指本机的意思，也就是表示本机的所有IP，因为有些机子不止一块网卡，多网卡的情况下，这个就表示所有网卡ip地址的意思。
        st_sersock.sin_port = htons(port);              // 端口

        // 设置端口复用
        int opt = 1;
        setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // 绑定
        if (::bind(_serverFd, (struct sockaddr *)&st_sersock, sizeof(st_sersock)) < 0) { // 将套接字绑定IP和端口用于监听
            LOG_ERROR("bind Error: %s (errno: %d)", strerror(errno), errno);
            break;
        }

        // 设置监听: 设定可同时排队的客户端最大连接个数
        if (::listen(_serverFd, maxQueue) < 0) {
            LOG_ERROR("listen Error: %s (errno: %d)", strerror(errno), errno);
            break;
        }

        // --- 创建epoll 实例 ---
        if ((_epollFd = ::epoll_create(maxConnect)) < 0) {
            LOG_ERROR("epoll_create Error: %s (errno: %d)", strerror(errno), errno);
            break;
        }

        // --- 将监听套接字添加到 epoll 实例中 ---
        _ev.events = EPOLLIN;
        _ev.data.fd = _serverFd;
        if (::epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &_ev) < 0) {
            LOG_ERROR("epoll_ctl Error: %s (errno: %d)", strerror(errno), errno);
            break;
        }
        
        // --- 初始化信息到类 ---
        _events = new struct ::epoll_event[maxConnect];

        LOG_INFO("====== Epoll已启动: \033]8;;http://localhost:%d/\033\\http://localhost:%d/\033]8;;\033\\ ======", port, port);
        return;
    } while (0);
    exit(-1); // 错误处理
}

HXEpoll::~HXEpoll() {
    ::close(_epollFd);
	::close(_serverFd);
    delete[] _events;
    LOG_INFO("====== Epoll已关闭: True ======");
}

int HXEpoll::wait(int timeOut) {
    return ::epoll_wait(_epollFd, _events, _maxConnect, timeOut);
}

int HXEpoll::ctl_add(int fd) {
    // --- 默认模式 --- | 那个高速模式我还不会?
    _ev.events = EPOLLIN;
    _ev.data.fd = fd;
    return ::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &_ev);
}

int HXEpoll::ctl_del(int fd) {
    return ::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
}

int HXEpoll::ctl_mod(int fd) {
    // --- 封装不确定 ---
    return ::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &_ev);
}

} // namespace HXHttp
