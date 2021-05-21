// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TIFF.h>

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
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }

                    uint32  width = 0;
                    uint32  height = 0;
                    uint16  photometric = 0;
                    uint16  samples = 0;
                    uint16  sampleDepth = 0;
                    uint16  sampleFormat = 0;
                    uint16* extraSamples = nullptr;
                    uint16  extraSamplesSize = 0;
                    uint16  orient = 0;
                    uint16  compression = 0;
                    uint16  channels = 0;
                    TIFFGetFieldDefaulted(f, TIFFTAG_IMAGEWIDTH, &width);
                    TIFFGetFieldDefaulted(f, TIFFTAG_IMAGELENGTH, &height);
                    TIFFGetFieldDefaulted(f, TIFFTAG_PHOTOMETRIC, &photometric);
                    TIFFGetFieldDefaulted(f, TIFFTAG_SAMPLESPERPIXEL, &samples);
                    TIFFGetFieldDefaulted(f, TIFFTAG_BITSPERSAMPLE, &sampleDepth);
                    TIFFGetFieldDefaulted(f, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
                    TIFFGetFieldDefaulted(f, TIFFTAG_EXTRASAMPLES, &extraSamplesSize, &extraSamples);
                    TIFFGetFieldDefaulted(f, TIFFTAG_ORIENTATION, &orient);
                    TIFFGetFieldDefaulted(f, TIFFTAG_COMPRESSION, &compression);
                    TIFFGetFieldDefaulted(f, TIFFTAG_PLANARCONFIG, &channels);
                    TIFFGetFieldDefaulted(f, TIFFTAG_COLORMAP, &colormap[0], &colormap[1], &colormap[2]);
                    scanlineSize = width * samples * sampleDepth / 8;

                    imaging::PixelType pixelType = imaging::PixelType::None;
                    switch (photometric)
                    {
                    case PHOTOMETRIC_PALETTE:
                        pixelType = imaging::PixelType::RGB_U8;
                        break;
                    case PHOTOMETRIC_MINISWHITE:
                    case PHOTOMETRIC_MINISBLACK:
                    case PHOTOMETRIC_RGB:
                        if (32 == sampleDepth && sampleFormat != SAMPLEFORMAT_IEEEFP)
                            break;
                        if (SAMPLEFORMAT_IEEEFP == sampleFormat)
                        {
                            pixelType = imaging::getFloatType(samples, sampleDepth);
                        }
                        else
                        {
                            pixelType = imaging::getIntType(samples, sampleDepth);
                        }
                        break;
                    }
                    if (imaging::PixelType::None == pixelType)
                    {
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }

                    compression = compression != COMPRESSION_NONE;
                    palette = PHOTOMETRIC_PALETTE == photometric;

                    info = imaging::Info(width, height, pixelType);
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
                bool          compression = false;
                bool          palette = false;
                uint16*       colormap[3] = { nullptr, nullptr, nullptr };
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

        io::VideoFrame Read::_getVideoFrame(const otime::RationalTime& time)
        {
            io::VideoFrame out;

            const std::string fileName = _getFileName(time);
            auto f = std::shared_ptr<File>(new File(fileName));

            out.time = time;
            out.image = imaging::Image::create(f->info);

            for (uint16_t y = 0; y < f->info.size.h; ++y)
            {
                uint8_t* p = out.image->getData() + y * f->scanlineSize;
                if (TIFFReadScanline(f->f, (tdata_t*)p, y) == -1)
                {
                    break;
                }
                if (f->palette)
                {
                    readPalette(
                        p,
                        f->info.size.w,
                        static_cast<int>(imaging::getChannelCount(f->info.pixelType)),
                        f->colormap[0], f->colormap[1], f->colormap[2]);
                }
            }

            return out;
        }

        Plugin::Plugin()
        {
            TIFFSetErrorHandler(nullptr);
            TIFFSetWarningHandler(nullptr);
        }
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init({ ".tiff", ".tif" });
            return out;
        }

        /*bool Plugin::canRead(const std::string& fileName)
        {
            bool out = false;
            try
            {
                File::create(fileName);
                out = true;
            }
            catch (const std::exception&)
            {}
            return out;
        }*/

        std::shared_ptr<io::IRead> Plugin::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            return Read::create(fileName, defaultSpeed);
        }
    }
}
