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
        void Plugin::_init()
        {
            IPlugin::_init(
                "TIFF",
                { ".tiff", ".tif" });
            TIFFSetErrorHandler(nullptr);
            TIFFSetWarningHandler(nullptr);
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init();
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            return Read::create(path, options);
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
                Write::create(path, info, options) :
                nullptr;
        }
    }
}
