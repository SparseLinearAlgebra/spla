/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#ifndef SPLA_STORAGE_MANAGER_HPP
#define SPLA_STORAGE_MANAGER_HPP

#include <spla/config.hpp>

#include <core/tdecoration.hpp>

#include <algorithm>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class StorageManager
     * @brief General format converter for vector or matrix decoration storage
     *
     * @tparam T Type of elements stored
     * @tparam F Format of stored data
     * @tparam capacity Capacity if storage
     */
    template<typename T, typename F, int capacity>
    class StorageManager final {
    public:
        typedef TDecorationStorage<T, F, capacity>    Storage;
        typedef std::function<void(Storage& storage)> Function;

        explicit StorageManager();

        void register_constructor(F format, Function function);
        void register_validator(F format, Function function);
        void register_discard(F format, Function function);
        void register_validator_discard(F format, Function function);
        void register_converter(F from, F to, Function function);

        void validate_ctor(F format, Storage& storage);
        void validate_rw(F format, Storage& storage);
        void validate_rwd(F format, Storage& storage);
        void validate_wd(F format, Storage& storage);

    private:
        std::vector<std::vector<std::pair<int, int>>> m_convert_rules;
        std::vector<Function>                         m_constructors;
        std::vector<Function>                         m_validators;
        std::vector<Function>                         m_discards;
        std::vector<Function>                         m_converters;
    };

    template<typename T, typename F, int capacity>
    StorageManager<T, F, capacity>::StorageManager() {
        m_convert_rules.resize(capacity);
        m_constructors.resize(capacity);
        m_validators.resize(capacity);
        m_discards.resize(capacity);
    }

    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::register_constructor(F format, StorageManager::Function function) {
        const int i       = static_cast<int>(format);
        m_constructors[i] = std::move(function);
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::register_validator(F format, StorageManager::Function function) {
        const int i     = static_cast<int>(format);
        m_validators[i] = std::move(function);
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::register_discard(F format, StorageManager::Function function) {
        const int i   = static_cast<int>(format);
        m_discards[i] = std::move(function);
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::register_validator_discard(F format, StorageManager::Function function) {
        const int i   = static_cast<int>(format);
        m_discards[i] = m_validators[i] = std::move(function);
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::register_converter(F from, F to, StorageManager::Function function) {
        const int i  = static_cast<int>(from);
        const int j  = static_cast<int>(to);
        const int id = static_cast<int>(m_converters.size());
        m_convert_rules[i].push_back({j, id});
        m_converters.push_back(std::move(function));
    }

    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::validate_ctor(F format, Storage& storage) {
        const int i = static_cast<int>(format);
        if (!storage.get_ptr_i(i)) {
            m_constructors[i](storage);
        }
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::validate_rw(F format, Storage& storage) {
        if (storage.is_valid(format)) {
            return;
        }
        if (!storage.is_valid_any()) {
            const int i = static_cast<int>(format);
            if (!storage.get_ptr_i(i)) {
                if (!m_constructors[i]) {
                    LOG_MSG(Status::NotImplemented, "no such constructor for format " << i);
                    assert(false);
                    return;
                }
                m_constructors[i](storage);
            }
            if (m_validators[i]) {
                m_validators[i](storage);
            }
            storage.validate(format);
            return;
        }

        const int source   = -1;
        const int infinity = -2;
        const int target   = static_cast<int>(format);

        std::array<int, capacity> reached;
        std::queue<int>           queue;
        reached.fill(infinity);

        for (int i = 0; i < capacity; ++i) {
            if (storage.is_valid_i(i)) {
                reached[i] = source;
                queue.push(i);
            }
        }

        while (reached[target] == infinity) {
            int u = queue.front();
            queue.pop();

            for (const auto& target_format : m_convert_rules[u]) {
                if (reached[target_format.first] == infinity) {
                    reached[target_format.first] = u;
                    queue.push(target_format.first);
                }
            }
        }

        std::vector<std::pair<int, int>> path;
        int                              current = target;

        while (reached[current] != source) {
            path.push_back({reached[current], current});
            current = reached[current];
        }

        for (auto iter = path.rbegin(); iter != path.rend(); ++iter) {
            std::pair<int, int> from_to          = *iter;
            auto                search_predicate = [from_to](const std::pair<int, int>& rule) { return rule.first == from_to.second; };
            auto                rule             = std::find_if(m_convert_rules[from_to.first].begin(), m_convert_rules[from_to.first].end(), search_predicate);

            if (storage.get_ref_i(from_to.second).is_null()) {
                m_constructors[from_to.second](storage);
            }

            m_converters[rule->second](storage);
            storage.validate(static_cast<F>(from_to.second));
        }
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::validate_rwd(F format, Storage& storage) {
        validate_rw(format, storage);
        storage.invalidate();
        storage.validate(format);
    }
    template<typename T, typename F, int capacity>
    void StorageManager<T, F, capacity>::validate_wd(F format, Storage& storage) {
        const int i = static_cast<int>(format);
        if (!storage.get_ptr_i(i)) {
            m_constructors[i](storage);
        }
        if (m_discards[i]) {
            m_discards[i](storage);
        }
        storage.invalidate();
        storage.validate(format);
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_STORAGE_MANAGER_HPP
