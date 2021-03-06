// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/IOSystem.h>

#include <tlIO/Cineon.h>
#include <tlIO/DPX.h>
#include <tlIO/PPM.h>
#include <tlIO/SGI.h>
#if defined(FFmpeg_FOUND)
#include <tlIO/FFmpeg.h>
#endif
#if defined(JPEG_FOUND)
#include <tlIO/JPEG.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlIO/OpenEXR.h>
#endif
#if defined(PNG_FOUND)
#include <tlIO/PNG.h>
#endif
#if defined(TIFF_FOUND)
#include <tlIO/TIFF.h>
#endif

#include <tlCore/Context.h>
#include <tlCore/File.h>
#include <tlCore/String.h>

#include <iomanip>
#include <sstream>

namespace tl
{
    namespace io
    {
        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::io::System", context);

            if (auto context = _context.lock())
            {
                auto logSystem = context->getLogSystem();
                _plugins.push_back(cineon::Plugin::create(logSystem));
                _plugins.push_back(dpx::Plugin::create(logSystem));
                _plugins.push_back(ppm::Plugin::create(logSystem));
                _plugins.push_back(sgi::Plugin::create(logSystem));
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
        }

        System::System()
        {}

        System::~System()
        {}

        std::shared_ptr<System> System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System);
                out->_init(context);
            }
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

        std::set<std::string> System::getExtensions(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions(types);
                out.insert(extensions.begin(), extensions.end());
            }
            return out;
        }
        
        FileType System::getFileType(const std::string& extension) const
        {
            FileType out = FileType::Unknown;
            for (const auto& plugin : _plugins)
            {
                for (auto fileType : { FileType::Movie, FileType::Sequence, FileType::Audio })
                {
                    const auto& extensions = plugin->getExtensions(static_cast<int>(fileType));
                    const auto i = extensions.find(extension);
                    if (i != extensions.end())
                    {
                        out = fileType;
                        break;
                    }
                }
            }
            return out;
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

