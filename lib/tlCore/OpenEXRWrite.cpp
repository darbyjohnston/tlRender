// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/OpenEXR.h>

#include <tlCore/StringFormat.h>

#include <ImfRgbaFile.h>
#include <ImfStandardAttributes.h>

namespace tl
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

            auto i = options.find("exr/Compression");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _compression;
            }
            i = options.find("exr/DWACompressionLevel");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _dwaCompressionLevel;
            }
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
            const auto& info = image->getInfo();
            Imf::Header header(
                info.size.w,
                info.size.h,
                1.F,
                Imath::V2f(0.F, 0.F),
                1.F,
                Imf::INCREASING_Y,
                toImf(_compression));
            addDwaCompressionLevel(header, _dwaCompressionLevel);
            writeTags(image->getTags(), avio::sequenceDefaultSpeed, header);
            Imf::RgbaOutputFile f(fileName.c_str(), header);
            const size_t scanlineSize = static_cast<size_t>(info.size.w) * 4 * 2;
            const uint8_t* p = image->getData() + (info.size.h - 1) * scanlineSize;
            f.setFrameBuffer(reinterpret_cast<const Imf::Rgba*>(p), 1, -info.size.w);
            f.writePixels(info.size.h);
        }
    }
}
