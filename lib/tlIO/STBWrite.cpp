// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <tlIO/STB.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

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
                    const std::shared_ptr<feather_tk::Image>& image)
                {
                    const auto& info = image->getInfo();
                    const int comp = feather_tk::getChannelCount(info.type);
                    const size_t bytes = feather_tk::getBitDepth(info.type) / 8;
                    if (bytes > 1)
                        throw std::runtime_error(feather_tk::Format("Unsupported image depth: \"{0}\"").
                            arg(fileName));
                    
                    stbi_flip_vertically_on_write(1);
    
                    file::Path path(fileName);
                    std::string ext = path.getExtension();
                    int res = 0;
                    if (feather_tk::compare(
                        ext,
                        ".tga",
                        feather_tk::CaseCompare::Insensitive))
                    {
                        res = stbi_write_tga(
                            fileName.c_str(), info.size.w, info.size.h, comp,
                            image->getData());
                    }
                    else if (feather_tk::compare(
                        ext,
                        ".bmp",
                        feather_tk::CaseCompare::Insensitive))
                    {
                        res = stbi_write_bmp(
                            fileName.c_str(), info.size.w, info.size.h, comp,
                            image->getData());
                    }
                    else
                    {
                        throw std::runtime_error(feather_tk::Format("Unsupported image format: \"{0}\"").
                            arg(fileName));
                    }
                    
                    if (res == 0)
                        throw std::runtime_error(feather_tk::Format("Save image failed: \"{0}\"").
                            arg(fileName));
                }
            };
        }

        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
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
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeVideo(
            const std::string& fileName,
            const OTIO_NS::RationalTime&,
            const std::shared_ptr<feather_tk::Image>& image,
            const io::Options&)
        {
            const auto f = File(fileName, image);
        }
    }
}
