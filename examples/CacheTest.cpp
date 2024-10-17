#ifdef CACHE_TEST_MAIN
#include <memory>

#include <HXSTL/container/LRUCache.hpp>
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

    // Node(const Node& node)
    //     : Node(node.abc, node.cba)
    // {
    //     HX::print::print("copy!! (", abc, ", ", cba, ")");
    // }

    // Node& operator=(const Node& other) {
    //     if (this != &other) {
    //         // 拷贝成员变量
    //         this->abc = std::move(other.abc);
    //         this->cba = std::move(other.cba);
    //     }
    //     HX::print::print("awa");
    //     return *this;
    // }


    Node& operator=(Node&&) = delete;
    // Node& operator=(const Node&) = delete;

    void print() const {
        HX::print::print(abc, cba);
    }

    ~Node() {
        HX::print::print(">>>> End: ", abc, ", ", cba);
    }
};

HX::STL::container::LRUCache<int, Node> testGetURLCache() {
    HX::STL::container::LRUCache<int, Node> tmp(1);
    return tmp;
}

auto wdf() {
    HX::STL::container::LRUCache<int, Node> lruCache = testGetURLCache();
    lruCache.emplace(1, 1, 2);
    auto&& item = lruCache.get(1);
    item._ptr = std::make_shared<int>();;
    HX::print::print(item);
    lruCache.emplace(1, 4, 5);
    lruCache.emplace(1, 8, 9);
    HX::print::print("size = ", lruCache.size());
    HX::print::print(item);

    // std::list<Node> sb;
    // sb.emplace(sb.end(), 22, 33);

    // std::unordered_map<int, Node> dsb;
    // dsb.emplace_hint(dsb.end(), 22, Node{22, 33});
    // return item;
}

int main() {
    // auto&& op = (void)wdf(), 1;
    wdf();
    // HX::print::print(op);
    return 0;
}

#endif // CACHE_TEST_MAIN