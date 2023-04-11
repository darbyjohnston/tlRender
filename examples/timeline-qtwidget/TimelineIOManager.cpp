// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineIOManager.h"

#include <tlIO/IOSystem.h>

#include <sstream>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TimelineIOManager::_init(
                const io::Options& ioOptions,
                const std::shared_ptr<system::Context>& context)
            {
                _context = context;
                _ioOptions = ioOptions;
                {
                    std::stringstream ss;
                    ss << 1;
                    _ioOptions["ffmpeg/VideoBufferSize"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << otime::RationalTime(1.0, 1.0);
                    _ioOptions["ffmpeg/AudioBufferSize"] = ss.str();
                }
                _cancelRequests = observer::Value<bool>::create(false);
            }

            TimelineIOManager::TimelineIOManager()
            {}

            TimelineIOManager::~TimelineIOManager()
            {}

            std::shared_ptr<TimelineIOManager> TimelineIOManager::create(
                const io::Options& options,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<TimelineIOManager>(new TimelineIOManager);
                out->_init(options, context);
                return out;
            }

            std::future<io::Info> TimelineIOManager::getInfo(const file::Path& path)
            {
                std::future<io::Info> out;
                std::shared_ptr<io::IRead> read;
                const std::string fileName = path.get();
                if (!_cache.get(fileName, read))
                {
                    if (auto context = _context.lock())
                    {
                        auto ioSystem = context->getSystem<io::System>();
                        read = ioSystem->read(path, _ioOptions);
                        _cache.add(fileName, read);
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

            std::future<io::VideoData> TimelineIOManager::readVideo(
                const file::Path& path,
                const otime::RationalTime& time,
                uint16_t layer)
            {
                std::future<io::VideoData> out;
                std::shared_ptr<io::IRead> read;
                const std::string fileName = path.get();
                if (!_cache.get(fileName, read))
                {
                    if (auto context = _context.lock())
                    {
                        auto ioSystem = context->getSystem<io::System>();
                        read = ioSystem->read(path, _ioOptions);
                        _cache.add(fileName, read);
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

            std::future<io::AudioData> TimelineIOManager::readAudio(
                const file::Path& path,
                const otime::TimeRange& range)
            {
                std::future<io::AudioData> out;
                std::shared_ptr<io::IRead> read;
                const std::string fileName = path.get();
                if (!_cache.get(fileName, read))
                {
                    if (auto context = _context.lock())
                    {
                        auto ioSystem = context->getSystem<io::System>();
                        read = ioSystem->read(path, _ioOptions);
                        _cache.add(fileName, read);
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

            void TimelineIOManager::cancelRequests()
            {
                _cancelRequests->setAlways(true);
                for (const auto& i : _cache.getValues())
                {
                    if (i)
                    {
                        i->cancelRequests();
                    }
                }
            }

            std::shared_ptr<observer::IValue<bool> > TimelineIOManager::observeCancelRequests() const
            {
                return _cancelRequests;
            }
        }
    }
}
