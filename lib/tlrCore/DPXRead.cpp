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
            const std::string& fileName,
            const avio::Options& options)
        {
            ISequenceRead::_init(fileName, options);
        }

        Read::Read()
        {}

        Read::~Read()
        {}

        std::shared_ptr<Read> Read::create(
            const std::string& fileName,
            const avio::Options& options)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, options);
            return out;
        }

        avio::Info Read::_getInfo(const std::string& fileName)
        {
            avio::Info out;
            out.videoDuration = otime::RationalTime(1.0, avio::sequenceDefaultSpeed);
            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Read);
            Transfer transfer = Transfer::User;
            Header::read(io, out, transfer);
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
            Header::read(io, info, transfer);

            out.image = imaging::Image::create(info.video[0]);
            out.image->setTags(info.tags);
            io->read(out.image->getData(), imaging::getDataByteCount(info.video[0]));
            return out;
        }
    }
}
