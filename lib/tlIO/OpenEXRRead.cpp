// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/OpenEXRPrivate.h>

#include <dtk/core/Format.h>
#include <dtk/core/LogSystem.h>

#include <ImfChannelList.h>
#include <ImfRgbaFile.h>

#include <array>
#include <cstring>

namespace tl
{
    namespace exr
    {
        struct IStream::Private
        {
            std::shared_ptr<dtk::FileIO> f;
            const uint8_t* p = nullptr;
            uint64_t size = 0;
            uint64_t pos = 0;
        };

        IStream::IStream(const std::string& fileName) :
            Imf::IStream(fileName.c_str()),
            _p(new Private)
        {
            DTK_P();
            p.f = dtk::FileIO::create(fileName, dtk::FileMode::Read);
            p.p = p.f->getMemoryP();
            p.size = p.f->getSize();
        }

        IStream::IStream(const std::string& fileName, const uint8_t* memoryP, size_t memorySize) :
            Imf::IStream(fileName.c_str()),
            _p(new Private)
        {
            DTK_P();
            p.p = memoryP;
            p.size = memorySize;
        }

        IStream::~IStream()
        {}

        bool IStream::isMemoryMapped() const
        {
            return _p->p;
        }

        char* IStream::readMemoryMapped(int n)
        {
            DTK_P();
            if (p.pos >= p.size || (p.pos + n) > p.size)
            {
                throw std::runtime_error(dtk::Format("Error reading file: \"{0}\"").arg(fileName()));
            }
            char* out = nullptr;
            if (p.p)
            {
                out = const_cast<char*>(reinterpret_cast<const char*>(p.p)) + p.pos;
                p.pos += n;
            }
            return out;
        }

        bool IStream::read(char c[], int n)
        {
            DTK_P();
            if (p.pos >= p.size || (p.pos + n) > p.size)
            {
                throw std::runtime_error(dtk::Format("Error reading file: \"{0}\"").arg(fileName()));
            }
            if (p.p)
            {
                std::memcpy(c, p.p + p.pos, n);
            }
            else
            {
                p.f->read(c, n);
            }
            p.pos += n;
            return p.pos < p.size;
        }

        uint64_t IStream::tellg()
        {
            return _p->pos;
        }

        void IStream::seekg(uint64_t pos)
        {
            DTK_P();
            if (p.f)
            {
                p.f->setPos(pos);
            }
            p.pos = pos;
        }

        namespace
        {
            std::string getLabel(Imf::PixelType value)
            {
                const std::array<std::string, 3> data =
                {
                    "UInt",
                    "Half",
                    "Float"
                };
                return data[value];
            }

            std::string getLabel(Imf::Compression value)
            {
                const std::array<std::string, static_cast<size_t>(Imf::Compression::NUM_COMPRESSION_METHODS) > data =
                {
                    "None",
                    "RLE",
                    "ZIPS",
                    "ZIP",
                    "PIZ",
                    "PXR24",
                    "B44",
                    "B44A",
                    "DWAA",
                    "DWAB"
                };
                return data[value];
            }

            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const dtk::InMemoryFile* memory,
                    ChannelGrouping channelGrouping,
                    const std::shared_ptr<dtk::LogSystem>& logSystem)
                {
                    // Open the file.
                    if (memory)
                    {
                        _s.reset(new IStream(fileName, memory->p, memory->size));
                    }
                    else
                    {
                        _s.reset(new IStream(fileName));
                    }
                    _f.reset(new Imf::InputFile(*_s));

                    // Get the display and data windows.
                    _displayWindow = fromImath(_f->header().displayWindow());
                    _dataWindow = fromImath(_f->header().dataWindow());
                    _intersectedWindow = dtk::intersect(_displayWindow, _dataWindow);
                    _fast = _displayWindow == _dataWindow;

                    {
                        const std::string id = dtk::Format("tl::io::exr::Read {0}").arg(this);
                        std::vector<std::string> s;
                        s.push_back(dtk::Format(
                            "\n"
                            "    file name: {0}\n"
                            "    display window {1}\n"
                            "    data window: {2}\n"
                            "    compression: {3}").
                            arg(fileName).
                            arg(_displayWindow).
                            arg(_dataWindow).
                            arg(getLabel(_f->header().compression())));
                        const auto& channels = _f->header().channels();
                        for (auto i = channels.begin(); i != channels.end(); ++i)
                        {
                            std::stringstream ss2;
                            ss2 << "    channel " << i.name() << ": " << getLabel(i.channel().type) << ", " << i.channel().xSampling << "x" << i.channel().ySampling;
                            s.push_back(ss2.str());
                        }
                        logSystem->print(id, dtk::join(s, '\n'));
                    }

                    // Get the tags.
                    readTags(_f->header(), _info.tags);

                    // Get the layers.
                    _layers = getLayers(_f->header().channels(), channelGrouping);
                    _info.video.resize(_layers.size());
                    for (size_t i = 0; i < _layers.size(); ++i)
                    {
                        const auto& layer = _layers[i];
                        const dtk::V2I sampling(layer.channels[0].sampling.x, layer.channels[0].sampling.y);
                        if (sampling.x != 1 || sampling.y != 1)
                        {
                            _fast = false;
                        }
                        auto& info = _info.video[i];
                        info.name = layer.name;
                        info.size.w = _displayWindow.w();
                        info.size.h = _displayWindow.h();
                        info.pixelAspectRatio = _f->header().pixelAspectRatio();
                        switch (layer.channels[0].pixelType)
                        {
                        case Imf::PixelType::HALF:
                            info.type = io::getFloatType(layer.channels.size(), 16);
                            break;
                        case Imf::PixelType::FLOAT:
                            info.type = io::getFloatType(layer.channels.size(), 32);
                            break;
                        case Imf::PixelType::UINT:
                            info.type = io::getIntType(layer.channels.size(), 32);
                            break;
                        default: break;
                        }
                        if (dtk::ImageType::None == info.type)
                        {
                            throw std::runtime_error(dtk::Format("Unsupported image type: \"{0}\"").arg(fileName));
                        }
                        info.layout.mirror.y = true;
                    }
                }

                const io::Info& getInfo() const
                {
                    return _info;
                }

                io::VideoData read(
                    const std::string& fileName,
                    const OTIO_NS::RationalTime& time,
                    const io::Options& options)
                {
                    io::VideoData out;
                    int layer = 0;
                    const auto i = options.find("Layer");
                    if (i != options.end())
                    {
                        layer = std::min(
                            std::atoi(i->second.c_str()),
                            static_cast<int>(_info.video.size()) - 1);
                    }
                    dtk::ImageInfo imageInfo = _info.video[layer];
                    out.image = dtk::Image::create(imageInfo);
                    out.image->setTags(_info.tags);
                    const size_t channels = dtk::getChannelCount(imageInfo.type);
                    const size_t channelByteCount = dtk::getBitDepth(imageInfo.type) / 8;
                    const size_t cb = channels * channelByteCount;
                    const size_t scb = imageInfo.size.w * channels * channelByteCount;
                    if (_fast)
                    {
                        Imf::FrameBuffer frameBuffer;
                        for (size_t c = 0; c < channels; ++c)
                        {
                            const std::string& name = _layers[layer].channels[c].name;
                            const dtk::V2I& sampling = _layers[layer].channels[c].sampling;
                            frameBuffer.insert(
                                name.c_str(),
                                Imf::Slice(
                                    _layers[layer].channels[c].pixelType,
                                    reinterpret_cast<char*>(out.image->getData()) + (c * channelByteCount),
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
                            const dtk::V2I& sampling = _layers[layer].channels[c].sampling;
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
                                std::memset(p, 0, size);
                                p += size;
                                size = _intersectedWindow.w() * cb;
                                _f->readPixels(y, y);
                                std::memcpy(
                                    p,
                                    buf.data() + std::max(_displayWindow.min.x - _dataWindow.min.x, 0) * cb,
                                    size);
                                p += size;
                            }
                            std::memset(p, 0, end - p);
                        }
                    }
                    return out;
                }

            private:
                ChannelGrouping                 _channelGrouping = ChannelGrouping::Known;
                std::unique_ptr<Imf::IStream>   _s;
                std::unique_ptr<Imf::InputFile> _f;
                dtk::Box2I                    _displayWindow;
                dtk::Box2I                    _dataWindow;
                dtk::Box2I                    _intersectedWindow;
                std::vector<Layer>              _layers;
                bool                            _fast = false;
                io::Info                        _info;
            };
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, logSystem);

            auto option = options.find("OpenEXR/ChannelGrouping");
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
            const io::Options& options,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const dtk::InMemoryFile* memory)
        {
            io::Info out = File(fileName, memory, _channelGrouping, _logSystem.lock()).getInfo();
            float speed = _defaultSpeed;
            const auto i = out.tags.find("Frame Per Second");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, speed),
                OTIO_NS::RationalTime(_endFrame, speed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const dtk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            return File(fileName, memory, _channelGrouping, _logSystem.lock()).read(fileName, time, options);
        }
    }
}
