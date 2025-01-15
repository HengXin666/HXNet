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
        HX::print::println("new: ", a, ", ", b);
    }

    Node(const Node& node)
        : Node(node.abc, node.cba)
    {
        HX::print::println("copy!! (", abc, ", ", cba, ")");
    }

    Node& operator=(const Node& other) {
        if (this != &other) {
            // 拷贝成员变量
            this->abc = std::move(other.abc);
            this->cba = std::move(other.cba);
        }
        HX::print::println("operator=(const Node&)");
        return *this;
    }

    Node& operator=(Node&&) = delete;
    // Node& operator=(const Node&) = delete;

    void print() const {
        HX::print::println(abc, cba);
    }

    ~Node() {
        HX::print::println(">>>> End: ", abc, ", ", cba);
    }
};

void LRUCacheTest() {
    HX::STL::cache::LRUCache<int, Node> cache = []() -> HX::STL::cache::LRUCache<int, Node> {
        HX::STL::cache::LRUCache<int, Node> tmp(1);
        tmp.emplace(2233, 114514, 0721);
        return tmp; // 可以作为返回值 (编译器自动实现 && 转化)
    }();
    cache.emplace(1, 1, 2);
    auto&& item = cache.get(1);
    item._ptr = std::make_shared<int>(); // 使用智能指针
    HX::print::print(item);
    cache.emplace(1, 4, 5); // 原地构造会释放指针
    cache.insert(2, {8, 9});
    HX::print::print("size = ", cache.size());

    // 线程安全的LRUCache
    HX::STL::cache::ThreadSafeLRUCache<std::string, std::string> ts(3);
    ts.emplace("abc", "char * -> string");
    HX::print::println(ts.get("abc"));

    // 透明查找的示例: 比如允许通过 const char* 查找 std::string 的键
    // 这使得用户可以在调用 contains 时, 不必显式地转换键的类型
    HX::print::println("ts中是否存在 [abc]: ", ts.contains("abc"));

    // HX::STL::cache::ThreadSafeLRUCache<int, Node> tts(std::move(cache));
    // tts.get(2);
}

void LFUCacheTest() {
    HX::STL::cache::LFUCache<int, std::string> cache(1);
    cache.insert(2, "nb 666");
    HX::print::println(cache.get(2));
    cache.emplace(2, "nb 777");
    HX::print::println(cache.get(2));
    cache.emplace(2233, "nb Heng_Xin!!");
    HX::print::println(cache.get(2233));

    // 支持从普通线程不安全的LFUCache移动构造出线程安全的ThreadSafeLFUCache
    HX::STL::cache::ThreadSafeLFUCache<int, std::string> ts(std::move(cache));
    HX::print::println("size = ", ts.size());
    HX::print::println(ts.get(2233));
    ts.emplace(114514, "oh my god!");
    HX::print::println(ts.get(114514));
    HX::print::println(ts.get(114514));
    ts.clear();
    HX::print::println("ts.clear() => size = ", ts.size());
}

int main() {
    LRUCacheTest();
    HX::print::println("\n===============\n");
    LFUCacheTest();
    return 0;
}