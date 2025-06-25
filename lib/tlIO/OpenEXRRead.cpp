// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/OpenEXRPrivate.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/LogSystem.h>

#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfInputPart.h>
#include <ImfMultiPartInputFile.h>

#include <array>
#include <cstring>

namespace tl
{
    namespace exr
    {
        struct IStream::Private
        {
            std::shared_ptr<feather_tk::FileIO> f;
            const uint8_t* p = nullptr;
            uint64_t size = 0;
            uint64_t pos = 0;
        };

        IStream::IStream(const std::string& fileName) :
            Imf::IStream(fileName.c_str()),
            _p(new Private)
        {
            FEATHER_TK_P();
            p.f = feather_tk::FileIO::create(fileName, feather_tk::FileMode::Read);
            p.p = p.f->getMemoryP();
            p.size = p.f->getSize();
        }

        IStream::IStream(const std::string& fileName, const uint8_t* memoryP, size_t memorySize) :
            Imf::IStream(fileName.c_str()),
            _p(new Private)
        {
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            if (p.pos >= p.size || (p.pos + n) > p.size)
            {
                throw std::runtime_error(feather_tk::Format("Error reading file: \"{0}\"").arg(fileName()));
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
            FEATHER_TK_P();
            if (p.pos >= p.size || (p.pos + n) > p.size)
            {
                throw std::runtime_error(feather_tk::Format("Error reading file: \"{0}\"").arg(fileName()));
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
            FEATHER_TK_P();
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
                    const feather_tk::InMemoryFile* memory,
                    const std::shared_ptr<feather_tk::LogSystem>& logSystem)
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
                    _f.reset(new Imf::MultiPartInputFile(*_s));

                    {
                        const std::string id = feather_tk::Format("tl::io::exr::Read {0}").arg(this);
                        logSystem->print(id, feather_tk::Format("file name: {0}").arg(fileName));
                    }

                    // Get the tags.
                    const int partsCount = _f->parts();
                    if (partsCount > 0)
                    {
                        readTags(_f->header(0), _info.tags);
                    }

                    // Get the layers.
                    for (int part = 0; part < partsCount; ++part)
                    {
                        std::string view;
                        const Imf::Header& imfHeader = _f->header(part);
                        if (_f->header(part).hasView())
                        {
                            view = imfHeader.view();
                            if (!view.empty() && view[0] != '.')
                            {
                                view.insert(view.begin(), '.');
                            }
                        }

                        const feather_tk::Box2I displayWindow = fromImath(imfHeader.displayWindow());
                        const auto& imfChannels = imfHeader.channels();
                        std::set<std::string> imfChannelNames;
                        for (auto i = imfChannels.begin(); i != imfChannels.end(); ++i)
                        {
                            imfChannelNames.insert(i.name());
                        }
                        std::set<std::string> imfDefaultChannelNames = getDefaultChannels(imfChannelNames);

                        // Add RGB and RGBA layers.
                        auto r = imfDefaultChannelNames.find("r");
                        if (r == imfDefaultChannelNames.end())
                        {
                            r = imfDefaultChannelNames.find("R");
                        }
                        auto g = imfDefaultChannelNames.find("g");
                        if (g == imfDefaultChannelNames.end())
                        {
                            g = imfDefaultChannelNames.find("G");
                        }
                        auto b = imfDefaultChannelNames.find("b");
                        if (b == imfDefaultChannelNames.end())
                        {
                            b = imfDefaultChannelNames.find("B");
                        }
                        auto a = imfDefaultChannelNames.find("a");
                        if (a == imfDefaultChannelNames.end())
                        {
                            a = imfDefaultChannelNames.find("A");
                        }
                        if (r != imfDefaultChannelNames.end() &&
                            g != imfDefaultChannelNames.end() &&
                            b != imfDefaultChannelNames.end() &&
                            a != imfDefaultChannelNames.end())
                        {
                            const Imf::PixelType imfPixelType = imfChannels[*r].type;
                            if (imfPixelType == imfChannels[*g].type &&
                                imfPixelType == imfChannels[*b].type &&
                                imfPixelType == imfChannels[*a].type &&
                                1 == imfChannels[*r].xSampling &&
                                1 == imfChannels[*r].ySampling &&
                                1 == imfChannels[*g].xSampling &&
                                1 == imfChannels[*g].ySampling &&
                                1 == imfChannels[*b].xSampling &&
                                1 == imfChannels[*b].ySampling &&
                                1 == imfChannels[*a].xSampling &&
                                1 == imfChannels[*a].ySampling)
                            {
                                feather_tk::ImageInfo info;
                                info.name = "RGBA" + view;
                                info.size.w = displayWindow.w();
                                info.size.h = displayWindow.h();
                                info.pixelAspectRatio = imfHeader.pixelAspectRatio();
                                info.layout.mirror.y = true;
                                switch (imfPixelType)
                                {
                                case Imf::PixelType::HALF:  info.type = feather_tk::ImageType::RGBA_F16; break;
                                case Imf::PixelType::FLOAT: info.type = feather_tk::ImageType::RGBA_F32; break;
                                case Imf::PixelType::UINT:  info.type = feather_tk::ImageType::RGBA_U32; break;
                                default: break;
                                }
                                if (info.type != feather_tk::ImageType::None)
                                {
                                    _info.video.push_back(info);
                                    Layer layer;
                                    layer.part = part;
                                    layer.channels.push_back(*r);
                                    layer.channels.push_back(*g);
                                    layer.channels.push_back(*b);
                                    layer.channels.push_back(*a);
                                    layer.pixelType = imfPixelType;
                                    _layers.push_back(layer);
                                    imfDefaultChannelNames.erase(r);
                                    imfDefaultChannelNames.erase(g);
                                    imfDefaultChannelNames.erase(b);
                                    imfDefaultChannelNames.erase(a);
                                }
                            }
                        }
                        else if (r != imfDefaultChannelNames.end() &&
                            g != imfDefaultChannelNames.end() &&
                            b != imfDefaultChannelNames.end())
                        {
                            const Imf::PixelType imfPixelType = imfChannels[*r].type;
                            if (imfPixelType == imfChannels[*g].type &&
                                imfPixelType == imfChannels[*b].type &&
                                1 == imfChannels[*r].xSampling &&
                                1 == imfChannels[*r].ySampling &&
                                1 == imfChannels[*g].xSampling &&
                                1 == imfChannels[*g].ySampling &&
                                1 == imfChannels[*b].xSampling &&
                                1 == imfChannels[*b].ySampling)
                            {
                                feather_tk::ImageInfo info;
                                switch (imfPixelType)
                                {
                                case Imf::PixelType::HALF:  info.type = feather_tk::ImageType::RGB_F16; break;
                                case Imf::PixelType::FLOAT: info.type = feather_tk::ImageType::RGB_F32; break;
                                case Imf::PixelType::UINT:  info.type = feather_tk::ImageType::RGB_U32; break;
                                default: break;
                                }
                                if (info.type != feather_tk::ImageType::None)
                                {
                                    info.name = "RGB" + view;
                                    info.size.w = displayWindow.w();
                                    info.size.h = displayWindow.h();
                                    info.pixelAspectRatio = imfHeader.pixelAspectRatio();
                                    info.layout.mirror.y = true;
                                    _info.video.push_back(info);
                                    Layer layer;
                                    layer.part = part;
                                    layer.channels.push_back(*r);
                                    layer.channels.push_back(*g);
                                    layer.channels.push_back(*b);
                                    layer.pixelType = imfPixelType;
                                    _layers.push_back(layer);
                                    imfDefaultChannelNames.erase(r);
                                    imfDefaultChannelNames.erase(g);
                                    imfDefaultChannelNames.erase(b);
                                }
                            }
                        }

                        // Add remaining default layers.
                        for (const auto& imfChannelName : imfDefaultChannelNames)
                        {
                            feather_tk::ImageInfo info;
                            const Imf::PixelType imfPixelType = imfChannels[imfChannelName].type;
                            switch (imfPixelType)
                            {
                            case Imf::PixelType::HALF:  info.type = feather_tk::ImageType::L_F16; break;
                            case Imf::PixelType::FLOAT: info.type = feather_tk::ImageType::L_F32; break;
                            case Imf::PixelType::UINT:  info.type = feather_tk::ImageType::L_U32; break;
                            default: break;
                            }
                            if (info.type != feather_tk::ImageType::None &&
                                1 == imfChannels[imfChannelName].xSampling &&
                                1 == imfChannels[imfChannelName].ySampling)
                            {
                                info.name = imfChannelName + view;
                                info.size.w = displayWindow.w();
                                info.size.h = displayWindow.h();
                                info.pixelAspectRatio = imfHeader.pixelAspectRatio();
                                info.layout.mirror.y = true;
                                _info.video.push_back(info);
                                Layer layer;
                                layer.part = part;
                                layer.channels.push_back(imfChannelName);
                                layer.pixelType = imfPixelType;
                                _layers.push_back(layer);
                            }
                        }

                        // Add OpenEXR layers.
                        std::set<std::string> imfLayerNames;
                        imfChannels.layers(imfLayerNames);
                        for (const auto& i : imfLayerNames)
                        {
                            std::vector<std::string> imfHalfNames;
                            std::vector<std::string> imfFloatNames;
                            std::vector<std::string> imfUIntNames;
                            Imf::ChannelList::ConstIterator j0;
                            Imf::ChannelList::ConstIterator j1;
                            imfChannels.channelsInLayer(i, j0, j1);
                            for (auto j = j0; j != j1; ++j)
                            {
                                if (1 == j.channel().xSampling &&
                                    1 == j.channel().ySampling)
                                {
                                    switch (j.channel().type)
                                    {
                                    case Imf::PixelType::HALF:  imfHalfNames.push_back(j.name());  break;
                                    case Imf::PixelType::FLOAT: imfFloatNames.push_back(j.name()); break;
                                    case Imf::PixelType::UINT:  imfUIntNames.push_back(j.name());  break;
                                    default: break;
                                    }
                                }
                            }
                            feather_tk::ImageInfo info;
                            Layer layer;
                            if (imfHalfNames.size() > 0 && imfHalfNames.size() <= 4)
                            {
                                info.type = io::getFloatType(imfHalfNames.size(), 16);
                                reorderChannels(imfHalfNames);
                                layer.channels.insert(layer.channels.end(), imfHalfNames.begin(), imfHalfNames.end());
                                layer.pixelType = Imf::PixelType::HALF;
                            }
                            else if (imfFloatNames.size() > 0 && imfFloatNames.size() <= 4)
                            {
                                info.type = io::getFloatType(imfFloatNames.size(), 32);
                                reorderChannels(imfFloatNames);
                                layer.channels.insert(layer.channels.end(), imfFloatNames.begin(), imfFloatNames.end());
                                layer.pixelType = Imf::PixelType::FLOAT;
                            }
                            else if (imfUIntNames.size() > 0 && imfUIntNames.size() <= 4)
                            {
                                info.type = io::getIntType(imfUIntNames.size(), 16);
                                reorderChannels(imfUIntNames);
                                layer.channels.insert(layer.channels.end(), imfUIntNames.begin(), imfUIntNames.end());
                                layer.pixelType = Imf::PixelType::UINT;
                            }
                            if (info.type != feather_tk::ImageType::None)
                            {
                                info.name = i + view;
                                info.size.w = displayWindow.w();
                                info.size.h = displayWindow.h();
                                info.pixelAspectRatio = imfHeader.pixelAspectRatio();
                                info.layout.mirror.y = true;
                                _info.video.push_back(info);
                                layer.part = part;
                                _layers.push_back(layer);
                            }
                        }
                    }
                    if (_info.video.empty())
                    {
                        throw std::runtime_error(feather_tk::Format("Unsupported image type: \"{0}\"").arg(fileName));
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
                    if (layer >= 0 && layer < _info.video.size() && layer < _layers.size())
                    {
                        Imf::InputPart imfPart(*_f, _layers[layer].part);
                        const Imf::Header& imfHeader = _f->header(_layers[layer].part);
                        const feather_tk::Box2I displayWindow = fromImath(imfHeader.displayWindow());
                        const feather_tk::Box2I dataWindow = fromImath(imfHeader.dataWindow());
                        const feather_tk::Box2I intersectedWindow = feather_tk::intersect(displayWindow, dataWindow);
                        const bool fast = displayWindow == dataWindow;

                        const feather_tk::ImageInfo& imageInfo = _info.video[layer];
                        out.image = feather_tk::Image::create(imageInfo);
                        out.image->setTags(_info.tags);
                        const size_t channels = feather_tk::getChannelCount(imageInfo.type);
                        const size_t channelByteCount = feather_tk::getBitDepth(imageInfo.type) / 8;
                        const size_t cb = channels * channelByteCount;
                        const size_t scb = imageInfo.size.w * channels * channelByteCount;
                        if (fast)
                        {
                            Imf::FrameBuffer frameBuffer;
                            for (size_t c = 0; c < channels; ++c)
                            {
                                const feather_tk::V2I sampling(1, 1);
                                frameBuffer.insert(
                                    _layers[layer].channels[c],
                                    Imf::Slice(
                                        _layers[layer].pixelType,
                                        reinterpret_cast<char*>(out.image->getData()) + (c * channelByteCount),
                                        cb,
                                        scb,
                                        sampling.x,
                                        sampling.y,
                                        0.F));
                            }
                            imfPart.setFrameBuffer(frameBuffer);
                            imfPart.readPixels(displayWindow.min.y, displayWindow.max.y);
                        }
                        else
                        {
                            Imf::FrameBuffer frameBuffer;
                            std::vector<char> buf(dataWindow.w() * cb);
                            for (int c = 0; c < channels; ++c)
                            {
                                const feather_tk::V2I sampling(1, 1);
                                frameBuffer.insert(
                                    _layers[layer].channels[c],
                                    Imf::Slice(
                                        _layers[layer].pixelType,
                                        buf.data() - (dataWindow.min.x * cb) + (c * channelByteCount),
                                        cb,
                                        0,
                                        sampling.x,
                                        sampling.y,
                                        0.F));
                            }
                            imfPart.setFrameBuffer(frameBuffer);
                            for (int y = displayWindow.min.y; y <= displayWindow.max.y; ++y)
                            {
                                uint8_t* p = out.image->getData() + ((y - displayWindow.min.y) * scb);
                                uint8_t* end = p + scb;
                                if (y >= intersectedWindow.min.y && y <= intersectedWindow.max.y)
                                {
                                    size_t size = (intersectedWindow.min.x - displayWindow.min.x) * cb;
                                    std::memset(p, 0, size);
                                    p += size;
                                    size = intersectedWindow.w() * cb;
                                    imfPart.readPixels(y, y);
                                    std::memcpy(
                                        p,
                                        buf.data() + std::max(displayWindow.min.x - dataWindow.min.x, 0) * cb,
                                        size);
                                    p += size;
                                }
                                std::memset(p, 0, end - p);
                            }
                        }
                    }
                    return out;
                }

            private:
                std::unique_ptr<Imf::IStream> _s;
                std::unique_ptr<Imf::MultiPartInputFile> _f;
                io::Info _info;

                struct Layer
                {
                    int part = 0;
                    std::vector<std::string> channels;
                    Imf::PixelType pixelType = Imf::PixelType::HALF;
                };
                std::vector<Layer> _layers;
            };
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, logSystem);
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
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const feather_tk::InMemoryFile* memory)
        {
            io::Info out = File(fileName, memory, _logSystem.lock()).getInfo();
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
            const feather_tk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            return File(fileName, memory, _logSystem.lock()).read(fileName, time, options);
        }
    }
}
