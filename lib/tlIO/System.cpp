// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/System.h>

#if defined(TLRENDER_FFMPEG)
#include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_EXR)
#include <tlIO/OpenEXR.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_OIIO)
#include <tlIO/OIIO.h>
#endif // TLRENDER_OIIO
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD
#if defined(TLRENDER_WMF)
#include <tlIO/WMF.h>
#endif // TLRENDER_WMF

#include <ftk/Core/Context.h>
#include <ftk/Core/String.h>

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

        ReadSystem::ReadSystem(const std::shared_ptr<ftk::Context>& context) :
            ISystem(context, "tl::io::ReadSystem"),
            _p(new Private)
        {
            FTK_P();

            if (auto context = _context.lock())
            {
                auto logSystem = context->getLogSystem();
//#if defined(TLRENDER_EXR)
//                _plugins.push_back(exr::ReadPlugin::create(logSystem));
//#endif // TLRENDER_EXR
#if defined(TLRENDER_OIIO)
                _plugins.push_back(oiio::ReadPlugin::create(logSystem));
#endif // TLRENDER_OIIO
#if defined(TLRENDER_WMF)
                // \todo WMF support is still a WIP.
                //_plugins.push_back(wmf::ReadPlugin::create(logSystem));
#endif // TLRENDER_WMF
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::ReadPlugin::create(logSystem));
#endif // TLRENDER_FFMPEG
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

        std::shared_ptr<ReadSystem> ReadSystem::create(const std::shared_ptr<ftk::Context>& context)
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
            const std::string extension = ftk::toLower(path.getExtension());
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
            const std::string lower = ftk::toLower(extension);
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
            const std::string extension = ftk::toLower(path.getExtension());
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
            const std::vector<ftk::InMemoryFile>& memory,
            const Options& options)
        {
            const std::string extension = ftk::toLower(path.getExtension());
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

        WriteSystem::WriteSystem(const std::shared_ptr<ftk::Context>& context) :
            ISystem(context, "tl::io::WriteSystem"),
            _p(new Private)
        {
            FTK_P();

            if (auto context = _context.lock())
            {
                auto logSystem = context->getLogSystem();
//#if defined(TLRENDER_EXR)
//                _plugins.push_back(exr::WritePlugin::create(logSystem));
//#endif // TLRENDER_EXR
#if defined(TLRENDER_OIIO)
                _plugins.push_back(oiio::WritePlugin::create(logSystem));
#endif // TLRENDER_OIIO
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::WritePlugin::create(logSystem));
#endif // TLRENDER_FFMPEG
            }

            for (const auto& plugin : _plugins)
            {
                p.names.push_back(plugin->getName());
            }
        }

        WriteSystem::~WriteSystem()
        {}

        std::shared_ptr<WriteSystem> WriteSystem::create(const std::shared_ptr<ftk::Context>& context)
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
            const std::string extension = ftk::toLower(path.getExtension());
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
            const std::string lower = ftk::toLower(extension);
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
            const std::string extension = ftk::toLower(path.getExtension());
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

