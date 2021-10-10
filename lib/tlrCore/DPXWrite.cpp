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
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            ISequenceWrite::_init(path, info, options, logSystem);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeVideo(
            const std::string& fileName,
            const otime::RationalTime&,
            const std::shared_ptr<imaging::Image>& image)
        {
            auto io = file::FileIO::create();
            io->open(fileName, file::Mode::Write);

            avio::Info info;
            const auto& imageInfo = image->getInfo();
            info.video.push_back(imageInfo);
            info.tags = image->getTags();

            Version version = Version::_2_0;
            Endian endian = Endian::Auto;
            Transfer transfer = Transfer::FilmPrint;
            write(io, info, version, endian, transfer);

            const size_t scanlineSize = imaging::align(static_cast<size_t>(imageInfo.size.w) * 4, imageInfo.layout.alignment);
            const uint8_t* imageP = image->getData() + (imageInfo.size.h - 1) * scanlineSize;
            for (uint16_t y = 0; y < imageInfo.size.h; ++y, imageP -= scanlineSize)
            {
                io->write(imageP, scanlineSize);
            }

            finishWrite(io);
        }
    }
}
