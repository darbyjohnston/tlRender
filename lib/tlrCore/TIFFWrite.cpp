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
                File(
                    const std::string& fileName,
                    const std::shared_ptr<imaging::Image>& image)
                {
                    f = TIFFOpen(fileName.data(), "w");
                    if (!f)
                    {
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }

                    uint16 photometric = 0;
                    uint16 samples = 0;
                    uint16 sampleDepth = 0;
                    uint16 sampleFormat = 0;
                    uint16 extraSamples[] = { EXTRASAMPLE_ASSOCALPHA };
                    uint16 extraSamplesSize = 0;
                    uint16 compression = 0;
                    const auto& info = image->getInfo();
                    switch (imaging::getChannelCount(info.pixelType))
                    {
                    case 1:
                        photometric = PHOTOMETRIC_MINISBLACK;
                        samples = 1;
                        break;
                    case 2:
                        photometric = PHOTOMETRIC_MINISBLACK;
                        samples = 2;
                        extraSamplesSize = 1;
                        break;
                    case 3:
                        photometric = PHOTOMETRIC_RGB;
                        samples = 3;
                        break;
                    case 4:
                        photometric = PHOTOMETRIC_RGB;
                        samples = 4;
                        extraSamplesSize = 1;
                        break;
                    default: break;
                    }
                    switch (info.pixelType)
                    {
                    case imaging::PixelType::L_U8:
                    case imaging::PixelType::LA_U8:
                    case imaging::PixelType::RGB_U8:
                    case imaging::PixelType::RGBA_U8:
                        sampleDepth = 8;
                        sampleFormat = SAMPLEFORMAT_UINT;
                        break;
                    case imaging::PixelType::L_U16:
                    case imaging::PixelType::LA_U16:
                    case imaging::PixelType::RGB_U16:
                    case imaging::PixelType::RGBA_U16:
                        sampleDepth = 16;
                        sampleFormat = SAMPLEFORMAT_UINT;
                        break;
                    case imaging::PixelType::L_F32:
                    case imaging::PixelType::LA_F32:
                    case imaging::PixelType::RGB_F32:
                    case imaging::PixelType::RGBA_F32:
                        sampleDepth = 32;
                        sampleFormat = SAMPLEFORMAT_IEEEFP;
                        break;
                    default: break;
                    }
                    if (!samples || !sampleDepth)
                    {
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }
                    const size_t scanlineSize = info.size.w * samples * sampleDepth / 8;

                    compression = COMPRESSION_NONE;
                    /*switch (_p->options.compression)
                    {
                    case Compression::None:
                        compression = COMPRESSION_NONE;
                        break;
                    case Compression::RLE:
                        compression = COMPRESSION_PACKBITS;
                        break;
                    case Compression::LZW:
                        compression = COMPRESSION_LZW;
                        break;
                    default: break;
                    }*/
                    TIFFSetField(f, TIFFTAG_IMAGEWIDTH, info.size.w);
                    TIFFSetField(f, TIFFTAG_IMAGELENGTH, info.size.h);
                    TIFFSetField(f, TIFFTAG_PHOTOMETRIC, photometric);
                    TIFFSetField(f, TIFFTAG_SAMPLESPERPIXEL, samples);
                    TIFFSetField(f, TIFFTAG_BITSPERSAMPLE, sampleDepth);
                    TIFFSetField(f, TIFFTAG_SAMPLEFORMAT, sampleFormat);
                    TIFFSetField(f, TIFFTAG_EXTRASAMPLES, extraSamplesSize, extraSamples);
                    TIFFSetField(f, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
                    TIFFSetField(f, TIFFTAG_COMPRESSION, compression);
                    TIFFSetField(f, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

                    for (uint16_t y = 0; y < info.size.h; ++y)
                    {
                        uint8_t* p = image->getData() + imaging::getDataByteCount(info) - (y + 1) * scanlineSize;
                        if (TIFFWriteScanline(f, (tdata_t*)p, y) == -1)
                        {
                            std::stringstream ss;
                            ss << fileName << ": Cannot write scanline: " << y;
                            throw std::runtime_error(ss.str());
                        }
                    }
                }

                ~File()
                {
                    if (f)
                    {
                        TIFFClose(f);
                        f = nullptr;
                    }
                }

                TIFF* f = nullptr;
            };
        }

        void Write::_init(
            const std::string& fileName,
            const io::Info& info)
        {
            ISequenceWrite::_init(fileName, info);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const std::string& fileName,
            const io::Info& info)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(fileName, info);
            return out;
        }

        void Write::_writeVideoFrame(
            const std::string& fileName,
            const otime::RationalTime&,
            const std::shared_ptr<imaging::Image>& image)
        {
            File(fileName, image);
        }
    }
}
