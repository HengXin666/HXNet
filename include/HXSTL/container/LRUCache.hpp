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

namespace HX { namespace STL { namespace container {

/**
 * @brief 一个满足LRU(最近最少使用)缓存约束的数据结构
 * @tparam K 键类型
 * @tparam V 值类型
 */
template <class K, class V>
class LRUCache {
public:
    using ListType = std::pair<K, V>;
    using ListIterator = std::list<ListType>::iterator;

    LRUCache(std::size_t capacity) noexcept
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
     * @warning 值得注意的是, 因为返回的是引用, 所以请尽早的使用, 防止悬挂引用! (缓存开大点)
     */
    const V& get(const K& key) const {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            return _cacheList.begin()->second;
        }
        throw std::range_error("There is no such key in cache");
    }
    
    /**
     * @brief 插入一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void insert(const K& key, const V& value) {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
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

    // TODO 是否有实现 emplace 的必要?
    template <class... Args>
    void emplace(const K& key, Args&&... args) {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            // 修改现有元素
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            // 原地构造
            auto& value = _cacheList.begin()->second;
            value.~V(); // 显式调用析构函数
            new (&value) V(std::forward<Args>(args)...); // 原地构造
        } else {
            if (_cacheMap.size() == _capacity) {
                _cacheMap.erase(_cacheList.rbegin()->first);
                _cacheList.pop_back();
            }
            _cacheMap.emplace(key, _cacheList.emplace(_cacheList.begin(), key, V(std::forward<Args>(args)...)));
        }
    }

    std::size_t size() const noexcept {
        return _cacheMap.size();
    }

    bool empty() const noexcept {
        return _cacheMap.empty();
    }

private:
    mutable std::list<ListType> _cacheList;
    std::unordered_map<K, ListIterator> _cacheMap;
    std::size_t _capacity;
};

}}} // namespace HX::STL::container

#endif // !_HX_LRU_CACHE_H_