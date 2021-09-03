// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TIFF.h>

#include <tlrCore/StringFormat.h>

#include <tiffio.h>

#include <sstream>

namespace tlr
{
    namespace tiff
    {
        namespace
        {
            void readPalette(
                uint8_t*  in,
                int       size,
                int       bytes,
                uint16_t* red,
                uint16_t* green,
                uint16_t* blue)
            {
                switch (bytes)
                {
                case 1:
                {
                    const uint8_t* inP = in + size - 1;
                    uint8_t* outP = in + (size_t(size) - 1) * 3;
                    for (int x = 0; x < size; ++x, outP -= 3)
                    {
                        const uint8_t index = *inP--;
                        outP[0] = static_cast<uint8_t>(red[index]);
                        outP[1] = static_cast<uint8_t>(green[index]);
                        outP[2] = static_cast<uint8_t>(blue[index]);
                    }
                }
                break;
                case 2:
                {
                    const uint16_t* inP = reinterpret_cast<const uint16_t*>(in) + size - 1;
                    uint16_t* outP = reinterpret_cast<uint16_t*>(in) + (size_t(size) - 1) * 3;
                    for (int x = 0; x < size; ++x, outP -= 3)
                    {
                        const uint16_t index = *inP--;
                        outP[0] = red[index];
                        outP[1] = green[index];
                        outP[2] = blue[index];
                    }
                }
                break;
                }
            }

            class File
            {
            public:
                File(const std::string& fileName)
                {
                    _f = TIFFOpen(fileName.data(), "r");
                    if (!_f)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    uint32  tiffWidth = 0;
                    uint32  tiffHeight = 0;
                    uint16  tiffPhotometric = 0;
                    uint16  tiffSamples = 0;
                    uint16  tiffSampleDepth = 0;
                    uint16  tiffSampleFormat = 0;
                    uint16* tiffExtraSamples = nullptr;
                    uint16  tiffExtraSamplesSize = 0;
                    uint16  tiffOrient = 0;
                    uint16  tiffCompression = 0;
                    uint16  tiffPlanarConfig = 0;
                    TIFFGetFieldDefaulted(_f, TIFFTAG_IMAGEWIDTH, &tiffWidth);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_IMAGELENGTH, &tiffHeight);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_PHOTOMETRIC, &tiffPhotometric);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_SAMPLESPERPIXEL, &tiffSamples);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_BITSPERSAMPLE, &tiffSampleDepth);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_SAMPLEFORMAT, &tiffSampleFormat);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_EXTRASAMPLES, &tiffExtraSamplesSize, &tiffExtraSamples);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_ORIENTATION, &tiffOrient);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_COMPRESSION, &tiffCompression);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_PLANARCONFIG, &tiffPlanarConfig);
                    TIFFGetFieldDefaulted(_f, TIFFTAG_COLORMAP, &_colormap[0], &_colormap[1], &_colormap[2]);
                    _palette = PHOTOMETRIC_PALETTE == tiffPhotometric;
                    _planar = PLANARCONFIG_SEPARATE == tiffPlanarConfig;
                    _samples = tiffSamples;
                    _sampleDepth = tiffSampleDepth;
                    _scanlineSize = tiffWidth * tiffSamples * tiffSampleDepth / 8;

                    imaging::PixelType pixelType = imaging::PixelType::None;
                    switch (tiffPhotometric)
                    {
                    case PHOTOMETRIC_PALETTE:
                        pixelType = imaging::PixelType::RGB_U8;
                        break;
                    case PHOTOMETRIC_MINISWHITE:
                    case PHOTOMETRIC_MINISBLACK:
                    case PHOTOMETRIC_RGB:
                        if (32 == tiffSampleDepth && tiffSampleFormat != SAMPLEFORMAT_IEEEFP)
                            break;
                        if (SAMPLEFORMAT_IEEEFP == tiffSampleFormat)
                        {
                            pixelType = imaging::getFloatType(tiffSamples, tiffSampleDepth);
                        }
                        else
                        {
                            pixelType = imaging::getIntType(tiffSamples, tiffSampleDepth);
                        }
                        break;
                    }
                    if (imaging::PixelType::None == pixelType)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    imaging::Info imageInfo(tiffWidth, tiffHeight, pixelType);
                    imageInfo.layout.mirror.y = true;
                    _info.video.push_back(imageInfo);

                    char* tag = 0;
                    if (TIFFGetField(_f, TIFFTAG_ARTIST, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Creator"] = tag;
                        }
                    }
                    if (TIFFGetField(_f, TIFFTAG_IMAGEDESCRIPTION, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Description"] = tag;
                        }
                    }
                    if (TIFFGetField(_f, TIFFTAG_COPYRIGHT, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Copyright"] = tag;
                        }
                    }
                    if (TIFFGetField(_f, TIFFTAG_DATETIME, &tag))
                    {
                        if (tag)
                        {
                            _info.tags["Time"] = tag;
                        }
                    }
                }

                ~File()
                {
                    if (_f)
                    {
                        TIFFClose(_f);
                    }
                }

                const avio::Info& getInfo() const
                {
                    return _info;
                }

                avio::VideoFrame read(
                    const std::string& fileName,
                    const otime::RationalTime& time,
                    const std::shared_ptr<imaging::Image>& image)
                {
                    avio::VideoFrame out;
                    out.time = time;
                    const auto& info = _info.video[0];
                    out.image = image && image->getInfo() == info ? image : imaging::Image::create(info);
                    out.image->setTags(_info.tags);

                    if (_planar)
                    {
                        std::vector<uint8_t> scanline;
                        scanline.resize(info.size.w * _sampleDepth / 8);
                        for (size_t sample = 0; sample < _samples; ++sample)
                        {
                            for (uint16_t y = 0; y < info.size.h; ++y)
                            {
                                if (TIFFReadScanline(_f, (tdata_t*)scanline.data(), y, sample) == -1)
                                {
                                    break;
                                }
                                uint8_t* p = out.image->getData() + y * _scanlineSize;
                                switch (_sampleDepth)
                                {
                                case 8:
                                {
                                    const uint8_t* inP = scanline.data();
                                    uint8_t* outP = p + sample;
                                    for (uint16_t x = 0; x < info.size.w; ++x, ++inP, outP += _samples)
                                    {
                                        *outP = *inP;
                                    }
                                    break;
                                }
                                case 16:
                                {
                                    const uint16_t* inP = reinterpret_cast<uint16_t*>(scanline.data());
                                    uint16_t* outP = reinterpret_cast<uint16_t*>(p) + sample;
                                    for (uint16_t x = 0; x < info.size.w; ++x, ++inP, outP += _samples)
                                    {
                                        *outP = *inP;
                                    }
                                    break;
                                }
                                case 32:
                                {
                                    const float* inP = reinterpret_cast<float*>(scanline.data());
                                    float* outP = reinterpret_cast<float*>(p) + sample;
                                    for (uint16_t x = 0; x < info.size.w; ++x, ++inP, outP += _samples)
                                    {
                                        *outP = *inP;
                                    }
                                    break;
                                }
                                default:
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        for (uint16_t y = 0; y < info.size.h; ++y)
                        {
                            const uint8_t* p = out.image->getData() + y * _scanlineSize;
                            if (TIFFReadScanline(_f, (tdata_t*)p, y) == -1)
                            {
                                break;
                            }
                        }
                    }

                    if (_palette)
                    {
                        for (uint16_t y = 0; y < info.size.h; ++y)
                        {
                            readPalette(
                                out.image->getData() + y * _scanlineSize,
                                info.size.w,
                                static_cast<int>(imaging::getChannelCount(info.pixelType)),
                                _colormap[0], _colormap[1], _colormap[2]);
                        }
                    }

                    return out;
                }

            private:
                TIFF*      _f = nullptr;
                bool       _palette = false;
                uint16*    _colormap[3] = { nullptr, nullptr, nullptr };
                bool       _planar = false;
                size_t     _samples = 0;
                size_t     _sampleDepth = 0;
                size_t     _scanlineSize = 0;
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
            avio::Info out = std::unique_ptr<File>(new File(fileName))->getInfo();
            out.videoTimeRange = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, _defaultSpeed),
                otime::RationalTime(_endFrame, _defaultSpeed));
            out.videoType = avio::VideoType::Sequence;
            return out;
        }

        avio::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            return std::unique_ptr<File>(new File(fileName))->read(fileName, time, image);
        }
    }
}
