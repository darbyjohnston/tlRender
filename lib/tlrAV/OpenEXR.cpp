// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrAV/OpenEXR.h>

#include <tlrCore/Assert.h>
#include <tlrCore/String.h>

#include <ImfRgbaFile.h>

namespace tlr
{
    namespace av
    {
        namespace exr
        {
            namespace
            {
                imaging::Info imfInfo(const Imf::RgbaInputFile& f)
                {
                    imaging::PixelType pixelType = imaging::getFloatType(4, 16);
                    if (imaging::PixelType::None == pixelType)
                    {
                        std::stringstream ss;
                        ss << f.fileName() << ": File not supported";
                        throw std::runtime_error(ss.str());
                    }
                    const auto dw = f.dataWindow();
                    const int width = dw.max.x - dw.min.x + 1;
                    const int height = dw.max.y - dw.min.y + 1;
                    io::VideoInfo info;
                    return imaging::Info(width, height, pixelType);
                }
            }

            void Read::_init(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed,
                size_t videoQueueSize)
            {
                ISequenceRead::_init(fileName, defaultSpeed, videoQueueSize);

                Imf::RgbaInputFile f(fileName.c_str());
                io::VideoInfo info;
                info.info = imfInfo(f);
                info.duration = _defaultSpeed;
                info.codec = "EXR";
                _info.video.push_back(info);
            }

            Read::Read()
            {}

            Read::~Read()
            {}

            std::shared_ptr<Read> Read::create(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed,
                size_t videoQueueSize)
            {
                auto out = std::shared_ptr<Read>(new Read);
                out->_init(fileName, defaultSpeed, videoQueueSize);
                return out;
            }

            void Read::tick()
            {
                if (_hasSeek)
                {
                    _currentTime = _seekTime.rescaled_to(_info.video[0].duration.rate());
                    while (_videoQueue.size())
                    {
                        _videoQueue.pop();
                    }
                }

                if (_videoQueue.size() < _videoQueueSize)
                {
                    av::io::VideoFrame frame;

                    try
                    {
                        const std::string fileName = _getFileName(_currentTime);
                        Imf::RgbaInputFile f(fileName.c_str());

                        frame.time = _currentTime;
                        frame.image = imaging::Image::create(imfInfo(f));

                        const auto dw = f.dataWindow();
                        const int width = dw.max.x - dw.min.x + 1;
                        const int height = dw.max.y - dw.min.y + 1;
                        f.setFrameBuffer(
                            reinterpret_cast<Imf::Rgba*>(frame.image->getData()) - dw.min.x - dw.min.y * width,
                            1,
                            width);
                        f.readPixels(dw.min.y, dw.max.y);
                    }
                    catch (const std::exception&)
                    {}

                    _videoQueue.push(frame);
                    _currentTime += otime::RationalTime(1, _info.video[0].duration.rate());
                }

                _hasSeek = false;
            }

            Plugin::Plugin()
            {}
            
            std::shared_ptr<Plugin> Plugin::create()
            {
                auto out = std::shared_ptr<Plugin>(new Plugin);
                out->_init();
                return out;
            }

            bool Plugin::canRead(const std::string& fileName)
            {
                bool out = false;
                try
                {
                    Imf::RgbaInputFile(fileName.c_str());
                    out = true;
                }
                catch (const std::exception&)
                {}
                return out;
            }

            std::shared_ptr<io::IRead> Plugin::read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed)
            {
                return Read::create(fileName, defaultSpeed, _videoQueueSize);
            }
        }
    }
}
