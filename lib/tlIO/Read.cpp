// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Read.h>

#include <dtk/core/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IRead::_init(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const Options& options,
            const std::shared_ptr<Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IIO::_init(path, options, logSystem);
            _memory = memory;
            _cache = cache;
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        std::future<VideoData> IRead::readVideo(
            const OTIO_NS::RationalTime&,
            const Options&)
        {
            return std::future<VideoData>();
        }

        std::future<AudioData> IRead::readAudio(
            const OTIO_NS::TimeRange&,
            const Options&)
        {
            return std::future<AudioData>();
        }

        struct IReadPlugin::Private
        {
        };

        void IReadPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IPlugin::_init(name, extensions, logSystem);
            DTK_P();
            _cache = cache;
        }

        IReadPlugin::IReadPlugin() :
            _p(new Private)
        {}

        IReadPlugin::~IReadPlugin()
        {}
    }
}
