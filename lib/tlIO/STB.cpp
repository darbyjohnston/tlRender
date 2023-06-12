// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/STB.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace stb
    {
        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "STB",
                {
                    { ".tga", io::FileType::Sequence },
                    { ".bmp", io::FileType::Sequence },
                    { ".psd", io::FileType::Sequence },
                },
                logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, io::merge(options, _options), _logSystem);
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, io::merge(options, _options), _logSystem);
        }

        imaging::Info Plugin::getWriteInfo(const imaging::Info& info, const io::Options& options) const
        {
            imaging::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::L_U16:
            case imaging::PixelType::LA_U8:
            case imaging::PixelType::LA_U16:
            case imaging::PixelType::RGB_U8:
            case imaging::PixelType::RGB_U16:
            case imaging::PixelType::RGBA_U8:
            case imaging::PixelType::RGBA_U16:
                out.pixelType = info.pixelType;
                break;
            case imaging::PixelType::L_F16:
                out.pixelType = imaging::PixelType::L_U8;
                break;
            case imaging::PixelType::LA_F16:
                out.pixelType = imaging::PixelType::LA_U8;
                break;
            case imaging::PixelType::RGB_F16:
            case imaging::PixelType::RGB_U32:
            case imaging::PixelType::RGB_F32:
                out.pixelType = imaging::PixelType::RGB_U8;
                break;
            case imaging::PixelType::RGBA_F16:
            case imaging::PixelType::RGBA_U32:
            case imaging::PixelType::RGBA_F32:
                out.pixelType = imaging::PixelType::RGBA_U8;
                break;
            default: break;
            }
            out.layout.endian = memory::Endian::MSB;
            return out;
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isWriteCompatible(info.video[0], options)))
                throw std::runtime_error(string::Format("{0}: {1}").
                                         arg(path.get()).
                                         arg("Unsupported video"));
            return Write::create(path, info, io::merge(options, _options), _logSystem);
        }
    }
}
