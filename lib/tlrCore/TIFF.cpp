// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TIFF.h>

#include <tiffio.h>

#include <sstream>

namespace tlr
{
    namespace tiff
    {
        void Plugin::_init(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IPlugin::_init(
                "TIFF",
                {
                    { ".tiff", avio::FileExtensionType::VideoOnly },
                    { ".tif", avio::FileExtensionType::VideoOnly }
                },
                logSystem);
            TIFFSetErrorHandler(nullptr);
            TIFFSetWarningHandler(nullptr);
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            return Read::create(path, avio::merge(options, _options), _logSystem);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::L_U8,
                imaging::PixelType::L_U16,
                imaging::PixelType::L_F32,
                imaging::PixelType::LA_U8,
                imaging::PixelType::LA_U16,
                imaging::PixelType::LA_F32,
                imaging::PixelType::RGB_U8,
                imaging::PixelType::RGB_U16,
                imaging::PixelType::RGB_F32,
                imaging::PixelType::RGBA_U8,
                imaging::PixelType::RGBA_U16,
                imaging::PixelType::RGBA_F32
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            return !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                Write::create(path, info, avio::merge(options, _options), _logSystem) :
                nullptr;
        }
    }
}
