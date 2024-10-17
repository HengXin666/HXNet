#ifdef CACHE_TEST_MAIN
#include <HXSTL/container/LRUCache.hpp>

HX::STL::container::LRUCache<int, int> testGetURLCache() {
    HX::STL::container::LRUCache<int, int> tmp(3);
    return tmp;
}

int main() {
    HX::STL::container::LRUCache<int, int> lruCache = testGetURLCache();
    lruCache.put(1, 2);
    lruCache.put(1, 3);
    
    return 0;
}

#endif // CACHE_TEST_MAIN