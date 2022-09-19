// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/TIFF.h>

#include <tlCore/StringFormat.h>

#include <tiffio.h>

#include <sstream>

namespace tl
{
    namespace tiff
    {
        void Plugin::_init(const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "TIFF",
                {
                    { ".tiff", io::FileType::Sequence },
                    { ".tif", io::FileType::Sequence }
                },
                logSystem);
            TIFFSetErrorHandler(nullptr);
            TIFFSetWarningHandler(nullptr);
        }

        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(logSystem);
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
            const std::vector<io::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, io::merge(options, _options), _logSystem);
        }

        imaging::Info Plugin::getWriteInfo(
            const imaging::Info& info,
            const io::Options& options) const
        {
            imaging::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::L_U16:
            case imaging::PixelType::L_F32:
            case imaging::PixelType::LA_U8:
            case imaging::PixelType::LA_U16:
            case imaging::PixelType::LA_F32:
            case imaging::PixelType::RGB_U8:
            case imaging::PixelType::RGB_U16:
            case imaging::PixelType::RGB_F32:
            case imaging::PixelType::RGBA_U8:
            case imaging::PixelType::RGBA_U16:
            case imaging::PixelType::RGBA_F32:
                out.pixelType = info.pixelType;
                break;
            default: break;
            }
            out.layout.mirror.y = true;
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
