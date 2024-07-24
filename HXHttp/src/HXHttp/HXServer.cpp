#include <HXHttp/HXServer.h>

#include <fcntl.h>

namespace HXHttp {

HXServer::AsyncFile HXServer::AsyncFile::asyncWrap(int fd) {
    int flags = CHECK_CALL(::fcntl, fd, F_GETFL);
    flags |= O_NONBLOCK;
    CHECK_CALL(::fcntl, fd, F_SETFL, flags);

    struct ::epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = nullptr; // fd 对应 回调函数 (没有)
    CHECK_CALL(::epoll_ctl, HXServer::Epoll::get()._epfd, EPOLL_CTL_ADD, fd, &event);

    return AsyncFile{fd};
}

void HXServer::AsyncFile::asyncAccept(
    HXAddressResolver::address addr, 
    HXSTL::HXCallback<HXErrorHandlingTools::Expected<int>> cd
    ) {
    auto ret = HXErrorHandlingTools::convertError<int>(::accept(_fd, &addr._addr, &addr._addrlen));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        cd(ret);
        return;
    }

    // 如果是 EAGAIN, 那么就让操作系统通知我吧
    // 到时候调用这个回调
    HXSTL::HXCallback<> resume = [this, &addr, cd = std::move(cd)]() mutable {
        return asyncAccept(addr, std::move(cd));
    };

    // 让操作系统通知我
    struct ::epoll_event event;
    /**
     * EPOLLIN: 当有数据可读时，`epoll` 会触发事件
     * EPOLLET: 设置边沿触发模式
     * EPOLLONESHOT: 表示事件只会触发一次。当一个文件描述符上的一个事件触发并被处理后，这个文件描述符会从 `epoll` 监控队列中移除。
     */
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    event.data.ptr = resume.leakAddress(); // fd 对应 回调函数 (没有)
    CHECK_CALL(::epoll_ctl, HXServer::Epoll::get()._epfd, EPOLL_CTL_MOD, _fd, &event);
}

void HXServer::AsyncFile::asyncRead(
    HXSTL::HXBytesBuffer& buf,
    std::size_t count,
    HXSTL::HXCallback<HXErrorHandlingTools::Expected<size_t>> cd
    ) {
    auto ret = HXErrorHandlingTools::convertError<size_t>(::recv(_fd, buf.data(), count, 0));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        cd(ret);
        return;
    }

    // 如果是 EAGAIN, 那么就让操作系统通知我吧
    // 到时候调用这个回调
    HXSTL::HXCallback<> resume = [this, &buf, count, cd = std::move(cd)]() mutable {
        return asyncRead(buf, count, std::move(cd));
    };

    // 让操作系统通知我
    struct ::epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT; // 只通知一次
    event.data.ptr = resume.leakAddress(); // fd 对应 回调函数 (没有)
    CHECK_CALL(::epoll_ctl, HXServer::Epoll::get()._epfd, EPOLL_CTL_MOD, _fd, &event);
}

void HXServer::ConnectionHandler::start(int fd) {
    _fd = HXServer::AsyncFile::asyncWrap(fd);
    return read();
}

void HXServer::ConnectionHandler::read() {
    return _fd.asyncRead(_buf, _buf.size(), [self = shared_from_this()] (HXErrorHandlingTools::Expected<size_t> ret) {
        if (ret.error()) {
            return;
        }

        size_t n = ret.value(); // 读取到的字节数
        if (n == 0) {
            // 断开连接
            LOG_INFO("客户端已断开连接!");
            return;
        }
        
        // 进行解析

    });
}

void HXServer::Acceptor::start(const std::string& name, const std::string& port) {
    HXAddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    LOG_INFO("====== HXServer start: \033[33m\033]8;;http://%s:%s/\033\\http://%s:%s/\033]8;;\033\\\033[0m\033[1;32m ======", 
        name.c_str(),
        port.c_str(),
        name.c_str(),
        port.c_str()
    );
    int listenfd = entry.createSocketAndBind();
    _serverFd = HXServer::AsyncFile::asyncWrap(listenfd);
    return accept();
}

void HXServer::Acceptor::accept() {
    return _serverFd.asyncAccept(_addr, [self = shared_from_this()] (HXErrorHandlingTools::Expected<int> ret) {
        int fd = ret.expect("accept");
        HXServer::ConnectionHandler::make()->start(fd); // 开始读取
        return self->accept(); // 继续回调(如果没有就挂起, 就返回了)
    });
}

} // namespace HXHttp
