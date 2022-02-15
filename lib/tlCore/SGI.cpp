// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/SGI.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace sgi
    {
        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "SGI",
                {
                    { ".sgi", avio::FileExtensionType::VideoOnly },
                    { ".rgba", avio::FileExtensionType::VideoOnly },
                    { ".rgb", avio::FileExtensionType::VideoOnly },
                    { ".bw", avio::FileExtensionType::VideoOnly }
                },
                logSystem);
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            return Read::create(path, avio::merge(options, _options), _logSystem);
        }

        imaging::Info Plugin::getWriteInfo(const imaging::Info& info, const avio::Options& options) const
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
            default: break;
            }
            out.layout.endian = memory::Endian::MSB;
            return out;
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isWriteCompatible(info.video[0], options)))
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(path.get()).
                    arg("Unsupported video"));
            return Write::create(path, info, avio::merge(options, _options), _logSystem);
        }
    }
}
