// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/StringFormat.h>

namespace tlr
{
    namespace ffmpeg
    {
        void Write::_init(
            const std::string& fileName,
            const io::Info& info)
        {
            IWrite::_init(fileName, info);
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

        void Write::writeVideoFrame(
            const otime::RationalTime&,
            const std::shared_ptr<imaging::Image>& image)
        {
        }
    }
}
