#include <unistd.h>

#include <HXWeb/HXApiHelper.h>
#include <HXSTL/utils/FileUtils.h>
#include <HXSTL/coroutine/loop/AsyncLoop.h>
#include <HXWeb/server/Acceptor.h>
#include <HXJson/HXJson.h>
#include <chrono>

using namespace std::chrono;

/**
 * @brief 实现一个轮询的聊天室 By Http服务器
 * 
 * 分析一下: 客户端会定时请求, 参数为当前消息的数量
 * 
 * 服务端如果 [客户端当前消息的数量 >= 服务端消息池.size()], 
 *      则会返回null
 * 服务端如果 [客户端当前消息的数量 < 服务端消息池.size()], 
 *      则会返回 消息: index: 服务端消息池.begin() + 客户端当前消息的数量 ~ .end()
 * 
 * 困难点, json解析与封装 (得实现json的反射先...), 那可不行! 反正就那几个, 直接手动得了..
 */

struct Message {
    std::string _user;
    std::string _content;

    explicit Message(
        std::string user, 
        std::string content
    ) : _user(user)
      , _content(content)
    {}

    static std::string toJson(
        std::vector<Message>::iterator beginIt, 
        std::vector<Message>::iterator endIt
    ) {
        std::string res = "[";
        if (beginIt != endIt) {
            for (; beginIt != endIt; ++beginIt)
                res += "{\"user\":\""+ beginIt->_user + "\",\"content\":\"" + beginIt->_content  +"\"},";
            res.pop_back();
        }
        res += "]";
        return res;
    }
};

std::vector<Message> messageArr;

class ChatController {
    ENDPOINT_BEGIN(API_GET, "/", root) {
        RESPONSE_DATA(
            200,
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("index.html"),
            "text/html", "UTF-8"
        );
        co_return;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/favicon.ico", faviconIco) {
        RESPONSE_DATA(
            200, 
            co_await HX::STL::utils::FileUtils::asyncGetFileContent("favicon.ico"),
            "image/x-icon"
        );
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/files/**", files) {
        PARSE_MULTI_LEVEL_PARAM(path);
        RESPONSE_STATUS(200).setContentType("text/html", "UTF-8")
                            .setBodyData("<h1> files URL is " + path + "</h1>");
        co_return;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_GET, "/home/{id}/{name}", getIdAndNameByHome) {
        START_PARSE_PATH_PARAMS; // 开始解析请求路径参数
        PARSE_PARAM(0, u_int32_t, id);
        PARSE_PARAM(1, std::string, name);

        // 解析查询参数为键值对; ?awa=xxx 这种
        GET_PARSE_QUERY_PARAMETERS(queryMap);

        if (queryMap.count("loli")) // 如果解析到 ?loli=xxx
            std::cout << queryMap["loli"] << '\n'; // xxx 的值

        RESPONSE_DATA(
            200, 
            "<h1> Home id 是 " + std::to_string(*id) + ", 而名字是 " + *name + "</h1><h2> 来自 URL: " + req.getRequesPath() + " 的解析</h2>",
            "text/html", "UTF-8"
        );
        co_return;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_POST, "/send", send) { // 客户端发送消息过来
        auto body = req.getRequesBody();
        auto jsonPair = HX::Json::parse(body);
        if (jsonPair.second) {
            messageArr.emplace_back(
                jsonPair.first.get<HX::Json::JsonDict>()["user"].get<std::string>(),
                jsonPair.first.get<HX::Json::JsonDict>()["content"].get<std::string>()
            );
            printf("%s\n", Message::toJson(messageArr.begin(), messageArr.end()).c_str());
        } else {
            printf("解析客户端出错\n");
        }
        
        RESPONSE_STATUS(200).setContentType("text/plain", "UTF-8");
        co_return;
    } ENDPOINT_END;

    ENDPOINT_BEGIN(API_POST, "/recv", recv) { // 发送内容给客户端
        using namespace std::chrono_literals;

        auto body = req.getRequesBody();
        printf("recv (%s)\n", body.c_str());
        auto jsonPair = HX::Json::parse(body);

        if (jsonPair.second) {
            int len = jsonPair.first.get<HX::Json::JsonDict>()["first"].get<int>();
            printf("内容是: %d\n", len);
            if (len < (int)messageArr.size()) {
                printf("立马回复, 是新消息~\n");
                RESPONSE_DATA(
                    200,
                    Message::toJson(messageArr.begin() + len, messageArr.end()),
                    "text/plain", "UTF-8"
                );
                co_return;
            }
            else {
                printf("等我3秒~\n");
                co_await HX::STL::coroutine::loop::TimerLoop::sleep_for(3s);
                std::vector<Message> submessages;
                printf("3秒之期已到, 马上回复~\n");
                RESPONSE_DATA(
                    200,
                    Message::toJson(messageArr.begin() + len, messageArr.end()),
                    "text/plain", "UTF-8"
                );
                co_return;
            }
        } else {
            printf("啥也没有...\n");
        }
        co_return;
    } ENDPOINT_END;

public:

};

HX::STL::coroutine::awaiter::Task<> startChatServer() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    messageArr.emplace_back("系统", "欢迎来到聊天室!");
    ROUTER_BIND(ChatController);
    try {
        auto ptr = HX::web::server::Acceptor::make();
        co_await ptr->start("0.0.0.0", "28205");
    } catch(const std::system_error &e) {
        std::cerr << e.what() << '\n';
    }
    co_return;
}

#include <chrono>

using namespace std::chrono;

HX::STL::coroutine::awaiter::TimerTask C() {
    std::cout << "\t\tC start\n";
    co_await HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().sleep_for(3s); // Simulate an asynchronous operation
    std::cout << "\t\tC continues\n";
}

HX::STL::coroutine::awaiter::TaskSuspend<
    void,
    HX::STL::coroutine::awaiter::Promise<void>,
    HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::awaiter::Promise<void>>
> B() {
    std::cout << "\tB start\n";
    // auto task = C(); // 需要保证 task 未被销毁!
    HX::STL::coroutine::loop::AsyncLoop::getLoop().getTimerLoop().addTimer(
        std::chrono::system_clock::now(),
        nullptr,
        std::make_shared<HX::STL::coroutine::awaiter::TimerTask>(C())
    );
    std::cout << "\tB end\n";
    co_return;
}

HX::STL::coroutine::awaiter::Task<
    void,
    HX::STL::coroutine::awaiter::Promise<void>,
    HX::STL::coroutine::awaiter::ExitAwaiter<void, HX::STL::coroutine::awaiter::Promise<void>>
> A() {
    std::cout << "A start\n";
    co_await B();
    std::cout << "A end\n";
}

#include <liburing.h>

int _main() {
    io_uring ring;
    /**
     * @brief 初始化长度为 32 (一般是2的幂) 的环型队列
     * 给 ring,
     * tag 是 0, 即没有标志
     */
    io_uring_queue_init(32, &ring, 0);
    // 获取任务队列
    io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    char buf[16];
    /**
     * @brief 向任务队列添加异步读任务
     * sqe 需要添加任务的任务队列指针
     * STDIN_FILENO (输入流) fd (启动程序系统自动打开的文件)
     * buf 存放读取结果的数组
     * 16 一般是需要读取的长度(buf.size())
     * 0 文件偏移量
     */
    // sqe->user_data = (u_int32_t)&A; 可以存放用户数据
    io_uring_prep_read(sqe, STDIN_FILENO, buf, 16, 0);

    // 提交任务队列给内核 (为什么不是sqe, 因为sqe是从ring中get出来的, 故其本身就包含了sqe)
    io_uring_submit(&ring);

    io_uring_cqe* cqe = nullptr;

    // 阻塞等待内核, 返回是错误码; cqe是完成队列, 为传出参数
    io_uring_wait_cqe(&ring, &cqe);
    // io_uring_wait_cqe_timeout() 有带超时时间的

    // 在销毁之前, 我们需要取出数据
    int res = cqe->res; // 这个就是对应任务的返回值(read的返回值, 即读取的字节数)
    // cqe->user_data 这个是 u_int64_t 到时候就可以放置指针, 从而回复协程 

    // 销毁完成队列, 不然会一直在里面滞留(占用空间)
    io_uring_cqe_seen(&ring, cqe);

    LOG_INFO("输入 [%s], 长度是 %d", buf, res);

    // C语言魅力时刻 (因为他们没有析构函数)
    io_uring_queue_exit(&ring);
    return 0;
}

int main() {
    chdir("../static");
    HX::STL::coroutine::awaiter::runTask(
        HX::STL::coroutine::loop::AsyncLoop::getLoop(), 
        startChatServer()
    );
    return 0;
}