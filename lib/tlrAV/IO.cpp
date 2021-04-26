// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrAV/IO.h>

#include <tlrAV/FFmpeg.h>

namespace tlr
{
    namespace av
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
                size_t videoQueueSize)
            {
                IIO::_init(fileName);

                _videoQueueSize = videoQueueSize;
            }

            IRead::IRead()
            {}

            void IRead::seek(const otime::RationalTime& time)
            {
                _hasSeek = true;
                _seekTime = time;
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
                _plugins.push_back(ffmpeg::Plugin::create());

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

            std::shared_ptr<IRead> System::read(const std::string& fileName)
            {
                for (const auto& i : _plugins)
                {
                    if (i->canRead(fileName))
                    {
                        return i->read(fileName);
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
}
