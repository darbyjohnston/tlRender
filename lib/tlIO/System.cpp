// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/System.h>

#include <tlIO/Cineon.h>
#include <tlIO/DPX.h>
#include <tlIO/PPM.h>
#include <tlIO/SGI.h>
#if defined(TLRENDER_STB)
#include <tlIO/STB.h>
#endif
#if defined(TLRENDER_FFMPEG)
#include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
#include <tlIO/JPEG.h>
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
#include <tlIO/OpenEXR.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
#include <tlIO/PNG.h>
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
#include <tlIO/TIFF.h>
#endif // TLRENDER_TIFF
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <dtk/core/Context.h>
#include <dtk/core/String.h>

#include <iomanip>
#include <sstream>

namespace tl
{
    namespace io
    {
        struct ReadSystem::Private
        {
            std::vector<std::string> names;
        };

        ReadSystem::ReadSystem(const std::shared_ptr<dtk::Context>& context) :
            ISystem(context, "tl::io::ReadSystem"),
            _p(new Private)
        {
            DTK_P();

            if (auto context = _context.lock())
            {
                auto logSystem = context->getLogSystem();
                _plugins.push_back(cineon::ReadPlugin::create(logSystem));
                _plugins.push_back(dpx::ReadPlugin::create(logSystem));
                _plugins.push_back(ppm::ReadPlugin::create(logSystem));
                _plugins.push_back(sgi::ReadPlugin::create(logSystem));
#if defined(TLRENDER_STB)
                _plugins.push_back(stb::ReadPlugin::create(logSystem));
#endif
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::ReadPlugin::create(logSystem));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
                _plugins.push_back(jpeg::ReadPlugin::create(logSystem));
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
                _plugins.push_back(exr::ReadPlugin::create(logSystem));
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
                _plugins.push_back(png::ReadPlugin::create(logSystem));
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
                _plugins.push_back(tiff::ReadPlugin::create(logSystem));
#endif // TLRENDER_TIFF
#if defined(TLRENDER_USD)
                _plugins.push_back(usd::ReadPlugin::create(logSystem));
#endif // TLRENDER_USD
            }

            for (const auto& plugin : _plugins)
            {
                p.names.push_back(plugin->getName());
            }
        }

        ReadSystem::~ReadSystem()
        {}

        std::shared_ptr<ReadSystem> ReadSystem::create(const std::shared_ptr<dtk::Context>& context)
        {
            auto out = context->getSystem<ReadSystem>();
            if (!out)
            {
                out = std::shared_ptr<ReadSystem>(new ReadSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        std::shared_ptr<IReadPlugin> ReadSystem::getPlugin(const file::Path& path) const
        {
            const std::string extension = dtk::toLower(path.getExtension());
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
        
        void ReadSystem::addPlugin(const std::shared_ptr<IReadPlugin>& plugin)
        {
            _plugins.push_back(plugin);
        }

        void ReadSystem::removePlugin(const std::shared_ptr<IReadPlugin>& plugin)
        {
            const auto i = std::find(_plugins.begin(), _plugins.end(), plugin);
            if (i != _plugins.end())
            {
                _plugins.erase(i);
            }
        }

        const std::vector<std::string>& ReadSystem::getNames() const
        {
            return _p->names;
        }

        std::set<std::string> ReadSystem::getExtensions(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions(types);
                out.insert(extensions.begin(), extensions.end());
            }
            return out;
        }
        
        FileType ReadSystem::getFileType(const std::string& extension) const
        {
            FileType out = FileType::Unknown;
            const std::string lower = dtk::toLower(extension);
            for (const auto& plugin : _plugins)
            {
                for (auto fileType : { FileType::Media, FileType::Sequence })
                {
                    const auto& extensions = plugin->getExtensions(static_cast<int>(fileType));
                    const auto i = extensions.find(lower);
                    if (i != extensions.end())
                    {
                        out = fileType;
                        break;
                    }
                }
            }
            return out;
        }

        std::shared_ptr<IRead> ReadSystem::read(
            const file::Path& path,
            const Options& options)
        {
            const std::string extension = dtk::toLower(path.getExtension());
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

        std::shared_ptr<IRead> ReadSystem::read(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const Options& options)
        {
            const std::string extension = dtk::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->read(path, memory, options);
                }
            }
            return nullptr;
        }

        struct WriteSystem::Private
        {
            std::vector<std::string> names;
        };

        WriteSystem::WriteSystem(const std::shared_ptr<dtk::Context>& context) :
            ISystem(context, "tl::io::WriteSystem"),
            _p(new Private)
        {
            DTK_P();

            if (auto context = _context.lock())
            {
                auto logSystem = context->getLogSystem();
                _plugins.push_back(cineon::WritePlugin::create(logSystem));
                _plugins.push_back(dpx::WritePlugin::create(logSystem));
                _plugins.push_back(ppm::WritePlugin::create(logSystem));
                _plugins.push_back(sgi::WritePlugin::create(logSystem));
#if defined(TLRENDER_STB)
                _plugins.push_back(stb::WritePlugin::create(logSystem));
#endif
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::WritePlugin::create(logSystem));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
                _plugins.push_back(jpeg::WritePlugin::create(logSystem));
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
                _plugins.push_back(exr::WritePlugin::create(logSystem));
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
                _plugins.push_back(png::WritePlugin::create(logSystem));
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
                _plugins.push_back(tiff::WritePlugin::create(logSystem));
#endif // TLRENDER_TIFF
            }

            for (const auto& plugin : _plugins)
            {
                p.names.push_back(plugin->getName());
            }
        }

        WriteSystem::~WriteSystem()
        {}

        std::shared_ptr<WriteSystem> WriteSystem::create(const std::shared_ptr<dtk::Context>& context)
        {
            auto out = context->getSystem<WriteSystem>();
            if (!out)
            {
                out = std::shared_ptr<WriteSystem>(new WriteSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        std::shared_ptr<IWritePlugin> WriteSystem::getPlugin(const file::Path& path) const
        {
            const std::string extension = dtk::toLower(path.getExtension());
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

        void WriteSystem::addPlugin(const std::shared_ptr<IWritePlugin>& plugin)
        {
            _plugins.push_back(plugin);
        }

        void WriteSystem::removePlugin(const std::shared_ptr<IWritePlugin>& plugin)
        {
            const auto i = std::find(_plugins.begin(), _plugins.end(), plugin);
            if (i != _plugins.end())
            {
                _plugins.erase(i);
            }
        }

        const std::vector<std::string>& WriteSystem::getNames() const
        {
            return _p->names;
        }

        std::set<std::string> WriteSystem::getExtensions(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions(types);
                out.insert(extensions.begin(), extensions.end());
            }
            return out;
        }

        FileType WriteSystem::getFileType(const std::string& extension) const
        {
            FileType out = FileType::Unknown;
            const std::string lower = dtk::toLower(extension);
            for (const auto& plugin : _plugins)
            {
                for (auto fileType : { FileType::Media, FileType::Sequence })
                {
                    const auto& extensions = plugin->getExtensions(static_cast<int>(fileType));
                    const auto i = extensions.find(lower);
                    if (i != extensions.end())
                    {
                        out = fileType;
                        break;
                    }
                }
            }
            return out;
        }

        std::shared_ptr<IWrite> WriteSystem::write(
            const file::Path& path,
            const Info& info,
            const Options& options)
        {
            const std::string extension = dtk::toLower(path.getExtension());
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

