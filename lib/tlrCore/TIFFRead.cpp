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
            class File
            {
            public:
                File(const std::string& fileName)
                {
                    f = TIFFOpen(fileName.data(), "r");
                    if (!f)
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
                    TIFFGetFieldDefaulted(f, TIFFTAG_IMAGEWIDTH, &tiffWidth);
                    TIFFGetFieldDefaulted(f, TIFFTAG_IMAGELENGTH, &tiffHeight);
                    TIFFGetFieldDefaulted(f, TIFFTAG_PHOTOMETRIC, &tiffPhotometric);
                    TIFFGetFieldDefaulted(f, TIFFTAG_SAMPLESPERPIXEL, &tiffSamples);
                    TIFFGetFieldDefaulted(f, TIFFTAG_BITSPERSAMPLE, &tiffSampleDepth);
                    TIFFGetFieldDefaulted(f, TIFFTAG_SAMPLEFORMAT, &tiffSampleFormat);
                    TIFFGetFieldDefaulted(f, TIFFTAG_EXTRASAMPLES, &tiffExtraSamplesSize, &tiffExtraSamples);
                    TIFFGetFieldDefaulted(f, TIFFTAG_ORIENTATION, &tiffOrient);
                    TIFFGetFieldDefaulted(f, TIFFTAG_COMPRESSION, &tiffCompression);
                    TIFFGetFieldDefaulted(f, TIFFTAG_PLANARCONFIG, &tiffPlanarConfig);
                    TIFFGetFieldDefaulted(f, TIFFTAG_COLORMAP, &colormap[0], &colormap[1], &colormap[2]);
                    palette = PHOTOMETRIC_PALETTE == tiffPhotometric;
                    planar = PLANARCONFIG_SEPARATE == tiffPlanarConfig;
                    samples = tiffSamples;
                    sampleDepth = tiffSampleDepth;
                    scanlineSize = tiffWidth * tiffSamples * tiffSampleDepth / 8;

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

                    info = imaging::Info(tiffWidth, tiffHeight, pixelType);
                }

                ~File()
                {
                    if (f)
                    {
                        TIFFClose(f);
                        f = nullptr;
                    }
                }

                TIFF*         f = nullptr;
                bool          palette = false;
                uint16*       colormap[3] = { nullptr, nullptr, nullptr };
                bool          planar = false;
                size_t        samples = 0;
                size_t        sampleDepth = 0;
                size_t        scanlineSize = 0;
                imaging::Info info;
            };

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
        }

        void Read::_init(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            ISequenceRead::_init(fileName, defaultSpeed);
        }

        Read::Read()
        {}

        Read::~Read()
        {}

        std::shared_ptr<Read> Read::create(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, defaultSpeed);
            return out;
        }

        io::Info Read::_getInfo(const std::string& fileName)
        {
            io::Info out;
            io::VideoInfo videoInfo;
            videoInfo.info = std::shared_ptr<File>(new File(fileName))->info;
            videoInfo.duration = _defaultSpeed;
            videoInfo.codec = "TIFF";
            out.video.push_back(videoInfo);
            return out;
        }

        io::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            io::VideoFrame out;

            auto f = std::shared_ptr<File>(new File(fileName));

            out.time = time;
            if (image && image->getInfo() == f->info)
            {
                out.image = image;
            }
            else
            {
                out.image = imaging::Image::create(f->info);
            }

            if (f->planar)
            {
                std::vector<uint8_t> scanline;
                scanline.resize(f->info.size.w * f->sampleDepth / 8);
                for (size_t sample = 0; sample < f->samples; ++sample)
                {
                    for (uint16_t y = 0; y < f->info.size.h; ++y)
                    {
                        if (TIFFReadScanline(f->f, (tdata_t*)scanline.data(), y, sample) == -1)
                        {
                            break;
                        }
                        uint8_t* p = out.image->getData() + y * f->scanlineSize;
                        switch (f->sampleDepth)
                        {
                        case 8:
                        {
                            const uint8_t* inP = scanline.data();
                            uint8_t* outP = p + sample;
                            for (uint16_t x = 0; x < f->info.size.w; ++x, ++inP, outP += f->samples)
                            {
                                *outP = *inP;
                            }
                            break;
                        }
                        case 16:
                        {
                            const uint16_t* inP = reinterpret_cast<uint16_t*>(scanline.data());
                            uint16_t* outP = reinterpret_cast<uint16_t*>(p) + sample;
                            for (uint16_t x = 0; x < f->info.size.w; ++x, ++inP, outP += f->samples)
                            {
                                *outP = *inP;
                            }
                            break;
                        }
                        case 32:
                        {
                            const float* inP = reinterpret_cast<float*>(scanline.data());
                            float* outP = reinterpret_cast<float*>(p) + sample;
                            for (uint16_t x = 0; x < f->info.size.w; ++x, ++inP, outP += f->samples)
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
                for (uint16_t y = 0; y < f->info.size.h; ++y)
                {
                    if (TIFFReadScanline(f->f, (tdata_t*)out.image->getData() + y * f->scanlineSize, y) == -1)
                    {
                        break;
                    }
                }
            }

            if (f->palette)
            {
                for (uint16_t y = 0; y < f->info.size.h; ++y)
                {
                    readPalette(
                        out.image->getData() + y * f->scanlineSize,
                        f->info.size.w,
                        static_cast<int>(imaging::getChannelCount(f->info.pixelType)),
                        f->colormap[0], f->colormap[1], f->colormap[2]);
                }
            }

            return out;
        }
    }
}
