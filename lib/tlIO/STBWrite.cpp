// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <tlIO/STB.h>

#include <tlCore/StringFormat.h>

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
                    const std::shared_ptr<imaging::Image>& image)
                {
                    const auto& info = image->getInfo();
                    const int comp = static_cast<int>(
                        imaging::getChannelCount(info.pixelType));
                    const size_t bytes = imaging::getBitDepth(info.pixelType) / 8;
                    if (bytes > 1)
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Unsupported image depth"));
                    
                    stbi_flip_vertically_on_write(1);
    
                    file::Path path(fileName);
                    std::string ext = path.getExtension();
                    int res = 0;
                    if (string::compare(
                        ext,
                        ".tga",
                        string::Compare::CaseInsensitive))
                    {
                        res = stbi_write_tga(
                            fileName.c_str(), info.size.w, info.size.h, comp,
                            image->getData());
                    }
                    else if (string::compare(
                        ext,
                        ".bmp",
                        string::Compare::CaseInsensitive))
                    {
                        res = stbi_write_bmp(
                            fileName.c_str(), info.size.w, info.size.h, comp,
                            image->getData());
                    }
                    else
                    {
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Unsupported image format"));
                    }
                    
                    if (res == 0)
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Save image failed"));
                }
            };
        }

        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
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
            const std::weak_ptr<log::System>& logSystem)
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
            const auto f = File(fileName, image);
        }
    }
}
