// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/PNG.h>

#include <tlrCore/Memory.h>
#include <tlrCore/String.h>
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
                    _png = png_create_read_struct(
                        PNG_LIBPNG_VER_STRING,
                        &_error,
                        errorFunc,
                        warningFunc);

#if defined(_WINDOWS)
                    if (_wfopen_s(&_f, string::toWide(fileName).c_str(), L"rb") != 0)
                    {
                        _f = nullptr;
                    }
#else
                    _f = fopen(fileName, "rb");
#endif
                    if (!_f)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    uint16_t width = 0;
                    uint16_t height = 0;
                    uint8_t  channels = 0;
                    uint8_t  bitDepth = 0;
                    if (!pngOpen(_f, _png, &_pngInfo, &_pngInfoEnd, width, height, channels, bitDepth))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }
                    _scanlineSize = width * channels * bitDepth / 8;

                    imaging::PixelType pixelType = imaging::getIntType(channels, bitDepth);
                    if (imaging::PixelType::None == pixelType)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    _info = imaging::Info(width, height, pixelType);
                    _info.layout.mirror.y = true;
                }

                ~File()
                {
                    if (_f)
                    {
                        fclose(_f);
                    }
                    if (_png || _pngInfo || _pngInfoEnd)
                    {
                        png_destroy_read_struct(
                            _png ? &_png : nullptr,
                            _pngInfo ? &_pngInfo : nullptr,
                            _pngInfoEnd ? &_pngInfoEnd : nullptr);
                    }
                }

                const imaging::Info& getInfo() const
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
                    out.image = image && image->getInfo() == _info ? image : imaging::Image::create(_info);

                    uint8_t* p = out.image->getData();
                    for (uint16_t y = 0; y < _info.size.h; ++y, p += _scanlineSize)
                    {
                        if (!pngScanline(_png, p))
                        {
                            break;
                        }
                    }
                    pngEnd(_png, _pngInfoEnd);

                    return out;
                }

            private:
                FILE*         _f = nullptr;
                png_structp   _png = nullptr;
                png_infop     _pngInfo = nullptr;
                png_infop     _pngInfoEnd = nullptr;
                ErrorStruct   _error;
                size_t        _scanlineSize = 0;
                imaging::Info _info;
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
            avio::Info out;
            out.video.push_back(File(fileName).getInfo());
            out.videoTimeRange = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, _defaultSpeed),
                otime::RationalTime(_endFrame, _defaultSpeed));
            out.videoType = avio::VideoType::Sequence;
            return out;
        }

        avio::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time,
            uint16_t layer,
            const std::shared_ptr<imaging::Image>& image)
        {
            return File(fileName).read(fileName, time, image);
        }
    }
}
