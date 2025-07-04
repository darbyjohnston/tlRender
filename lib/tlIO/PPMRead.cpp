// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/PPM.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

namespace tl
{
    namespace ppm
    {
        namespace
        {
            class File
            {
            public:
                File(const std::string& fileName, const feather_tk::InMemoryFile* memory)
                {
                    _io = memory ?
                        feather_tk::FileIO::create(fileName, *memory) :
                        feather_tk::FileIO::create(fileName, feather_tk::FileMode::Read);

                    char magic[] = { 0, 0, 0 };
                    _io->read(magic, 2);
                    if (magic[0] != 'P')
                    {
                        throw std::runtime_error(feather_tk::Format("Bad magic number: \"{0}\"").
                            arg(fileName));
                    }
                    switch (magic[1])
                    {
                    case '2':
                    case '3':
                    case '5':
                    case '6': break;
                    default:
                    {
                        throw std::runtime_error(feather_tk::Format("Bad magic number: \"{0}\"").
                            arg(fileName));
                    }
                    }
                    const int ppmType = magic[1] - '0';
                    _data = (2 == ppmType || 3 == ppmType) ? Data::ASCII : Data::Binary;

                    char tmp[feather_tk::cStringSize] = "";
                    feather_tk::readWord(_io, tmp, feather_tk::cStringSize);
                    const int w = std::stoi(tmp);
                    feather_tk::readWord(_io, tmp, feather_tk::cStringSize);
                    const int h = std::stoi(tmp);
                    _info.size.w = w;
                    _info.size.h = h;

                    feather_tk::readWord(_io, tmp, feather_tk::cStringSize);
                    const int maxValue = std::stoi(tmp);
                    size_t channelCount = 0;
                    switch (ppmType)
                    {
                    case 2:
                    case 5: channelCount = 1; break;
                    case 3:
                    case 6: channelCount = 3; break;
                    default: break;
                    }
                    const size_t bitDepth = maxValue < 256 ? 8 : 16;
                    _info.type = io::getIntType(channelCount, bitDepth);
                    if (feather_tk::ImageType::None == _info.type)
                    {
                        throw std::runtime_error(feather_tk::Format("Unsupported image type: \"{0}\"").
                            arg(fileName));
                    }

                    const size_t ioSize = _io->getSize();
                    const size_t ioPos = _io->getPos();
                    const size_t fileDataByteCount = ioSize >= ioPos ? (ioSize - ioPos) : 0;
                    const size_t dataByteCount = _info.getByteCount();
                    if (Data::Binary == _data && dataByteCount > fileDataByteCount)
                    {
                        throw std::runtime_error(feather_tk::Format("Incomplete file: \"{0}\"").
                            arg(fileName));
                    }

                    _info.layout.endian = _data != Data::ASCII ? feather_tk::Endian::MSB : feather_tk::getEndian();
                }

                Data getData() const
                {
                    return _data;
                }

                const feather_tk::ImageInfo& getInfo() const
                {
                    return _info;
                }

                io::VideoData read(
                    const std::string& fileName,
                    const OTIO_NS::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    out.image = feather_tk::Image::create(_info);

                    uint8_t* p = out.image->getData();
                    switch (_data)
                    {
                    case Data::ASCII:
                    {
                        const size_t channelCount = feather_tk::getChannelCount(_info.type);
                        const size_t bitDepth = feather_tk::getBitDepth(_info.type);
                        const std::size_t scanlineByteCount = _info.size.w * channelCount * (bitDepth / 8);
                        for (uint16_t y = 0; y < _info.size.h; ++y, p += scanlineByteCount)
                        {
                            readASCII(_io, p, _info.size.w * channelCount, bitDepth);
                        }
                        break;
                    }
                    case Data::Binary:
                    {
                        _io->read(p, out.image->getByteCount());
                        break;
                    }
                    default: break;
                    }

                    return out;
                }

            private:
                std::shared_ptr<feather_tk::FileIO> _io;
                Data _data = Data::First;
                feather_tk::ImageInfo _info;
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
            io::Info out;
            out.video.push_back(File(fileName, memory).getInfo());
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, _defaultSpeed),
                OTIO_NS::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const feather_tk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            return File(fileName, memory).read(fileName, time);
        }
    }
}
