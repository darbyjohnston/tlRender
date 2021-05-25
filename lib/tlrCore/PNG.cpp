// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
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
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init({ ".png" });
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            return Read::create(fileName, defaultSpeed);
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

        std::shared_ptr<io::IWrite> Plugin::write(
            const std::string& fileName,
            const io::Info& info)
        {
            return Write::create(fileName, info);
        }
    }
}
