// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/DPX.h>

#include <tlrCore/StringFormat.h>

#include <sstream>

namespace tlr
{
    namespace dpx
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
        {}

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
            out.videoDuration = otime::RationalTime(1.0, avio::sequenceDefaultSpeed);
            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            Transfer transfer = Transfer::User;
            read(io, out, transfer);
            return out;
        }

        avio::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time)
        {
            avio::VideoFrame out;
            out.time = time;

            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            avio::Info info;
            Transfer transfer = Transfer::User;
            read(io, info, transfer);

            out.image = imaging::Image::create(info.video[0]);
            out.image->setTags(info.tags);
            io->read(out.image->getData(), imaging::getDataByteCount(info.video[0]));
            return out;
        }
    }
}
