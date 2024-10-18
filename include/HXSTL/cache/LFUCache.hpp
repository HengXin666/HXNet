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

namespace HX { namespace STL { namespace cache {

template <class K, class V, class _CntType = std::size_t>
class LFUCache {
public:
    using CntType = _CntType;
    using KeyValuePairType = std::pair<K, std::pair<V, CntType>>;
    using ListIterator = std::list<KeyValuePairType>::iterator;

    explicit LFUCache(std::size_t capacity) noexcept
        : _keyMap()
        , _freqMap()
        , _capacity(capacity)
    {}

    const V& get(const K& key) const {
        if (auto it = _keyMap.find(key); it != _keyMap.end()) {
            auto oldListIt = _freqMap.find(it->second->second.second);
            auto& newList = _freqMap[it->second->second.second + 1];
            newList.splice(newList.begin(), oldListIt->second, it->second);
            if (oldListIt->second.empty()) {
                _freqMap.erase(oldListIt);
                if (it->second->second.second == _minCnt)
                    ++_minCnt;
            }
            ++(it->second->second.second);
            return newList.begin()->second.first;
        }
        throw std::range_error("There is no such key in cache");
    }

    void insert(const K& key, const V& value) {
        if (auto it = _keyMap.find(key); it != _keyMap.end()) {
            auto oldListIt = _freqMap.find(it->second->second.second);
            auto& newList = _freqMap[it->second->second.second + 1];
            newList.splice(newList.begin(), oldListIt->second, it->second);
            if (oldListIt->second.empty()) {
                _freqMap.erase(oldListIt);
                if (it->second->second.second == _minCnt)
                    ++_minCnt;
            }
            ++(it->second->second.second);
            it->second->second.first = value;
        } else {
            if (_keyMap.size() == _capacity) {
                // 删除最久最不经常使用的
                auto lfuListIt = _freqMap.find(_minCnt);
                _keyMap.erase(lfuListIt->second.rbegin()->first);
                lfuListIt->second.pop_back();
                if (lfuListIt->second.empty()) {
                    _freqMap.erase(lfuListIt);
                }
            }
            // 插入
            auto& newList = _freqMap[_minCnt = 1];
            _keyMap[key] = newList.emplace(
                newList.begin(),
                std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(std::pair<V, CntType> {value, 1})
            );
        }
    }
protected:
    mutable std::unordered_map<K, ListIterator> _keyMap;
    mutable std::unordered_map<CntType, std::list<KeyValuePairType>> _freqMap;
    std::size_t _capacity;
    mutable CntType _minCnt;
};

}}} // namespace HX::STL::cache

#endif // !_HX_LFU_CACHE_H_