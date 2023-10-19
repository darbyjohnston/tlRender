// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/Cache.h>

#include <tlCore/LRUCache.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <mutex>

namespace tl
{
    namespace io
    {
        struct Cache::Private
        {
            size_t max = 4 * memory::gigabyte;
            memory::LRUCache<std::string, VideoData> video;
            memory::LRUCache<std::string, AudioData> audio;
            std::mutex mutex;
        };

        void Cache::_init()
        {
            _maxUpdate();
        }

        Cache::Cache() :
            _p(new Private)
        {}

        Cache::~Cache()
        {}

        std::shared_ptr<Cache> Cache::create()
        {
            auto out = std::shared_ptr<Cache>(new Cache);
            out->_init();
            return out;
        }

        size_t Cache::getMax() const
        {
            return _p->max;
        }

        void Cache::setMax(size_t value)
        {
            TLRENDER_P();
            if (value == p.max)
                return;
            p.max = value;
            _maxUpdate();
        }

        size_t Cache::getSize() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.getSize() + p.audio.getSize();
        }

        float Cache::getPercentage() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return
                (p.video.getSize() + p.audio.getSize()) /
                static_cast<float>(p.video.getMax() + p.audio.getMax()) * 100.F;
        }

        std::string Cache::getVideoKey(
            const std::string& fileName,
            const otime::RationalTime& time,
            const Options& options)
        {
            std::vector<std::string> s;
            s.push_back(fileName);
            s.push_back(string::Format("{0}").arg(time));
            for (const auto& i : options)
            {
                s.push_back(string::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return string::join(s, ';');
        }

        void Cache::addVideo(const std::string& key, const VideoData& videoData)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.add(
                key,
                videoData,
                videoData.image ? videoData.image->getDataByteCount() : 1);
        }

        bool Cache::containsVideo(const std::string& key) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.contains(key);
        }

        bool Cache::getVideo(const std::string& key, VideoData& videoData) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.get(key, videoData);
        }

        std::string Cache::getAudioKey(
            const std::string& fileName,
            const otime::TimeRange& timeRange,
            const Options& options)
        {
            std::vector<std::string> s;
            s.push_back(fileName);
            s.push_back(string::Format("{0}").arg(timeRange));
            for (const auto& i : options)
            {
                s.push_back(string::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return string::join(s, ';');
        }

        void Cache::addAudio(const std::string& key, const AudioData& audioData)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.audio.add(
                key,
                audioData,
                audioData.audio ? audioData.audio->getByteCount() : 1);
        }

        bool Cache::containsAudio(const std::string& key) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.audio.contains(key);
        }

        bool Cache::getAudio(const std::string& key, AudioData& audioData) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.audio.get(key, audioData);
        }

        void Cache::clear()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.clear();
            p.audio.clear();
        }

        void Cache::_maxUpdate()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.setMax(p.max * .9F);
            p.audio.setMax(p.max * .1F);
        }
    }
}
