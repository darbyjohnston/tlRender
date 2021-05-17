// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <map>
#include <vector>

namespace tlr
{
    //! Memory.
    namespace memory
    {
        //! LRU (least recently used) cache.
        template<typename T, typename U>
        class Cache
        {
        public:
            //! \name Size
            ///@{

            std::size_t getMax() const;
            std::size_t getSize() const;
            float getPercentageUsed() const;

            void setMax(std::size_t);

            ///@}

            //! \name Contents
            ///@{

            bool contains(const T& key) const;
            bool get(const T& key, U& value) const;

            void add(const T& key, const U& value);
            void remove(const T& key);
            void clear();

            std::vector<T> getKeys() const;
            std::vector<U> getValues() const;

            ///@}

        private:
            void _maxUpdate();

            std::size_t _max = 10000;
            std::map<T, U> _map;
            mutable std::map<T, int64_t> _counts;
            mutable int64_t _counter = 0;
        };
    }
}

#include <tlrCore/CacheInline.h>
