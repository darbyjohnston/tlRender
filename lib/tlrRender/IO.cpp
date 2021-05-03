// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrRender/IO.h>

#include <tlrRender/File.h>
#include <tlrRender/String.h>

#if defined(PNG_FOUND)
#include <tlrRender/PNG.h>
#endif
#if defined(JPEG_FOUND)
#include <tlrRender/JPEG.h>
#endif
#if defined(TIFF_FOUND)
#include <tlrRender/TIFF.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlrRender/OpenEXR.h>
#endif
#if defined(FFmpeg_FOUND)
#include <tlrRender/FFmpeg.h>
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
            const otime::RationalTime& defaultSpeed,
            size_t videoQueueSize)
        {
            IIO::_init(fileName);

            _defaultSpeed = defaultSpeed;
            _videoQueueSize = videoQueueSize;
        }

        IRead::IRead()
        {}

        void IRead::seek(const otime::RationalTime& time)
        {
            _hasSeek = true;
            _seekTime = time;
        }

        void ISequenceRead::_init(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed,
            size_t videoQueueSize)
        {
            IRead::_init(fileName, defaultSpeed, videoQueueSize);

            file::split(fileName, &_path, &_baseName, &_number, &_extension);
            _pad = !_number.empty() ? ('0' == _number[0] ? _number.size() : 0) : 0;
        }

        ISequenceRead::ISequenceRead()
        {}

        std::string ISequenceRead::_getFileName(const otime::RationalTime& value) const
        {
            std::stringstream ss;
            ss << _path << _baseName << std::setfill('0') << std::setw(_pad) << static_cast<int>(value.value()) << _extension;
            return ss.str();
        }

        void IPlugin::_init()
        {}

        IPlugin::IPlugin()
        {}

        IPlugin::~IPlugin()
        {}

        void IPlugin::setVideoQueueSize(size_t size)
        {
            _videoQueueSize = size;
        }

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
            for (const auto& i : _plugins)
            {
                i->setVideoQueueSize(_videoQueueSize);
            }
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

        void System::setVideoQueueSize(size_t size)
        {
            if (size == _videoQueueSize)
                return;
            _videoQueueSize = size;
            for (const auto& i : _plugins)
            {
                i->setVideoQueueSize(size);
            }
        }
    }
}
