// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Cineon.h>

#include <sstream>

namespace tl
{
    namespace cineon
    {
        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            ISequenceWrite::_init(path, info, options, logSystem);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeVideo(
            const std::string& fileName,
            const OTIO_NS::RationalTime&,
            const std::shared_ptr<dtk::Image>& image,
            const io::Options&)
        {
            auto io = dtk::FileIO::create(fileName, dtk::FileMode::Write);

            io::Info info;
            const auto& imageInfo = image->getInfo();
            info.video.push_back(imageInfo);
            info.tags = image->getTags();
            write(io, info);

            const size_t scanlineByteCount = dtk::getAlignedByteCount(
                static_cast<size_t>(imageInfo.size.w) * 4,
                imageInfo.layout.alignment);
            const uint8_t* imageP = image->getData() + (imageInfo.size.h - 1) * scanlineByteCount;
            for (uint16_t y = 0; y < imageInfo.size.h; ++y, imageP -= scanlineByteCount)
            {
                io->write(imageP, scanlineByteCount);
            }

            finishWrite(io);
        }
    }
}
