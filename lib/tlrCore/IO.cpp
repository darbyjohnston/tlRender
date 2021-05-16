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

        void IPlugin::_init()
        {}

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

        bool System::canRead(const std::string& fileName)
        {
            for (const auto& i : _plugins)
            {
                if (i->canRead(fileName))
                {
                    return true;
                }
            }
            return false;
        }

        std::shared_ptr<IRead> System::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            for (const auto& i : _plugins)
            {
                if (i->canRead(fileName))
                {
                    return i->read(fileName, defaultSpeed);
                }
            }
            return nullptr;
        }
    }
}
