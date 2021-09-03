// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Cineon.h>

#include <tlrCore/StringFormat.h>

#include <sstream>

namespace tlr
{
    namespace cineon
    {
        void Read::_init(
            const file::Path& path,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
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
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, options, logSystem);
            return out;
        }

        avio::Info Read::_getInfo(const std::string& fileName)
        {
            avio::Info out;
            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            const auto header = read(io, out);
            float speed = _defaultSpeed;
            const auto i = out.tags.find("Film Frame Rate");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            out.videoTimeRange = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, speed),
                otime::RationalTime(_endFrame, speed));
            out.videoType = avio::VideoType::Sequence;
            return out;
        }

        avio::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            avio::VideoFrame out;
            out.time = time;

            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            avio::Info info;
            read(io, info);

            out.image = image && image->getInfo() == info.video[0] ? image : imaging::Image::create(info.video[0]);
            out.image->setTags(info.tags);
            io->read(out.image->getData(), imaging::getDataByteCount(info.video[0]));
            return out;
        }
    }
}
