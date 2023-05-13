// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IOManager.h>

#include <tlIO/IOSystem.h>

#include <tlCore/LRUCache.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        struct IOManager::Private
        {
            std::weak_ptr<system::Context> context;
            io::Options ioOptions;
            memory::LRUCache<std::string, std::shared_ptr<io::IRead> > cache;
            std::shared_ptr<observer::Value<bool> > cancelRequests;
        };

        void IOManager::_init(
            const io::Options& ioOptions,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.ioOptions = ioOptions;
            {
                std::stringstream ss;
                ss << 1;
                p.ioOptions["ffmpeg/VideoBufferSize"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << otime::RationalTime(1.0, 1.0);
                p.ioOptions["ffmpeg/AudioBufferSize"] = ss.str();
            }
            p.cancelRequests = observer::Value<bool>::create(false);
        }

        IOManager::IOManager() :
            _p(new Private)
        {}

        IOManager::~IOManager()
        {}

        std::shared_ptr<IOManager> IOManager::create(
            const io::Options& options,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<IOManager>(new IOManager);
            out->_init(options, context);
            return out;
        }

        std::future<io::Info> IOManager::getInfo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead)
        {
            TLRENDER_P();
            std::future<io::Info> out;
            std::shared_ptr<io::IRead> read;
            const std::string fileName = path.get();
            if (!p.cache.get(fileName, read))
            {
                if (auto context = p.context.lock())
                {
                    auto ioSystem = context->getSystem<io::System>();
                    read = ioSystem->read(path, memoryRead, p.ioOptions);
                    p.cache.add(fileName, read);
                }
            }
            if (read)
            {
                out = read->getInfo();
            }
            else
            {
                std::promise<io::Info> promise;
                out = promise.get_future();
                promise.set_value(io::Info());
            }
            return out;
        }

        std::future<io::VideoData> IOManager::readVideo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            TLRENDER_P();
            std::future<io::VideoData> out;
            std::shared_ptr<io::IRead> read;
            const std::string fileName = path.get();
            if (!p.cache.get(fileName, read))
            {
                if (auto context = p.context.lock())
                {
                    auto ioSystem = context->getSystem<io::System>();
                    read = ioSystem->read(path, memoryRead, p.ioOptions);
                    p.cache.add(fileName, read);
                }
            }
            if (read)
            {
                out = read->readVideo(time, layer);
            }
            else
            {
                std::promise<io::VideoData> promise;
                out = promise.get_future();
                promise.set_value(io::VideoData());
            }
            return out;
        }

        std::future<io::AudioData> IOManager::readAudio(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::TimeRange& range)
        {
            TLRENDER_P();
            std::future<io::AudioData> out;
            std::shared_ptr<io::IRead> read;
            const std::string fileName = path.get();
            if (!p.cache.get(fileName, read))
            {
                if (auto context = p.context.lock())
                {
                    auto ioSystem = context->getSystem<io::System>();
                    read = ioSystem->read(path, memoryRead, p.ioOptions);
                    p.cache.add(fileName, read);
                }
            }
            if (read)
            {
                out = read->readAudio(range);
            }
            else
            {
                std::promise<io::AudioData> promise;
                out = promise.get_future();
                promise.set_value(io::AudioData());
            }
            return out;
        }

        void IOManager::cancelRequests()
        {
            TLRENDER_P();
            p.cancelRequests->setAlways(true);
            for (const auto& i : p.cache.getValues())
            {
                if (i)
                {
                    i->cancelRequests();
                }
            }
        }

        std::shared_ptr<observer::IValue<bool> > IOManager::observeCancelRequests() const
        {
            return _p->cancelRequests;
        }
    }
}