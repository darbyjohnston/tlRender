// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/PNG.h>

namespace tlr
{
    namespace png
    {
        extern "C"
        {
            void errorFunc(png_structp in, png_const_charp msg)
            {
                auto error = reinterpret_cast<ErrorStruct*>(png_get_error_ptr(in));
                error->message = msg;
                longjmp(png_jmpbuf(in), 1);
            }

            void warningFunc(png_structp in, png_const_charp msg)
            {
                auto error = reinterpret_cast<ErrorStruct*>(png_get_error_ptr(in));
                error->message = msg;
            }

        } // extern "C"

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "PNG",
                { { ".png", avio::FileExtensionType::VideoOnly } },
                logSystem);
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            std::shared_ptr<avio::IRead> out;
            if (auto logSystem = _logSystem.lock())
            {
                out = Read::create(path, avio::merge(options, _options), logSystem);
            }
            return out;
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::L_U8,
                imaging::PixelType::L_U16,
                imaging::PixelType::LA_U8,
                imaging::PixelType::LA_U16,
                imaging::PixelType::RGB_U8,
                imaging::PixelType::RGB_U16,
                imaging::PixelType::RGBA_U8,
                imaging::PixelType::RGBA_U16
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            std::shared_ptr<avio::IWrite> out;
            if (auto logSystem = _logSystem.lock())
            {
                out = !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                    Write::create(path, info, avio::merge(options, _options), logSystem) :
                    nullptr;
            }
            return out;
        }
    }
}
