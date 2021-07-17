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
        void Write::_init(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            ISequenceWrite::_init(path, info, options);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options);
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

            Version version = Version::_2_0;
            Endian endian = Endian::Auto;
            Transfer transfer = Transfer::FilmPrint;
            write(io, info, version, endian, transfer);

            io->write(image->getData(), imaging::getDataByteCount(image->getInfo()));
            finishWrite(io);
        }
    }
}
