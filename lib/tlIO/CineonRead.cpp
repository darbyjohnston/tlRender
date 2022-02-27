// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/Cineon.h>

#include <tlCore/StringFormat.h>

#include <sstream>

using namespace tl::core;

namespace tl
{
    namespace io
    {
        namespace cineon
        {
            void Read::_init(
                const file::Path& path,
                const Options& options,
                const std::weak_ptr<core::LogSystem>& logSystem)
            {
                ISequenceRead::_init(path, options, logSystem);
            }

            Read::Read()
            {}

            Read::~Read()
            {
                _finish();
            }

            std::shared_ptr<Read> Read::create(
                const file::Path& path,
                const Options& options,
                const std::weak_ptr<core::LogSystem>& logSystem)
            {
                auto out = std::shared_ptr<Read>(new Read);
                out->_init(path, options, logSystem);
                return out;
            }

            Info Read::_getInfo(const std::string& fileName)
            {
                Info out;
                auto io = file::FileIO::create();
                io->open(fileName, file::Mode::Read);
                const auto header = read(io, out);
                float speed = _defaultSpeed;
                const auto i = out.tags.find("Film Frame Rate");
                if (i != out.tags.end())
                {
                    speed = std::stof(i->second);
                }
                out.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                    otime::RationalTime(_startFrame, speed),
                    otime::RationalTime(_endFrame, speed));
                out.videoType = VideoType::Sequence;
                return out;
            }

            VideoData Read::_readVideo(
                const std::string& fileName,
                const otime::RationalTime& time,
                uint16_t layer)
            {
                VideoData out;
                out.time = time;

                auto io = file::FileIO::create();
                io->open(fileName, file::Mode::Read);
                Info info;
                read(io, info);

                out.image = imaging::Image::create(info.video[0]);
                out.image->setTags(info.tags);
                io->read(out.image->getData(), imaging::getDataByteCount(info.video[0]));
                return out;
            }
        }
    }
}
