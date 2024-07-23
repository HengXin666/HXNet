#include <HXHttp/HXServer.h>

#include <fcntl.h>

namespace HXHttp {

HXServer::asyncFile HXServer::asyncFile::asyncWrap(int fd) {
    int flags = CHECK_CALL(::fcntl, fd, F_GETFL);
    flags |= O_NONBLOCK;
    CHECK_CALL(::fcntl, fd, F_SETFL, flags);

    struct ::epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = nullptr; // fd 对应 回调函数 (没有)
    CHECK_CALL(::epoll_ctl, HXServer::Epoll::get()._epfd, EPOLL_CTL_ADD, fd, &event);

    return asyncFile{fd};
}

void HXServer::asyncFile::asyncAccept(
    HXAddressResolver::address addr, 
    HXCallback<HXErrorHandlingTools::Expected<int>> cd
    ) {
    auto ret = HXErrorHandlingTools::convertError<int>(::accept(_fd, &addr._addr, &addr._addrlen));

    if (!ret.isError(EAGAIN)) { // 不是EAGAIN错误
        cd(ret);
    }

    // 如果是 EAGAIN, 那么就让操作系统通知我吧
    // 到时候调用这个回调
    HXCallback<> resume = [this, &addr, cd = std::move(cd)]() mutable {
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

void HXServer::Acceptor::start(const std::string& name, const std::string& port) {
    HXAddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    int listenfd = entry.createSocketAndBind();
    _serverFd = HXServer::asyncFile::asyncWrap(listenfd);
    return accept();
}

void HXServer::Acceptor::accept() {
    return _serverFd.asyncAccept(
        _addr, [self = shared_from_this()] (HXErrorHandlingTools::Expected<int> ret) {
        auto fd = ret.expect("accept");

        // 开始读取
        return self->accept(); // 继续回调(如果没有就挂起, 就返回了)
    });
}

} // namespace HXHttp
