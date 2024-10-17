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
#include <optional>

namespace HX { namespace STL { namespace container {

/**
 * @brief 一个满足LRU(最近最少使用)缓存约束的数据结构
 * @tparam K 键类型
 * @tparam V 值类型
 */
template <class K, class V>
class LRUCache {
    using ListType = std::pair<K, V>;
    using ListIterator = std::list<ListType>::iterator;

    std::list<ListType> _cacheList;
    std::unordered_map<K, ListIterator> _cacheMap;
    std::size_t _capacity;
public:
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
        if (this != &that) {  // 防止自赋值
            _cacheList = std::move(that._cacheList);
            _cacheMap = std::move(that._cacheMap);
            _capacity = that._capacity;
            that._capacity = 0;
        }
        return *this;
    }
    
    /**
     * @brief 获取键`key`对应的值, 如果不存在则返回`std::nullopt`
     * @param key 
     * @return std::optional<V> 
     */
    std::optional<V> get(const K& key) {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            _cacheList.splice(_cacheList.begin(), _cacheList, _cacheMap[key]);
            return _cacheList.begin()->second;
        }
        return std::nullopt;
    }
    
    /**
     * @brief 新增一个键值对, 如果有相同的则会覆盖旧的
     * @param key 
     * @param value 
     */
    void put(const K& key, const V& value) {
        if (auto it = _cacheMap.find(key); it != _cacheMap.end()) {
            // 修改
            _cacheList.splice(_cacheList.begin(), _cacheList, _cacheMap[key]);
            _cacheList.begin()->second = value;
        } else {
            // 添加
            if (_cacheMap.size() == _capacity) {
                // 满了, 需要删除最久没有使用的
                _cacheMap.erase(_cacheList.rbegin()->first);
                _cacheList.pop_back();
            }
            // _cacheMap[key] = _cacheList.insert(_cacheList.begin(), {key, value});
            _cacheMap.emplace(key, _cacheList.emplace(_cacheList.begin(), key, value));
        }
    }
};

}}} // namespace HX::STL::container

#endif // !_HX_LRU_CACHE_H_