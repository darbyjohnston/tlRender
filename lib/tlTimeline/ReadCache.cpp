// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/ReadCache.h>

#include <tlCore/LRUCache.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            std::string getKey(const file::Path& path)
            {
                std::vector<std::string> out;
                out.push_back(path.get());
                out.push_back(path.getNumber());
                return string::join(out, ';');
            }
        }

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
            const file::Path& path = read.read->getPath();
            const std::string key = getKey(path);
            _p->cache.add(key, read);
        }

        bool ReadCache::get(const file::Path& path, ReadCacheItem& out)
        {
            return _p->cache.get(getKey(path), out);
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
