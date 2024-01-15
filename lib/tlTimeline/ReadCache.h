// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

namespace tl
{
    namespace timeline
    {
        //! I/O read cache item.
        struct ReadCacheItem
        {
            std::shared_ptr<io::IRead> read;
            io::Info ioInfo;
        };

        //! I/O read cache.
        class ReadCache
        {
            TLRENDER_NON_COPYABLE(ReadCache);

        protected:
            void _init();

            ReadCache();

        public:
            ~ReadCache();

            //! Create a new read cache.
            static std::shared_ptr<ReadCache> create();

            //! Add an item to the cache.
            void add(const ReadCacheItem&);

            //! Get an item from the cache.
            bool get(const file::Path&, ReadCacheItem&);

            //! Set the maximum number of read objects.
            void setMax(size_t);

            //! Get the number of read objects.
            size_t getCount() const;

            //! Cancel requests.
            void cancelRequests();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
