// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/ReadCache.h>

#include <tlCore/LRUCache.h>

namespace tl
{
    namespace timeline
    {
        struct ReadCache::Private
        {
            memory::LRUCache<std::string, ReadCacheItem> cache;
        };

        void ReadCache::_init()
        {
            TLRENDER_P();
        }

        ReadCache::ReadCache() :
            _p(new Private)
        {}

        ReadCache::~ReadCache()
        {}

        std::shared_ptr<ReadCache> ReadCache::create()
        {
            auto out = std::shared_ptr<ReadCache>(new ReadCache);
            out->_init();
            return out;
        }

        void ReadCache::add(const ReadCacheItem& read)
        {
            _p->cache.add(read.read->getPath().get(), read);
        }

        bool ReadCache::get(const std::string& fileName, ReadCacheItem& out)
        {
            return _p->cache.get(fileName, out);
        }

        void ReadCache::setMax(size_t value)
        {
            _p->cache.setMax(value);
        }

        size_t ReadCache::getCount() const
        {
            return _p->cache.getCount();
        }

        void ReadCache::cancelRequests()
        {
            for (auto& i : _p->cache.getValues())
            {
                i.read->cancelRequests();
            }
        }
    }
}
