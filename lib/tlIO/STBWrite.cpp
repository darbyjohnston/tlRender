// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <tlIO/STB.h>

#include <dtk/core/Format.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace stb
    {
        namespace
        {
            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<dtk::Image>& image)
                {
                    const auto& info = image->getInfo();
                    const int comp = dtk::getChannelCount(info.type);
                    const size_t bytes = dtk::getBitDepth(info.type) / 8;
                    if (bytes > 1)
                        throw std::runtime_error(dtk::Format("{0}: {1}").
                            arg(fileName).
                            arg("Unsupported image depth"));
                    
                    stbi_flip_vertically_on_write(1);
    
                    file::Path path(fileName);
                    std::string ext = path.getExtension();
                    int res = 0;
                    if (dtk::compare(
                        ext,
                        ".tga",
                        dtk::CaseCompare::Insensitive))
                    {
                        res = stbi_write_tga(
                            fileName.c_str(), info.size.w, info.size.h, comp,
                            image->getData());
                    }
                    else if (dtk::compare(
                        ext,
                        ".bmp",
                        dtk::CaseCompare::Insensitive))
                    {
                        res = stbi_write_bmp(
                            fileName.c_str(), info.size.w, info.size.h, comp,
                            image->getData());
                    }
                    else
                    {
                        throw std::runtime_error(dtk::Format("{0}: {1}").
                            arg(fileName).
                            arg("Unsupported image format"));
                    }
                    
                    if (res == 0)
                        throw std::runtime_error(dtk::Format("{0}: {1}").
                            arg(fileName).
                            arg("Save image failed"));
                }
            };
        }

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
            const auto f = File(fileName, image);
        }
    }
}
