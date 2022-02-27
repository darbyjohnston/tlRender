// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/DPX.h>

#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace dpx
    {
        void Read::_init(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
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
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(const std::string& fileName)
        {
            io::Info out;
            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            Transfer transfer = Transfer::User;
            const auto header = read(io, out, transfer);
            float speed = _defaultSpeed;
            auto i = out.tags.find("Film Frame Rate");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            else
            {
                i = out.tags.find("TV Frame Rate");
                if (i != out.tags.end())
                {
                    speed = std::stof(i->second);
                }
            }
            out.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, speed),
                otime::RationalTime(_endFrame, speed));
            out.videoType = io::VideoType::Sequence;
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            io::VideoData out;
            out.time = time;

            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            io::Info info;
            Transfer transfer = Transfer::User;
            read(io, info, transfer);

            out.image = imaging::Image::create(info.video[0]);
            out.image->setTags(info.tags);
            io->read(out.image->getData(), imaging::getDataByteCount(info.video[0]));
            return out;
        }
    }
}
