// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Cineon.h>

#include <sstream>

namespace tl
{
    namespace cineon
    {
        void Read::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);
        }

        Read::Read()
        {}

        Read::~Read()
        {
            _finish();
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, cache, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, cache, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const file::MemoryRead* memory)
        {
            io::Info out;
            auto io = memory ?
                file::FileIO::create(fileName, *memory) :
                file::FileIO::create(fileName, file::Mode::Read);
            const auto header = read(io, out);
            float speed = _defaultSpeed;
            const auto i = out.tags.find("Film Frame Rate");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, speed),
                OTIO_NS::RationalTime(_endFrame, speed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const file::MemoryRead* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            io::VideoData out;
            out.time = time;

            auto io = memory ?
                file::FileIO::create(fileName, *memory) :
                file::FileIO::create(fileName, file::Mode::Read);
            io::Info info;
            read(io, info);

            out.image = image::Image::create(info.video[0]);
            out.image->setTags(info.tags);
            io->read(out.image->getData(), image::getDataByteCount(info.video[0]));
            return out;
        }
    }
}
