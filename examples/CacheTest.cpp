#ifdef CACHE_TEST_MAIN
#include <memory>

#include <HXSTL/cache/LRUCache.hpp>
#include <HXSTL/cache/LFUCache.hpp>
#include <HXprint/print.h>

struct Node {
    int abc;
    int cba;
    mutable std::shared_ptr<int> _ptr;
 
    Node(int a, int b)
        : abc(a)
        , cba(b)
    {
        HX::print::print("new: ", a, ", ", b);
    }

    Node(const Node& node)
        : Node(node.abc, node.cba)
    {
        HX::print::print("copy!! (", abc, ", ", cba, ")");
    }

    Node& operator=(const Node& other) {
        if (this != &other) {
            // 拷贝成员变量
            this->abc = std::move(other.abc);
            this->cba = std::move(other.cba);
        }
        HX::print::print("awa");
        return *this;
    }

    Node& operator=(Node&&) = delete;
    // Node& operator=(const Node&) = delete;

    void print() const {
        HX::print::print(abc, cba);
    }

    ~Node() {
        HX::print::print(">>>> End: ", abc, ", ", cba);
    }
};

HX::STL::cache::LRUCache<int, Node> testGetURLCache() {
    HX::STL::cache::LRUCache<int, Node> tmp(1);
    tmp.emplace(2233, 114514, 0721);
    return tmp;
}

auto wdf() {
    HX::STL::cache::LRUCache<int, Node> lruCache = testGetURLCache();
    lruCache.emplace(1, 1, 2);
    auto&& item = lruCache.get(1);
    item._ptr = std::make_shared<int>();;
    HX::print::print(item);
    lruCache.emplace(1, 4, 5);
    lruCache.insert(2, {8, 9});
    HX::print::print("size = ", lruCache.size());
    // HX::print::print(item);
    // std::list<Node> sb;
    // sb.erase(sb.begin());
    // sb.emplace(sb.end(), 22, 33);

    // std::unordered_map<int, Node> dsb;
    // dsb.emplace_hint(dsb.end(), 22, Node{22, 33});
    // return item;
}

auto lfuTest() {
    HX::STL::cache::LFUCache<int, std::string> cache(1);
    cache.emplace(2, "nb 666");
    HX::print::print(cache.get(2));
    cache.emplace(2, "nb 777");
    HX::print::print(cache.get(2));
    cache.emplace(2233, "nb Heng_Xin!!");
    HX::print::print(cache.get(2233));
}

class TestBase {
public:
    // virtual 
    void test() 
    // = 0;
    { printf("test base\n"); }
};

class TestImpl : public TestBase {
public:
    void test() {
        printf("test impl\n");
    }
};

int main() {
    // TestBase* tb = new TestImpl;
    // auto&& op = (void)wdf(), 1;
    wdf();
    lfuTest();
    // HX::print::print(op);

    HX::STL::cache::ThreadSafeLRUCache<std::string, std::string> ts(3);
    ts.emplace("abc", "char * -> string");
    HX::print::print(ts.get("abc"));

    // HX::STL::cache::ThreadSafeLRUCache<int, std::string> tts(std::move(ts));
    // HX::print::print(tts.get(1));

    HX::print::print("ts中是否存在 [abc]: ", ts.contains("abc"));
    
    return 0;
}

#endif // CACHE_TEST_MAIN