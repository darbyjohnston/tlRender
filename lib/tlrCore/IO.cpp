// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/IO.h>

#include <tlrCore/File.h>
#include <tlrCore/String.h>

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
    namespace io
    {
        VideoFrame::VideoFrame()
        {}

        VideoFrame::VideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image) :
            time(time),
            image(image)
        {}

        void IIO::_init(const std::string& fileName)
        {
            _fileName = fileName;
        }

        IIO::IIO()
        {}

        IIO::~IIO()
        {}

        void IRead::_init(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            IIO::_init(fileName);
            _defaultSpeed = defaultSpeed;
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        void IWrite::_init(
            const std::string & fileName,
            const io::Info& info)
        {
            IIO::_init(fileName);
            _info = info;
        }

        IWrite::IWrite()
        {}

        IWrite::~IWrite()
        {}

        void IPlugin::_init(const std::set<std::string>& extensions)
        {
            _extensions = extensions;
        }

        IPlugin::IPlugin()
        {}

        IPlugin::~IPlugin()
        {}

        void System::_init()
        {
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

        std::shared_ptr<IRead> System::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            const std::string extension = getExtension(fileName);
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->read(fileName, defaultSpeed);
                }
            }
            return nullptr;
        }

        std::shared_ptr<IWrite> System::write(
            const std::string& fileName,
            const io::Info& info)
        {
            const std::string extension = getExtension(fileName);
            for (const auto& i : _plugins)
            {
                const auto& extensions = i->getExtensions();
                if (extensions.find(extension) != extensions.end())
                {
                    return i->write(fileName, info);
                }
            }
            return nullptr;
        }
    }
}
