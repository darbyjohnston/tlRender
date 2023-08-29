// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

namespace tl
{
    namespace io
    {
        //! I/O cache.
        class Cache : public std::enable_shared_from_this<Cache>
        {
            TLRENDER_NON_COPYABLE(Cache);

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
            void addVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer,
                const VideoData&);

            //! Get whether the cache contains video.
            bool containsVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer) const;

            //! Get video.
            bool getVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer,
                VideoData&) const;

            //! Add audio to the cache.
            void addAudio(
                const std::string& fileName,
                const otime::TimeRange&,
                const AudioData&);

            //! Get whether the cache contains audio.
            bool containsAudio(
                const std::string& fileName,
                const otime::TimeRange&) const;

            //! Get audio from the cache.
            bool getAudio(
                const std::string& fileName,
                const otime::TimeRange&,
                AudioData&) const;

        private:
            std::string _getKey(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer) const;

            std::string _getKey(
                const std::string& fileName,
                const otime::TimeRange&) const;

            void _maxUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
