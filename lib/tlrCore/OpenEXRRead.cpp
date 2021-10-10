// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/OpenEXR.h>

#include <tlrCore/FileIO.h>
#include <tlrCore/StringFormat.h>

#include <ImfRgbaFile.h>

namespace tlr
{
    namespace exr
    {
#if defined(TLR_ENABLE_MMAP)
        struct MemoryMappedIStream::Private
        {
            std::shared_ptr<file::FileIO> f;
            uint64_t                      size = 0;
            uint64_t                      pos  = 0;
            char*                         p    = nullptr;
        };

        MemoryMappedIStream::MemoryMappedIStream(const char fileName[]) :
            IStream(fileName),
            _p(new Private)
        {
            TLR_PRIVATE_P();
            p.f = file::FileIO::create();
            p.f->open(fileName, file::Mode::Read);
            p.size = p.f->getSize();
            p.p = (char*)(p.f->mmapP());
        }

        MemoryMappedIStream::~MemoryMappedIStream()
        {}

        bool MemoryMappedIStream::isMemoryMapped() const
        {
            return true;
        }

        char* MemoryMappedIStream::readMemoryMapped(int n)
        {
            TLR_PRIVATE_P();
            if (p.pos >= p.size)
            {
                throw std::runtime_error(string::Format("{0}: Error reading file").arg(p.f->getFileName()));
            }
            if (p.pos + n > p.size)
            {
                throw std::runtime_error(string::Format("{0}: Error reading file").arg(p.f->getFileName()));
            }
            char* out = p.p + p.pos;
            p.pos += n;
            return out;
        }

        bool MemoryMappedIStream::read(char c[], int n)
        {
            TLR_PRIVATE_P();
            if (p.pos >= p.size)
            {
                throw std::runtime_error(string::Format("{0}: Error reading file").arg(p.f->getFileName()));
            }
            if (p.pos + n > p.size)
            {
                throw std::runtime_error(string::Format("{0}: Error reading file").arg(p.f->getFileName()));
            }
            memcpy(c, p.p + p.pos, n);
            p.pos += n;
            return p.pos < p.size;
        }

        uint64_t MemoryMappedIStream::tellg()
        {
            return _p->pos;
        }

        void MemoryMappedIStream::seekg(uint64_t pos)
        {
            _p->pos = pos;
        }
#endif // TLR_ENABLE_MMAP

        namespace
        {
            class File
            {
            public:
                File(const std::string& fileName, ChannelGrouping channelGrouping)
                {
                    // Open the file.
#if defined(TLR_ENABLE_MMAP)
                    _s.reset(new MemoryMappedIStream(fileName.c_str()));
                    _f.reset(new Imf::InputFile(*_s.get()));
#else // TLR_ENABLE_MMAP
                    _f.reset(new Imf::InputFile(fileName.c_str()));
#endif // TLR_ENABLE_MMAP

                    // Get the display and data windows.
                    _displayWindow = fromImath(_f->header().displayWindow());
                    _dataWindow = fromImath(_f->header().dataWindow());
                    _intersectedWindow = _displayWindow.intersect(_dataWindow);
                    _fast = _displayWindow == _dataWindow;

                    // Get the tags.
                    readTags(_f->header(), _info.tags);

                    // Get the layers.
                    _layers = getLayers(_f->header().channels(), channelGrouping);
                    _info.video.resize(_layers.size());
                    for (size_t i = 0; i < _layers.size(); ++i)
                    {
                        const auto& layer = _layers[i];
                        const math::Vector2i sampling(layer.channels[0].sampling.x, layer.channels[0].sampling.y);
                        if (sampling.x != 1 || sampling.y != 1)
                            _fast = false;
                        auto& info = _info.video[i];
                        info.name = layer.name;
                        info.size.w = _displayWindow.w();
                        info.size.h = _displayWindow.h();
                        info.pixelAspectRatio = _f->header().pixelAspectRatio();
                        switch (layer.channels[0].pixelType)
                        {
                        case Imf::PixelType::HALF:
                            info.pixelType = imaging::getFloatType(layer.channels.size(), 16);
                            break;
                        case Imf::PixelType::FLOAT:
                            info.pixelType = imaging::getFloatType(layer.channels.size(), 32);
                            break;
                        case Imf::PixelType::UINT:
                            info.pixelType = imaging::getIntType(layer.channels.size(), 32);
                            break;
                        default: break;
                        }
                        if (imaging::PixelType::None == info.pixelType)
                        {
                            throw std::runtime_error(string::Format("{0}: Unsupported image type").arg(fileName));
                        }
                        info.layout.mirror.y = true;
                    }
                }

                const avio::Info& getInfo() const
                {
                    return _info;
                }

                avio::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time,
                    uint16_t layer)
                {
                    avio::VideoData out;
                    imaging::Info imageInfo = _info.video[std::min(static_cast<size_t>(layer), _info.video.size() - 1)];
                    out.image = imaging::Image::create(imageInfo);
                    out.image->setTags(_info.tags);
                    const size_t channels = imaging::getChannelCount(imageInfo.pixelType);
                    const size_t channelByteCount = imaging::getBitDepth(imageInfo.pixelType) / 8;
                    const size_t cb = channels * channelByteCount;
                    const size_t scb = imageInfo.size.w * channels * channelByteCount;
                    if (_fast)
                    {
                        Imf::FrameBuffer frameBuffer;
                        for (size_t c = 0; c < channels; ++c)
                        {
                            const std::string& name = _layers[layer].channels[c].name;
                            const math::Vector2i& sampling = _layers[layer].channels[c].sampling;
                            frameBuffer.insert(
                                name.c_str(),
                                Imf::Slice(
                                    _layers[layer].channels[c].pixelType,
                                    (char*)out.image->getData() + (c * channelByteCount),
                                    cb,
                                    scb,
                                    sampling.x,
                                    sampling.y,
                                    0.F));
                        }
                        _f->setFrameBuffer(frameBuffer);
                        _f->readPixels(_displayWindow.min.y, _displayWindow.max.y);
                    }
                    else
                    {
                        Imf::FrameBuffer frameBuffer;
                        std::vector<char> buf(_dataWindow.w() * cb);
                        for (int c = 0; c < channels; ++c)
                        {
                            const std::string& name = _layers[layer].channels[c].name;
                            const math::Vector2i& sampling = _layers[layer].channels[c].sampling;
                            frameBuffer.insert(
                                name.c_str(),
                                Imf::Slice(
                                    _layers[layer].channels[c].pixelType,
                                    buf.data() - (_dataWindow.min.x * cb) + (c * channelByteCount),
                                    cb,
                                    0,
                                    sampling.x,
                                    sampling.y,
                                    0.F));
                        }
                        _f->setFrameBuffer(frameBuffer);
                        for (int y = _displayWindow.min.y; y <= _displayWindow.max.y; ++y)
                        {
                            uint8_t* p = out.image->getData() + ((y - _displayWindow.min.y) * scb);
                            uint8_t* end = p + scb;
                            if (y >= _intersectedWindow.min.y && y <= _intersectedWindow.max.y)
                            {
                                size_t size = (_intersectedWindow.min.x - _displayWindow.min.x) * cb;
                                memset(p, 0, size);
                                p += size;
                                size = _intersectedWindow.w() * cb;
                                _f->readPixels(y, y);
                                memcpy(
                                    p,
                                    buf.data() + std::max(_displayWindow.min.x - _dataWindow.min.x, 0) * cb,
                                    size);
                                p += size;
                            }
                            memset(p, 0, end - p);
                        }
                    }
                    return out;
                }

            private:
                ChannelGrouping                      _channelGrouping = ChannelGrouping::Known;
                std::unique_ptr<MemoryMappedIStream> _s;
                std::unique_ptr<Imf::InputFile>      _f;
                math::BBox2i                         _displayWindow;
                math::BBox2i                         _dataWindow;
                math::BBox2i                         _intersectedWindow;
                std::vector<Layer>                   _layers;
                bool                                 _fast              = false;
                avio::Info                           _info;
            };
        }

        void Read::_init(
            const file::Path& path,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, options, logSystem);

            auto option = options.find("exr/ChannelGrouping");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> _channelGrouping;
            }
        }

        Read::Read()
        {}

        Read::~Read()
        {
            _finish();
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, options, logSystem);
            return out;
        }

        avio::Info Read::_getInfo(const std::string& fileName)
        {
            avio::Info out = File(fileName, _channelGrouping).getInfo();
            float speed = _defaultSpeed;
            const auto i = out.tags.find("Frame Per Second");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            out.videoTimeRange = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, speed),
                otime::RationalTime(_endFrame, speed));
            out.videoType = avio::VideoType::Sequence;
            return out;
        }

        avio::VideoData Read::_readVideo(
            const std::string& fileName,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            return File(fileName, _channelGrouping).read(fileName, time, layer);
        }
    }
}
