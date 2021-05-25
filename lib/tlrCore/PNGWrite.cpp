// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/PNG.h>

#include <tlrCore/Memory.h>
#include <tlrCore/StringFormat.h>

namespace tlr
{
    namespace png
    {
        namespace
        {
            bool pngOpen(
                FILE* f,
                png_structp png,
                png_infop* pngInfo,
                const imaging::Info& info)
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
                png_init_io(png, f);

                int colorType = 0;
                switch (info.pixelType)
                {
                case imaging::PixelType::L_U8:
                case imaging::PixelType::L_U16:
                    colorType = PNG_COLOR_TYPE_GRAY;
                    break;
                case imaging::PixelType::LA_U8:
                case imaging::PixelType::LA_U16:
                    colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
                    break;
                case imaging::PixelType::RGB_U8:
                case imaging::PixelType::RGB_U16:
                    colorType = PNG_COLOR_TYPE_RGB;
                    break;
                case imaging::PixelType::RGBA_U8:
                case imaging::PixelType::RGBA_U16:
                    colorType = PNG_COLOR_TYPE_RGB_ALPHA;
                    break;
                default: break;
                }

                const int bitDepth = imaging::getBitDepth(info.pixelType);
                png_set_IHDR(
                    png,
                    *pngInfo,
                    info.size.w,
                    info.size.h,
                    bitDepth,
                    colorType,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT);
                png_write_info(png, *pngInfo);

                if (bitDepth > 8 && memory::Endian::LSB == memory::getEndian())
                {
                    png_set_swap(png);
                }

                return true;
            }

            bool pngScanline(png_structp png, const uint8_t* in)
            {
                if (setjmp(png_jmpbuf(png)))
                    return false;
                png_write_row(png, reinterpret_cast<const png_byte*>(in));
                return true;
            }

            bool pngEnd(png_structp png, png_infop pngInfo)
            {
                if (setjmp(png_jmpbuf(png)))
                    return false;
                png_write_end(png, pngInfo);
                return true;
            }

            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<imaging::Image>& image)
                {
                    _png = png_create_write_struct(
                        PNG_LIBPNG_VER_STRING,
                        &_error,
                        errorFunc,
                        warningFunc);
                    if (!_png)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    _f = fopen(fileName.c_str(), "wb");
                    if (!_f)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    const auto& info = image->getInfo();
                    if (!pngOpen(_f, _png, &_pngInfo, info))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    size_t scanlineSize = 0;
                    switch (info.pixelType)
                    {
                    case imaging::PixelType::L_U8: scanlineSize = info.size.w; break;
                    case imaging::PixelType::L_U16: scanlineSize = info.size.w * 2; break;
                    case imaging::PixelType::LA_U8: scanlineSize = info.size.w * 2; break;
                    case imaging::PixelType::LA_U16: scanlineSize = info.size.w * 2 * 2; break;
                    case imaging::PixelType::RGB_U8: scanlineSize = info.size.w * 3; break;
                    case imaging::PixelType::RGB_U16: scanlineSize = info.size.w * 3 * 2; break;
                    case imaging::PixelType::RGBA_U8: scanlineSize = info.size.w * 4; break;
                    case imaging::PixelType::RGBA_U16: scanlineSize = info.size.w * 4 * 2; break;
                    default: break;
                    }
                    for (uint16_t y = 0; y < info.size.h; ++y)
                    {
                        uint8_t* p = image->getData() + (info.flipY ?
                            (imaging::getDataByteCount(info) - (y + 1) * scanlineSize) :
                            (y * scanlineSize));
                        if (!pngScanline(_png, p))
                        {
                            throw std::runtime_error(string::Format("{0}: Cannot write scanline: {1}").arg(fileName).arg(y));
                        }
                    }
                    if (!pngEnd(_png, _pngInfo))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot close").arg(fileName));
                    }
                }

                ~File()
                {
                    if (_f)
                    {
                        fclose(_f);
                    }
                    if (_png || _pngInfo)
                    {
                        png_destroy_write_struct(
                            _png ? &_png : nullptr,
                            _pngInfo ? &_pngInfo : nullptr);
                    }
                }

            private:
                FILE* _f = nullptr;
                png_structp _png = nullptr;
                png_infop   _pngInfo = nullptr;
                ErrorStruct _error;
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
            const auto f = File(fileName, image);
        }
    }
}
