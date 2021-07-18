// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/AVIOSystem.h>

#include <tlrCore/Context.h>
#include <tlrCore/File.h>
#include <tlrCore/String.h>

#include <tlrCore/Cineon.h>
#include <tlrCore/DPX.h>
#if defined(FFmpeg_FOUND)
#include <tlrCore/FFmpeg.h>
#endif
#if defined(JPEG_FOUND)
#include <tlrCore/JPEG.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlrCore/OpenEXR.h>
#endif
#if defined(PNG_FOUND)
#include <tlrCore/PNG.h>
#endif
#if defined(TIFF_FOUND)
#include <tlrCore/TIFF.h>
#endif

#include <iomanip>
#include <sstream>

namespace tlr
{
    namespace avio
    {
        void System::_init()
        {
            auto logSystem = _context->getLogSystem();
            _plugins.push_back(cineon::Plugin::create(logSystem));
            _plugins.push_back(dpx::Plugin::create(logSystem));
#if defined(FFmpeg_FOUND)
            _plugins.push_back(ffmpeg::Plugin::create(logSystem));
#endif
#if defined(JPEG_FOUND)
            _plugins.push_back(jpeg::Plugin::create(logSystem));
#endif
#if defined(OpenEXR_FOUND)
            _plugins.push_back(exr::Plugin::create(logSystem));
#endif
#if defined(PNG_FOUND)
            _plugins.push_back(png::Plugin::create(logSystem));
#endif
#if defined(TIFF_FOUND)
            _plugins.push_back(tiff::Plugin::create(logSystem));
#endif
        }

        System::System(const std::shared_ptr<core::Context>& context) :
            ISystem("tlr::avio::System", context)
        {}

        System::~System()
        {}

        std::shared_ptr<System> System::create(const std::shared_ptr<core::Context>& context)
        {
            auto out = std::shared_ptr<System>(new System(context));
            out->_init();
            return out;
        }

        void System::setOptions(const Options& options)
        {
            for (const auto& i : _plugins)
            {
                i->setOptions(options);
            }
        }

        std::shared_ptr<IPlugin> System::getPlugin(const file::Path& path) const
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i;
                }
            }
            return nullptr;
        }

        std::shared_ptr<IRead> System::read(
            const file::Path& path,
            const Options& options)
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->read(path, options);
                }
            }
            return nullptr;
        }

        std::shared_ptr<IWrite> System::write(
            const file::Path& path,
            const Info& info,
            const Options& options)
        {
            const std::string extension = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->write(path, info, options);
                }
            }
            return nullptr;
        }
    }
}
