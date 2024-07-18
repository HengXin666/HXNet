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
        struct sockaddr_in stSersock;                  // 声明 IPv4 地址结构体
        memset(&stSersock, 0, sizeof(stSersock));      // 初始化
        stSersock.sin_family = AF_INET;                // IPv4协议
        stSersock.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY 转换过来就是0.0.0.0，泛指本机的意思，也就是表示本机的所有IP，因为有些机子不止一块网卡，多网卡的情况下，这个就表示所有网卡ip地址的意思。
        stSersock.sin_port = htons(port);              // 端口

        // 设置端口复用
        int opt = 1;
        setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // 绑定
        if (::bind(_serverFd, (struct sockaddr *)&stSersock, sizeof(stSersock)) < 0) { // 将套接字绑定IP和端口用于监听
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

        LOG_INFO("====== Epoll已启动: \033[33m\033]8;;http://localhost:%d/\033\\http://localhost:%d/\033]8;;\033\\\033[0m\033[1;32m ======", port, port);
        return;
    } while (0);
    doError();
}

void HXEpoll::doError() {
    if (_serverFd != -1)
        ::close(_serverFd);
    if (_epollFd != -1)
        ::close(_epollFd);
    LOG_ERROR("Repeat: Error: %s (errno: %d)", strerror(errno), errno);
    exit(EXIT_FAILURE); // 错误处理
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

int HXEpoll::ctlAdd(int fd) {
    // --- 默认模式 --- | 那个高速模式我还不会?
    _ev.events = EPOLLIN;
    _ev.data.fd = fd;
    return ::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &_ev);
}

int HXEpoll::ctlDel(int fd) {
    return ::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
}

int HXEpoll::ctlMod(int fd) {
    // --- 封装不确定 ---
    return ::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &_ev);
}

void HXEpoll::run() {
    while (1) {
        int nfds = wait(-1);
        if (nfds == -1) {
            LOG_ERROR("wait Error: %s (errno: %d)", strerror(errno), errno);
            doError();
        }

        for (int i = 0; i < nfds; ++i) {
            int tmpFd = _events[i].data.fd;
            if (tmpFd == _serverFd) { // 新连接
                int newClientFd = ::accept(_serverFd, (struct sockaddr *)NULL, NULL);
                if (newClientFd < 0) {
                    LOG_ERROR("连接新客户端出错! %s (errno: %d)", strerror(errno), errno);
                } else {
                    LOG_INFO("[INFO]: 欢迎新的客户机连接! id = %d", newClientFd);

                    if (ctlAdd(newClientFd) < 0) {
                        LOG_ERROR("添加时候出现错误! %s (errno: %d)", strerror(errno), errno);
                        ::close(newClientFd);
                    }
                }
            } else { // 处理新来的信息
                int strLen = ::recv(tmpFd, msg, MAX_SIZE, 0);
                if (strLen < 0) {
                    LOG_ERROR("接收客户端[%d]消息出错: Error: %s (errno: %d)", tmpFd, strerror(errno), errno);
                } else if (strLen == 0 && !(errno == EWOULDBLOCK || errno == EAGAIN)) {
                    // 断开
                    ctlDel(tmpFd);
                    ::close(tmpFd);
                }
            }
        }
    }
}

} // namespace HXHttp
