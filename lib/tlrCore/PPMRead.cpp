// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/PPM.h>

#include <tlrCore/StringFormat.h>

namespace tlr
{
    namespace ppm
    {
        namespace
        {
            class File
            {
            public:
                File(const std::string& fileName)
                {
                    _io = file::FileIO::create();
                    _io->open(fileName, file::Mode::Read);

                    char magic[] = { 0, 0, 0 };
                    _io->read(magic, 2);
                    if (magic[0] != 'P')
                    {
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Bad magic number"));
                    }
                    switch (magic[1])
                    {
                    case '2':
                    case '3':
                    case '5':
                    case '6': break;
                    default:
                    {
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Bad magic number"));
                    }
                    }
                    const int ppmType = magic[1] - '0';
                    _data = (2 == ppmType || 3 == ppmType) ? Data::ASCII : Data::Binary;

                    size_t channelCount = 0;
                    switch (ppmType)
                    {
                    case 2:
                    case 5: channelCount = 1; break;
                    case 3:
                    case 6: channelCount = 3; break;
                    default: break;
                    }
                    char tmp[string::cBufferSize] = "";
                    file::readWord(_io, tmp, string::cBufferSize);
                    const int w = std::stoi(tmp);
                    file::readWord(_io, tmp, string::cBufferSize);
                    const int h = std::stoi(tmp);
                    file::readWord(_io, tmp, string::cBufferSize);
                    const int maxValue = std::stoi(tmp);
                    const size_t bitDepth = maxValue < 256 ? 8 : 16;
                    const auto pixelType = imaging::getIntType(channelCount, bitDepth);
                    if (imaging::PixelType::None == pixelType)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Unsupported image type"));
                    }
                    imaging::Layout layout;
                    layout.endian = _data != Data::ASCII ? memory::Endian::MSB : memory::getEndian();
                    auto imageInfo = imaging::Info(w, h, pixelType);
                    imageInfo.layout = layout;

                    const size_t ioSize = _io->getSize();
                    const size_t ioPos = _io->getPos();
                    const size_t fileDataByteCount = ioSize > 0 ? (ioSize - ioPos) : 0;
                    const size_t dataByteCount = imaging::getDataByteCount(imageInfo);
                    if (Data::Binary == _data && dataByteCount > fileDataByteCount)
                    {
                        throw std::runtime_error(string::Format("{0}: {1}").
                            arg(fileName).
                            arg("Incomplete file"));
                    }
                    _info.video.push_back(imageInfo);
                }

                Data getData() const
                {
                    return _data;
                }

                const avio::Info& getInfo() const
                {
                    return _info;
                }

                avio::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    avio::VideoData out;
                    out.time = time;
                    const auto& info = _info.video[0];
                    out.image = imaging::Image::create(info);
                    out.image->setTags(_info.tags);

                    uint8_t* p = out.image->getData();
                    switch (_data)
                    {
                    case Data::ASCII:
                    {
                        const size_t channelCount = imaging::getChannelCount(info.pixelType);
                        const size_t bitDepth = imaging::getBitDepth(info.pixelType);
                        const std::size_t scanlineByteCount = info.size.w * channelCount * (bitDepth / 8);
                        for (uint16_t y = 0; y < info.size.h; ++y, p += scanlineByteCount)
                        {
                            readASCII(_io, p, info.size.w * channelCount, bitDepth);
                        }
                        break;
                    }
                    case Data::Binary:
                    {
                        _io->read(p, out.image->getDataByteCount());
                        break;
                    }
                    default: break;
                    }

                    return out;
                }

            private:
                std::shared_ptr<file::FileIO> _io;
                Data _data = Data::First;
                avio::Info _info;
            };
        }

        void Read::_init(
            const file::Path& path,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, options, logSystem);
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
            avio::Info out = File(fileName).getInfo();
            out.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, _defaultSpeed),
                otime::RationalTime(_endFrame, _defaultSpeed));
            out.videoType = avio::VideoType::Sequence;
            return out;
        }

        avio::VideoData Read::_readVideo(
            const std::string& fileName,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            return File(fileName).read(fileName, time);
        }
    }
}
