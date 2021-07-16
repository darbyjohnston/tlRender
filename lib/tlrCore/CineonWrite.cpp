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
        void Write::_init(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            ISequenceWrite::_init(fileName, info, options);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(fileName, info, options);
            return out;
        }

        void Write::_writeVideoFrame(
            const std::string& fileName,
            const otime::RationalTime&,
            const std::shared_ptr<imaging::Image>& image)
        {
            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Write);

            avio::Info info;
            info.video.push_back(image->getInfo());
            info.tags = image->getTags();
            write(io, info);

            io->write(image->getData(), imaging::getDataByteCount(image->getInfo()));
            finishWrite(io);
        }
    }
}
