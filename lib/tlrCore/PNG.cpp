// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/PNG.h>

#include <tlrCore/Memory.h>
#include <tlrCore/StringFormat.h>

#include <png.h>

#include <sstream>

namespace tlr
{
    namespace png
    {
        namespace
        {
            struct ErrorStruct
            {
                std::string message;
            };

            extern "C"
            {
                void pngErrorFunc(png_structp in, png_const_charp msg)
                {
                    auto error = reinterpret_cast<ErrorStruct*>(png_get_error_ptr(in));
                    error->message = msg;
                    longjmp(png_jmpbuf(in), 1);
                }

                void pngWarningFunc(png_structp in, png_const_charp msg)
                {
                    auto error = reinterpret_cast<ErrorStruct*>(png_get_error_ptr(in));
                    error->message = msg;
                }

            } // extern "C"

            bool pngOpen(
                FILE* f,
                png_structp png,
                png_infop* pngInfo,
                png_infop* pngInfoEnd,
                uint16_t& width,
                uint16_t& height,
                uint8_t& channels,
                uint8_t& bitDepth)
            {
                if (setjmp(png_jmpbuf(png)))
                {
                    return false;
                }

                *pngInfo = png_create_info_struct(png);
                if (!*pngInfo)
                {
                    return false;
                }

                *pngInfoEnd = png_create_info_struct(png);
                if (!*pngInfoEnd)
                {
                    return false;
                }

                uint8_t tmp[8];
                if (fread(tmp, 8, 1, f) != 1)
                {
                    return false;
                }
                if (png_sig_cmp(tmp, 0, 8))
                {
                    return false;
                }

                png_init_io(png, f);
                png_set_sig_bytes(png, 8);
                png_read_info(png, *pngInfo);

                if (png_get_interlace_type(png, *pngInfo) != PNG_INTERLACE_NONE)
                {
                    return false;
                }

                png_set_expand(png);
                //png_set_gray_1_2_4_to_8(png);
                png_set_palette_to_rgb(png);
                png_set_tRNS_to_alpha(png);

                width = png_get_image_width(png, *pngInfo);
                height = png_get_image_height(png, *pngInfo);

                channels = png_get_channels(png, *pngInfo);
                if (png_get_color_type(png, *pngInfo) == PNG_COLOR_TYPE_PALETTE)
                {
                    channels = 3;
                }
                if (png_get_valid(png, *pngInfo, PNG_INFO_tRNS))
                {
                    ++channels;
                }
                bitDepth = png_get_bit_depth(png, *pngInfo);
                if (bitDepth < 8)
                {
                    bitDepth = 8;
                }

                if (bitDepth >= 16 && memory::Endian::LSB == memory::getEndian())
                {
                    png_set_swap(png);
                }

                return true;
            }

            bool pngScanline(png_structp png, uint8_t* out)
            {
                if (setjmp(png_jmpbuf(png)))
                {
                    return false;
                }
                png_read_row(png, out, 0);
                return true;
            }

            bool pngEnd(png_structp png, png_infop pngInfo)
            {
                if (setjmp(png_jmpbuf(png)))
                {
                    return false;
                }
                png_read_end(png, pngInfo);
                return true;
            }

            class File
            {
            public:
                File(const std::string& fileName)
                {
                    png = png_create_read_struct(
                        PNG_LIBPNG_VER_STRING,
                        &pngError,
                        pngErrorFunc,
                        pngWarningFunc);

                    f = fopen(fileName.c_str(), "rb");
                    if (!f)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    uint16_t width = 0;
                    uint16_t height = 0;
                    uint8_t  channels = 0;
                    uint8_t  bitDepth = 0;
                    if (!pngOpen(f, png, &pngInfo, &pngInfoEnd, width, height, channels, bitDepth))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }
                    scanlineSize = width * channels * bitDepth / 8;

                    imaging::PixelType pixelType = imaging::getIntType(channels, bitDepth);
                    if (imaging::PixelType::None == pixelType)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    info = imaging::Info(width, height, pixelType);
                }

                ~File()
                {
                    if (f)
                    {
                        fclose(f);
                        f = nullptr;
                    }
                    if (png || pngInfo || pngInfoEnd)
                    {
                        png_destroy_read_struct(
                            png ? &png : nullptr,
                            pngInfo ? &pngInfo : nullptr,
                            pngInfoEnd ? &pngInfoEnd : nullptr);
                        png = nullptr;
                        pngInfo = nullptr;
                        pngInfoEnd = nullptr;
                    }
                }

                FILE*         f = nullptr;
                png_structp   png = nullptr;
                png_infop     pngInfo = nullptr;
                png_infop     pngInfoEnd = nullptr;
                ErrorStruct   pngError;
                size_t        scanlineSize = 0;
                imaging::Info info;
            };
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
            videoInfo.codec = "PNG";
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

            for (uint16_t y = 0; y < f->info.size.h; ++y)
            {
                if (!pngScanline(f->png, out.image->getData() + y * f->scanlineSize))
                {
                    break;
                }
            }
            pngEnd(f->png, f->pngInfoEnd);

            return out;
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init({ ".png" });
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            return Read::create(fileName, defaultSpeed);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return {};
        }

        std::shared_ptr<io::IWrite> Plugin::write(
            const std::string& fileName,
            const io::Info&)
        {
            return nullptr;
        }
    }
}
