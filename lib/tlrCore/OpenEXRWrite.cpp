// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/OpenEXR.h>

#include <tlrCore/StringFormat.h>

#include <ImfRgbaFile.h>

namespace tlr
{
    namespace exr
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

        void Write::_writeVideoFrame(
            const std::string& fileName,
            const otime::RationalTime&,
            const std::shared_ptr<imaging::Image>& image)
        {
            const auto& info = image->getInfo();
            Imf::Header header(info.size.w, info.size.h);
            writeTags(image->getTags(), avio::sequenceDefaultSpeed, header);
            Imf::RgbaOutputFile f(fileName.c_str(), header);
            const size_t scanlineSize = static_cast<size_t>(info.size.w) * 4 * 2;
            const uint8_t* p = image->getData() + (info.size.h - 1) * scanlineSize;
            f.setFrameBuffer(reinterpret_cast<const Imf::Rgba*>(p), 1, -info.size.w);
            f.writePixels(info.size.h);
        }
    }
}
