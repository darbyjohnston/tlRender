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
            const std::string& fileName,
            const io::Info& info)
        {
            ISequenceWrite::_init(fileName, info);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const std::string& fileName,
            const io::Info& info)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(fileName, info);
            return out;
        }

        void Write::_writeVideoFrame(
            const std::string& fileName,
            const otime::RationalTime&,
            const std::shared_ptr<imaging::Image>& image)
        {
            const auto& info = image->getInfo();
            Imf::RgbaOutputFile file(fileName.c_str(), Imf::Header(info.size.w, info.size.h));
            const size_t scanlineSize = info.size.w * 4 * 2;
            uint8_t* p = image->getData() + (info.flipY ?
                (imaging::getDataByteCount(info) - scanlineSize) :
                0);
            file.setFrameBuffer(reinterpret_cast<Imf::Rgba*>(p), 1, info.flipY ? -info.size.w : info.size.w);
            file.writePixels(info.size.h);
        }
    }
}
