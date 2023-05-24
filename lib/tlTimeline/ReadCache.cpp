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
            enum class FileNameType
            {
                Regular,
                Sequence
            };

            static FileNameType getFileNameType(const file::Path& path)
            {
                return path.getNumber().empty() ?
                    FileNameType::Regular :
                    FileNameType::Sequence;
            }

            typedef std::pair<std::string, FileNameType> Key;

            memory::LRUCache<Key, ReadCacheItem> cache;
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
            const Private::Key key(
                path.get(),
                Private::getFileNameType(path));
            _p->cache.add(key, read);
        }

        bool ReadCache::get(const file::Path& path, ReadCacheItem& out)
        {
            const Private::Key key(
                path.get(),
                Private::getFileNameType(path));
            return _p->cache.get(key, out);
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
