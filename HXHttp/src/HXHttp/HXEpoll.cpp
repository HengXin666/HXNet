#include <HXHttp/HXEpoll.h>

#include <netinet/in.h> // INADDR_ANY等宏的数据
#include <fcntl.h>      // Linux I/O
#include <sys/socket.h> // 套接字
#include <cstring>      // 错误码 to 错误码对应原因
#include <cerrno>       // 错误码
#include <unistd.h>     // 提供了 close

#include <HXprint/HXprint.h>
#include <HXJson/HXJson.h>

// 如果这个function<...>!=nullptr, 则调用
#define ATTEMPT_TO_CALL(funName, ...) \
if (funName) \
    funName(__VA_ARGS__)

// 解析缓冲区大小
#define MAX_BUFFER_SIZE 4096

namespace HXHttp {

HXEpoll::HXEpoll(int port, int maxQueue, int maxConnect) 
    : _maxConnect(maxConnect)
    , _running(false)
    , _tasks()
    , _queueMutex()
    , _condition()
    , _newConnectCallbackFunc(nullptr)
    , _newMsgCallbackFunc(nullptr)
    , _newUserBreakCallbackFunc(nullptr)  {
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
        _ev.events = EPOLLIN | EPOLLET; // EPOLLET 模式
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

    _condition.notify_all();
    for (auto&& it : _threads)
        it.join();

    LOG_INFO("====== Epoll已关闭: True ======");
}

bool HXEpoll::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) { // 获取文件描述符标志
        return true;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) { // 设置文件状态标志为非阻塞
        return true;
    }
    return false;
}

void HXEpoll::workerThread() {
    while (_running) {
        int clientFd;
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _condition.wait(lock, [this] { return !_tasks.empty() || !_running; });

            if (!_running) {
                break;
            }

            clientFd = _tasks.front();
            _tasks.pop();
        }

        char buffer[MAX_BUFFER_SIZE] = {0};
        LOG_INFO("[%llu] 读取中... {", std::this_thread::get_id());
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) { // 断开连接
            if (bytesRead == 0 || !(errno == EWOULDBLOCK || errno == EAGAIN)) {
                ctlDel(clientFd);
                ::close(clientFd);
            }
            ATTEMPT_TO_CALL(_newUserBreakCallbackFunc, clientFd);
        } else { // 处理收到的数据
            if (_newMsgCallbackFunc && _newMsgCallbackFunc(clientFd, buffer, sizeof(buffer))) {
                ctlDel(clientFd);
                ::close(clientFd);
            }
        }
        LOG_INFO("} // [%llu] 读取完毕并输出", std::this_thread::get_id());
    }
}

void HXEpoll::run(int timeOut /*= -1*/, std::function<bool()> conditional /*= nullptr*/) {
    _running = true;

    for (std::size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
        _threads.emplace_back(&HXEpoll::workerThread, this);
    }

    while (_running) {
        if (conditional)
            _running = conditional();

        int nfds = wait(timeOut);
        if (nfds == -1) {
            if (errno == EINTR) { // 屏蔽进程捕获信号而中断的错误
                LOG_WARNING("捕获到退出信号!");
                if (conditional)
                    _running = conditional();
                continue;
            }
            LOG_ERROR("wait Error: %s (errno: %d)", strerror(errno), errno);
            doError();
        }

        for (std::size_t i = 0; i < nfds; ++i) {
            int tmpFd = _events[i].data.fd;
            if (tmpFd == _serverFd) { // 新连接
                int newClientFd = ::accept(_serverFd, (struct sockaddr *)NULL, NULL);
                if (newClientFd < 0) {
                    LOG_ERROR("连接新客户端出错! %s (errno: %d)", strerror(errno), errno);
                } else if (setNonBlocking(newClientFd)) {
                    LOG_ERROR("设置fd为非阻塞时出错! %s (errno: %d)", strerror(errno), errno);
                    ::close(newClientFd);
                } else if (ctlAdd(newClientFd) < 0) {
                    LOG_ERROR("添加时候出现错误! %s (errno: %d)", strerror(errno), errno);
                    ::close(newClientFd);
                } else {
                    LOG_INFO("新的客户机连接! id = %d", newClientFd);
                    ATTEMPT_TO_CALL(_newConnectCallbackFunc, newClientFd); // 回调
                }
            } else { // 处理新来的信息
                LOG_WARNING("新信息=======================================");
                std::unique_lock<std::mutex> lock(_queueMutex);
                _tasks.push(tmpFd);
                _condition.notify_one();
            }
        }
        LOG_WARNING("for out =======================================");
    }
    LOG_INFO("run out!");
}

} // namespace HXHttp
