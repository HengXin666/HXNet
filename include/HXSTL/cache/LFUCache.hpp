#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2024-10-18 16:04:49
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
#ifndef _HX_LFU_CACHE_H_
#define _HX_LFU_CACHE_H_

#include <list>
#include <unordered_map>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>

namespace HX { namespace STL { namespace cache {

/**
 * @brief 一个满足LFU(最不经常使用)缓存约束的数据结构
 * @tparam K 键类型
 * @tparam V 值类型
 * @warning 这个是一个线程不安全的数据结构
 * @tparam _CntType 计数类型: 用于记录访问次数
 */
template <class K, class V, class _CntType = std::size_t>
class LFUCache {
public:
    /// @brief 计数类型: 用于记录访问次数
    using CntType = _CntType;
    using KeyValuePairType = std::pair<K, std::pair<V, CntType>>;
    using ListIterator = std::list<KeyValuePairType>::iterator;
private:
    /**
     * @brief 更新迭代器`it`的数据为经常使用的数据
     * @param it 需要更新的迭代器
     * @return const V& 迭代器的数据, 即`Value`
     */
    const V& _updateToFrequent(std::unordered_map<K, ListIterator>::iterator& it) const {
        auto& cnt = it->second->second.second;
        auto oldListIt = _freqMap.find(cnt);
        auto& newList = _freqMap[cnt + 1];
        newList.splice(newList.begin(), oldListIt->second, it->second);
        if (oldListIt->second.empty()) {
            _freqMap.erase(oldListIt);
            if (cnt == _minCnt)
                ++_minCnt;
        }
        ++cnt;
        return newList.begin()->second.first;
    }

    /**
     * @brief 尝试删除最久最不常用的元素
     */
    void _removeLFUItem() {
        // 删除最久最不经常使用的
        auto lfuListIt = _freqMap.find(_minCnt);
        _keyMap.erase(lfuListIt->second.rbegin()->first);
        lfuListIt->second.pop_back();
        if (lfuListIt->second.empty()) {
            _freqMap.erase(lfuListIt);
        }
    }
public:
    explicit LFUCache(std::size_t capacity) noexcept
        : _keyMap()
        , _freqMap()
        , _capacity(capacity)
    {}

    LFUCache(LFUCache&& that) noexcept
        : _keyMap(std::move(that._keyMap))
        , _freqMap(std::move(that._freqMap))
        , _capacity(that._capacity)
        , _minCnt(that._minCnt)
    {
        that._capacity = 0;
        that._minCnt = 1;
    }

    LFUCache(LFUCache&) = delete;
    LFUCache& operator=(LFUCache&) = delete;

    LFUCache& operator=(LFUCache&& that) noexcept {
        if (this != &that) [[likely]] {  // 防止自赋值
            _keyMap = std::move(that._keyMap);
            _freqMap = std::move(that._freqMap);
            _capacity = that._capacity;
            _minCnt = that._minCnt;
            that._capacity = 0;
            that._minCnt = 1;
        }
        return *this;
    }

    /**
     * @brief 获取键`key`对应的值, 如果不存在则`抛出异常`
     * @param key 
     * @return const V& 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    const V& get(const K& key) const {
        if (auto it = _keyMap.find(key); it != _keyMap.end()) {
            return _updateToFrequent(it);
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
        return _keyMap.find(key) != _keyMap.end();
    }

#if __cplusplus >= 201402L
    /**
     * @brief 获取键`key`对应的值 (透明查找), 如果不存在则`抛出异常`
     * @tparam X 需要支持`Compare::is_transparent`
     * @param key 
     * @return const V& 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    template <class X>
    const V& get(const X& key) const {
        if (auto it = _keyMap.find(key); it != _keyMap.end()) {
            return _updateToFrequent(it);
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
        return _keyMap.find(x) != _keyMap.end();
    }
#endif // __cplusplus >= 201402L

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, const V& value) {
        auto it = _keyMap.find(key);
        if (it != _keyMap.end()) {
            _updateToFrequent(it);
            it->second->second.first = value;
        } else {
            if (_keyMap.size() == _capacity) {
                _removeLFUItem();
            }
            // 插入
            auto& newList = _freqMap[_minCnt = 1];
            _keyMap[key] = newList.emplace(
                newList.begin(),
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(value, 1)
            );
        }
    }

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, V&& value) {
        if (auto it = _keyMap.find(key); it != _keyMap.end()) {
            _updateToFrequent(it);
            // 原地构造
            auto& value = it->second->second.first;
            value.~V();
            new (std::addressof(value)) V(std::move(value));
        } else {
            if (_keyMap.size() == _capacity) {
                _removeLFUItem();
            }
            // 插入
            auto& newList = _freqMap[_minCnt = 1];
            _keyMap[key] = newList.emplace(
                newList.begin(),
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(std::move(value), 1)
            );
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
        if (auto it = _keyMap.find(key); it != _keyMap.end()) {
            _updateToFrequent(it);
            // 原地构造
            auto& value = it->second->second.first;
            value.~V(); // 显式调用析构函数, 以析构如智能指针成员等
            // 使用 std::addressof 可以防止 & 运算符被重载
            new (std::addressof(value)) V(std::forward<Args>(args)...); // 原地构造
        } else {
            if (_keyMap.size() == _capacity) {
                _removeLFUItem();
            }
            // 插入
            auto& newList = _freqMap[_minCnt = 1];
            _keyMap[key] = newList.emplace(
                newList.begin(),
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(std::forward<Args>(args)..., 1)
            );
        }
    }

    /**
     * @brief 获取当前LFU中缓存的数据个数
     * @return std::size_t 
     */
    std::size_t size() const noexcept {
        return _keyMap.size();
    }

    /**
     * @brief 判断LFU是否为空
     * @return true 为空
     * @return false 非空
     */
    bool empty() const noexcept {
        return _keyMap.empty();
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
        _keyMap.clear();
        _freqMap.clear();
        _minCnt = 1;
    }
protected:
    mutable std::unordered_map<K, ListIterator> _keyMap;
    mutable std::unordered_map<CntType, std::list<KeyValuePairType>> _freqMap;
    std::size_t _capacity;
    mutable CntType _minCnt;
};

/**
 * @brief 一个满足LRU(最近最少使用)缓存约束的线程安全数据结构
 * @tparam K 键类型
 * @tparam V 值类型
 */
template <class K, class V>
class ThreadSafeLFUCache : public LFUCache<K, V> {
public:
    explicit ThreadSafeLFUCache(std::size_t capacity) noexcept
        : LFUCache<K, V>(capacity)
        , _mtx()
    {}

    /**
     * @brief 从线程不安全的LFUCache进行移动构造
     * @param that LFUCache<K, V>
     */
    ThreadSafeLFUCache(LFUCache<K, V>&& that) noexcept
        : LFUCache<K, V>(std::move(that))
        , _mtx()
    {}

    /**
     * @brief 因为关联了锁, 锁的赋值运算符是被删除的, 因此不支持(赋值/移动)(构造/拷贝)
     */

    // 删除拷贝构造函数和拷贝赋值操作符
    ThreadSafeLFUCache(const ThreadSafeLFUCache&) = delete;
    ThreadSafeLFUCache& operator=(const ThreadSafeLFUCache&) = delete;

    // 删除移动构造函数和移动赋值操作符
    ThreadSafeLFUCache(ThreadSafeLFUCache&&) = delete;
    ThreadSafeLFUCache& operator=(ThreadSafeLFUCache&&) = delete;

    /**
     * @brief 获取键`key`对应的值, 如果不存在则`抛出异常`
     * @param key 
     * @return V 
     * @throw std::range_error(键: 不存在)
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点); 不然请老老实实拷贝吧
     */
    const V& get(const K& key) const {
        std::shared_lock _{_mtx};
        return LFUCache<K, V>::get(key);
    }

    /**
     * @brief 检查缓存中是否包含某个键
     * @param key 需要检查的键
     * @return true 存在
     * @return false 不存在
     */
    bool contains(const K& key) const {
        std::shared_lock _{_mtx};
        return LFUCache<K, V>::contains(key);
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
    const V& get(const K& key) const {
        std::shared_lock _{_mtx};
        return LFUCache<K, V>::get(key);
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
        return LFUCache<K, V>::contains(x);
    }
#endif // __cplusplus >= 201402L

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, const V& value) {
        std::unique_lock _{_mtx};
        LFUCache<K, V>::insert(key, value);
    }

    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, V&& value) {
        std::unique_lock _{_mtx};
        LFUCache<K, V>::insert(key, std::move(value));
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
        LFUCache<K, V>::emplace(key, std::forward<Args>(args)...);
    }

    /**
     * @brief 获取当前LUR中缓存的数据个数
     * @return std::size_t 
     */
    std::size_t size() const noexcept {
        std::shared_lock _{_mtx};
        return LFUCache<K, V>::_keyMap.size();
    }

    /**
     * @brief 判断LUR是否为空
     * @return true 为空
     * @return false 非空
     */
    bool empty() const noexcept {
        std::shared_lock _{_mtx};
        return LFUCache<K, V>::_keyMap.empty();
    }

    /**
     * @brief 返回缓存的最大容量
     * @return std::size_t 
     */
    std::size_t capacity() const noexcept {
        return LFUCache<K, V>::_capacity;
    }

    /**
     * @brief 清空缓存中的所有元素
     */
    void clear() noexcept {
        std::unique_lock _{_mtx};
        LFUCache<K, V>::clear();
    }
protected:
    /// @brief 读写锁
    mutable std::shared_mutex _mtx;
};

}}} // namespace HX::STL::cache

#endif // !_HX_LFU_CACHE_H_