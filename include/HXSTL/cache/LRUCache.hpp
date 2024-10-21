#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-10-17 15:41:29
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_LRU_CACHE_H_
#define _HX_LRU_CACHE_H_

#include <list>
#include <unordered_map>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>

namespace HX { namespace STL { namespace cache {

/**
 * @brief 一个满足LRU(最近最少使用)缓存约束的数据结构
 * @tparam K 键类型
 * @tparam V 值类型
 * @warning 这个是一个线程不安全的数据结构
 */
template <class K, class V>
class LRUCache {
public:
    using KeyValuePairType = std::pair<K, V>;
    using ListIterator = std::list<KeyValuePairType>::iterator;

    explicit LRUCache(std::size_t capacity) noexcept
        : _cacheList()
        , _cacheMap()
        , _capacity(capacity)
    {}

    LRUCache(LRUCache&& that) noexcept
        : _cacheList(std::move(that._cacheList))
        , _cacheMap(std::move(that._cacheMap))
        , _capacity(that._capacity)
    {
        that._capacity = 0;
    }

    LRUCache(LRUCache&) = delete;
    LRUCache& operator=(LRUCache&) = delete;

    LRUCache& operator=(LRUCache&& that) noexcept {
        if (this != &that) [[likely]] {  // 防止自赋值
            _cacheList = std::move(that._cacheList);
            _cacheMap = std::move(that._cacheMap);
            _capacity = that._capacity;
            that._capacity = 0;
        }
        return *this;
    }
    
    /**
     * @brief 获取键`key`对应的值, 如果不存在则`抛出异常`
     * @param key 
     * @return V 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    const V& get(const K& key) const {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            return _cacheList.begin()->second;
        }
        throw std::range_error("There is no such key in cache");
    }

    /**
     * @brief 检查缓存中是否包含某个键
     * @param key 需要检查的键
     * @return true 存在
     * @return false 不存在
     */
    bool contains(const K& key) const {
        return _cacheMap.find(key) != _cacheMap.end();
    }

#if __cplusplus >= 201402L
    /**
     * @brief 获取键`key`对应的值 (透明查找), 如果不存在则`抛出异常`
     * @tparam X 需要支持`Compare::is_transparent`
     * @param key 
     * @return V 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    template <class X>
    const V& get(const X& key) const {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            return _cacheList.begin()->second;
        }
        throw std::range_error("There is no such key in cache");
    }

    /**
     * @brief 检查缓存中是否包含某个键 (透明比较)
     * @tparam X 需要支持`Compare::is_transparent`
     * @param key 需要检查的键
     * @return true 存在
     * @return false 不存在
     */
    template <class X>
    bool contains(const X& x) const {
        return _cacheMap.find(x) != _cacheMap.end();
    }
#endif // __cplusplus >= 201402L

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, const V& value) {
        auto it = _cacheMap.find(key);
        if (it != _cacheMap.end()) {
            // 修改
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            _cacheList.begin()->second = value;
        } else {
            // 添加
            if (_cacheMap.size() == _capacity) {
                // 满了, 需要删除最久没有使用的
                _cacheMap.erase(_cacheList.rbegin()->first);
                _cacheList.pop_back();
            }
            _cacheMap.emplace(key, _cacheList.emplace(_cacheList.begin(), key, value));
        }
    }

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, V&& value) {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            // 修改
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            // 原地构造
            auto& value = _cacheList.begin()->second;
            value.~V();
            new (std::addressof(value)) V(std::move(value));
        } else {
            // 添加
            if (_cacheMap.size() == _capacity) {
                // 满了, 需要删除最久没有使用的
                _cacheMap.erase(_cacheList.rbegin()->first);
                _cacheList.pop_back();
            }
            _cacheMap.emplace(key, _cacheList.emplace(_cacheList.begin(), key, std::move(value)));
        }
    }

    /**
     * @brief 插入一个键值对(以原地构造的方式), 如果有相同的则会覆盖旧的
     * @tparam Args 
     * @param key 
     * @param args 
     */
    template <class... Args>
    void emplace(const K& key, Args&&... args) {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            // 修改
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            // 原地构造
            auto& value = _cacheList.begin()->second;
            value.~V(); // 显式调用析构函数, 以析构如智能指针成员等
            // 使用 std::addressof 可以防止 & 运算符被重载
            new (std::addressof(value)) V(std::forward<Args>(args)...); // 原地构造
        } else {
            // 添加
            if (_cacheMap.size() == _capacity) {
                _cacheMap.erase(_cacheList.rbegin()->first);
                _cacheList.pop_back();
            }

            _cacheMap.emplace(key, _cacheList.emplace(
                _cacheList.begin(),
                std::piecewise_construct, 
                std::forward_as_tuple(key), 
                std::forward_as_tuple(std::forward<Args>(args)...)
            ));
        }
    }

    /**
     * @brief 获取当前LUR中缓存的数据个数
     * @return std::size_t 
     */
    std::size_t size() const noexcept {
        return _cacheMap.size();
    }

    /**
     * @brief 判断LUR是否为空
     * @return true 为空
     * @return false 非空
     */
    bool empty() const noexcept {
        return _cacheMap.empty();
    }

    /**
     * @brief 返回缓存的最大容量
     * @return std::size_t 
     */
    std::size_t capacity() const noexcept {
        return _capacity;
    }

    /**
     * @brief 清空缓存中的所有元素
     */
    void clear() noexcept {
        _cacheList.clear();
        _cacheMap.clear();
    }
protected:
    mutable std::list<KeyValuePairType> _cacheList;
    std::unordered_map<K, ListIterator> _cacheMap;
    std::size_t _capacity;
};

/**
 * @brief 一个满足LRU(最近最少使用)缓存约束的线程安全数据结构
 * @tparam K 键类型
 * @tparam V 值类型
 */
template <class K, class V>
class ThreadSafeLRUCache : public LRUCache<K, V> {
public:
    explicit ThreadSafeLRUCache(std::size_t capacity) noexcept
        : LRUCache<K, V>(capacity)
        , _mtx()
    {}

    /**
     * @brief 从线程不安全的LRUCache进行移动构造
     * @param that LRUCache<K, V>
     */
    ThreadSafeLRUCache(LRUCache<K, V>&& that) noexcept
        : LRUCache<K, V>(std::move(that))
        , _mtx()
    {}

    /**
     * @brief 因为关联了锁, 锁的赋值运算符是被删除的, 因此不支持(赋值/移动)(构造/拷贝)
     * 考虑到实际开发中LRU一般作为单例类成员, 并不太需要移动构造, 因此我认为这样设计是合理的
     * 你也不想我套一个智能指针, 
     * 或者在移动的语义下, 构造了一个新的锁成员吧...
     */

    // 删除拷贝构造函数和拷贝赋值操作符
    ThreadSafeLRUCache(const ThreadSafeLRUCache&) = delete;
    ThreadSafeLRUCache& operator=(const ThreadSafeLRUCache&) = delete;

    // 删除移动构造函数和移动赋值操作符
    ThreadSafeLRUCache(ThreadSafeLRUCache&&) = delete;
    ThreadSafeLRUCache& operator=(ThreadSafeLRUCache&&) = delete;

    /**
     * @brief 获取键`key`对应的值, 如果不存在则`抛出异常`
     * @param key 
     * @return V 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    const V& get(const K& key) const {
        std::shared_lock _{_mtx};
        return LRUCache<K, V>::get(key);
    }

    /**
     * @brief 检查缓存中是否包含某个键
     * @param key 需要检查的键
     * @return true 存在
     * @return false 不存在
     */
    bool contains(const K& key) const {
        std::shared_lock _{_mtx};
        return LRUCache<K, V>::contains(key);
    }

#if __cplusplus >= 201402L
    /**
     * @brief 获取键`key`对应的值 (透明查找), 如果不存在则`抛出异常`
     * @tparam X 需要支持`Compare::is_transparent`
     * @param key 
     * @return V 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    template <class X>
    const V& get(const X& key) const {
        std::shared_lock _{_mtx};
        return LRUCache<K, V>::get(key);
    }

    /**
     * @brief 检查缓存中是否包含某个键 (透明比较)
     * @tparam X 需要支持`Compare::is_transparent`
     * @param key 需要检查的键
     * @return true 存在
     * @return false 不存在
     */
    template <class X>
    bool contains(const X& x) const {
        std::shared_lock _{_mtx};
        return LRUCache<K, V>::contains(x);
    }
#endif // __cplusplus >= 201402L

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, const V& value) {
        std::unique_lock _{_mtx};
        LRUCache<K, V>::insert(key, value);
    }

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, V&& value) {
        std::unique_lock _{_mtx};
        LRUCache<K, V>::insert(key, std::move(value));
    }

    /**
     * @brief 插入一个键值对(以原地构造的方式), 如果有相同的则会覆盖旧的
     * @tparam Args 
     * @param key 
     * @param args 
     */
    template <class... Args>
    void emplace(const K& key, Args&&... args) {
        std::unique_lock _{_mtx};
        LRUCache<K, V>::emplace(key, std::forward<Args>(args)...);
    }

    /**
     * @brief 获取当前LUR中缓存的数据个数
     * @return std::size_t 
     */
    std::size_t size() const noexcept {
        std::shared_lock _{_mtx};
        return LRUCache<K, V>::_cacheMap.size();
    }

    /**
     * @brief 判断LUR是否为空
     * @return true 为空
     * @return false 非空
     */
    bool empty() const noexcept {
        std::shared_lock _{_mtx};
        return LRUCache<K, V>::_cacheMap.empty();
    }

    /**
     * @brief 返回缓存的最大容量
     * @return std::size_t 
     */
    std::size_t capacity() const noexcept {
        return LRUCache<K, V>::_capacity;
    }

    /**
     * @brief 清空缓存中的所有元素
     */
    void clear() noexcept {
        std::unique_lock _{_mtx};
        LRUCache<K, V>::clear();
    }
protected:
    /// @brief 读写锁
    mutable std::shared_mutex _mtx;
};

}}} // namespace HX::STL::cache

#endif // !_HX_LRU_CACHE_H_