// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace io
    {
        //! Get an I/O information cache key.
        std::string getInfoCacheKey(
            const file::Path&,
            const Options&);

        //! Get a video cache key.
        std::string getVideoCacheKey(
            const file::Path&,
            const OTIO_NS::RationalTime&,
            const Options& initOptions,
            const Options& frameOptions);

        //! Get an audio cache key.
        std::string getAudioCacheKey(
            const file::Path&,
            const OTIO_NS::TimeRange&,
            const Options& initOptions,
            const Options& frameOptions);

        //! I/O cache.
        class Cache : public std::enable_shared_from_this<Cache>
        {
            DTK_NON_COPYABLE(Cache);

        protected:
            void _init();

            Cache();

        public:
            ~Cache();

            //! Create a new cache.
            static std::shared_ptr<Cache> create();

            //! Get the maximum cache size in bytes.
            size_t getMax() const;

            //! Set the maximum cache size in bytes.
            void setMax(size_t);

            //! Get the current cache size in bytes.
            size_t getSize() const;

            //! Get the current cache size as a percentage.
            float getPercentage() const;

            //! Add video to the cache.
            void addVideo(const std::string& key, const VideoData&);

            //! Get whether the cache contains video.
            bool containsVideo(const std::string& key) const;

            //! Get video from the cache.
            bool getVideo(const std::string& key, VideoData&) const;

            //! Add audio to the cache.
            void addAudio(const std::string& key, const AudioData&);

            //! Get whether the cache contains audio.
            bool containsAudio(const std::string& key) const;

            //! Get audio from the cache.
            bool getAudio(const std::string& key, AudioData&) const;

            //! Clear the cache.
            void clear();

        private:
            void _maxUpdate();

            DTK_PRIVATE();
        };
    }
}
