// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/AVIO.h>

#include <tlrCore/File.h>
#include <tlrCore/String.h>

#include <tlrCore/Cineon.h>
#include <tlrCore/DPX.h>
#if defined(PNG_FOUND)
#include <tlrCore/PNG.h>
#endif
#if defined(JPEG_FOUND)
#include <tlrCore/JPEG.h>
#endif
#if defined(TIFF_FOUND)
#include <tlrCore/TIFF.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlrCore/OpenEXR.h>
#endif
#if defined(FFmpeg_FOUND)
#include <tlrCore/FFmpeg.h>
#endif

#include <iomanip>
#include <sstream>

namespace tlr
{
    namespace avio
    {
        VideoFrame::VideoFrame() :
            time(invalidTime)
        {}

        VideoFrame::VideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image) :
            time(time),
            image(image)
        {}

        void IIO::_init(
            const std::string& fileName,
            const Options& options)
        {
            _fileName = fileName;
            _options = options;
        }

        IIO::IIO()
        {}

        IIO::~IIO()
        {}

        void IRead::_init(
            const std::string& fileName,
            const Options& options)
        {
            IIO::_init(fileName, options);
            const auto i = options.find("DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _defaultSpeed;
            }
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        void IWrite::_init(
            const std::string & fileName,
            const Options& options,
            const Info& info)
        {
            IIO::_init(fileName, options);
            _info = info;
        }

        IWrite::IWrite()
        {}

        IWrite::~IWrite()
        {}

        void IPlugin::_init(
            const std::string& name,
            const std::set<std::string>& extensions)
        {
            _name = name;
            _extensions = extensions;
        }

        IPlugin::IPlugin()
        {}

        IPlugin::~IPlugin()
        {}

        uint8_t IPlugin::getWriteAlignment() const
        {
            return 1;
        }

        memory::Endian IPlugin::getWriteEndian() const
        {
            return memory::getEndian();
        }

        bool IPlugin::_isWriteCompatible(const imaging::Info& info) const
        {
            const auto pixelTypes = getWritePixelTypes();
            const auto i = std::find(pixelTypes.begin(), pixelTypes.end(), info.pixelType);
            return i != pixelTypes.end() &&
                info.layout.alignment == getWriteAlignment() &&
                info.layout.endian == getWriteEndian();
        }

        void System::_init()
        {
            _plugins.push_back(cineon::Plugin::create());
            _plugins.push_back(dpx::Plugin::create());
#if defined(PNG_FOUND)
            _plugins.push_back(png::Plugin::create());
#endif
#if defined(JPEG_FOUND)
            _plugins.push_back(jpeg::Plugin::create());
#endif
#if defined(TIFF_FOUND)
            _plugins.push_back(tiff::Plugin::create());
#endif
#if defined(OpenEXR_FOUND)
            _plugins.push_back(exr::Plugin::create());
#endif
#if defined(FFmpeg_FOUND)
            _plugins.push_back(ffmpeg::Plugin::create());
#endif
        }

        System::System()
        {}

        std::shared_ptr<System> System::create()
        {
            auto out = std::shared_ptr<System>(new System);
            out->_init();
            return out;
        }

        namespace
        {
            std::string getExtension(const std::string& fileName)
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                file::split(fileName, &path, &baseName, &number, &extension);
                return string::toLower(extension);
            }
        }

        std::shared_ptr<IPlugin> System::getPlugin(const std::string& fileName) const
        {
            const std::string extension = getExtension(fileName);
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
            const std::string& fileName,
            const Options& options)
        {
            Options optionsTmp;
            {
                std::stringstream ss;
                ss << otime::RationalTime(1.0, 24.0);
                optionsTmp["DefaultSpeed"] = ss.str();
            }
            for (const auto& i : options)
            {
                optionsTmp[i.first] = i.second;
            }
            const std::string extension = getExtension(fileName);
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->read(fileName, optionsTmp);
                }
            }
            return nullptr;
        }

        std::shared_ptr<IWrite> System::write(
            const std::string& fileName,
            const Info& info,
            const Options& options)
        {
            const std::string extension = getExtension(fileName);
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->write(fileName, info, options);
                }
            }
            return nullptr;
        }
    }
}
