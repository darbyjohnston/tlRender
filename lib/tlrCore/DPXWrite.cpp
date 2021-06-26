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
        namespace
        {
            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<imaging::Image>& image)
                {
                }

                ~File()
                {
                }

            private:
            };
        }

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
            const auto f = File(fileName, image);
        }
    }
}
