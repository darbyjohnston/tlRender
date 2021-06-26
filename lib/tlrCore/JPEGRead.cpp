// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/JPEG.h>

#include <tlrCore/StringFormat.h>

#include <cstring>

namespace tlr
{
    namespace jpeg
    {
        namespace
        {
            bool jpegCreate(
                jpeg_decompress_struct* decompress,
                ErrorStruct* error)
            {
                if (::setjmp(error->jump))
                {
                    return false;
                }
                jpeg_create_decompress(decompress);
                return true;
            }

            bool jpegOpen(
                FILE* f,
                jpeg_decompress_struct* decompress,
                ErrorStruct* error)
            {
                if (::setjmp(error->jump))
                {
                    return false;
                }
                jpeg_stdio_src(decompress, f);
                jpeg_save_markers(decompress, JPEG_COM, 0xFFFF);
                if (!jpeg_read_header(decompress, static_cast<boolean>(1)))
                {
                    return false;
                }
                if (!jpeg_start_decompress(decompress))
                {
                    return false;
                }
                return true;
            }

            bool jpegScanline(
                jpeg_decompress_struct* decompress,
                uint8_t* out,
                ErrorStruct* error)
            {
                if (::setjmp(error->jump))
                {
                    return false;
                }
                JSAMPROW p[] = { (JSAMPLE*)(out) };
                if (!jpeg_read_scanlines(decompress, p, 1))
                {
                    return false;
                }
                return true;
            }

            bool jpegEnd(
                jpeg_decompress_struct* decompress,
                ErrorStruct* error)
            {
                if (::setjmp(error->jump))
                {
                    return false;
                }
                jpeg_finish_decompress(decompress);
                return true;
            }

            class File
            {
            public:
                File(const std::string& fileName)
                {
                    std::memset(&_decompress, 0, sizeof(jpeg_decompress_struct));

                    _decompress.err = jpeg_std_error(&_error.pub);
                    _error.pub.error_exit = errorFunc;
                    _error.pub.emit_message = warningFunc;
                    if (!jpegCreate(&_decompress, &_error))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }
                    _init = true;
                    _f = fopen(fileName.c_str(), "rb");
                    if (!_f)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }
                    if (!jpegOpen(_f, &_decompress, &_error))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    imaging::PixelType pixelType = imaging::getIntType(_decompress.out_color_components, 8);
                    if (imaging::PixelType::None == pixelType)
                    {
                        throw std::runtime_error(string::Format("{0}: File not supported").arg(fileName));
                    }

                    _info = imaging::Info(_decompress.output_width, _decompress.output_height, pixelType);
                }

                ~File()
                {
                    if (_init)
                    {
                        jpeg_destroy_decompress(&_decompress);
                    }
                    if (_f)
                    {
                        fclose(_f);
                    }
                }

                const imaging::Info& getInfo() const
                {
                    return _info;
                }

                avio::VideoFrame read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    avio::VideoFrame out;
                    out.time = time;
                    out.image = imaging::Image::create(_info);

                    std::size_t scanlineByteCount = 0;
                    switch (_info.pixelType)
                    {
                    case imaging::PixelType::L_U8:
                        scanlineByteCount = _info.size.w;
                        break;
                    case imaging::PixelType::RGB_U8:
                        scanlineByteCount = _info.size.w * 3;
                        break;
                    }
                    for (uint16_t y = 0; y < _info.size.h; ++y)
                    {
                        if (!jpegScanline(&_decompress, out.image->getData() + scanlineByteCount * y, &_error))
                        {
                            break;
                        }
                    }

                    jpegEnd(&_decompress, &_error);

                    return out;
                }

            private:
                FILE*                  _f = nullptr;
                jpeg_decompress_struct _decompress;
                bool                   _init = false;
                ErrorStruct            _error;
                imaging::Info          _info;
            };
        }

        void Read::_init(
            const std::string& fileName,
            const avio::Options& options)
        {
            ISequenceRead::_init(fileName, options);
        }

        Read::Read()
        {}

        Read::~Read()
        {}

        std::shared_ptr<Read> Read::create(
            const std::string& fileName,
            const avio::Options& options)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, options);
            return out;
        }

        avio::Info Read::_getInfo(const std::string& fileName)
        {
            avio::Info out;
            avio::VideoInfo videoInfo;
            videoInfo.info = std::unique_ptr<File>(new File(fileName))->getInfo();
            videoInfo.duration = _defaultSpeed;
            out.video.push_back(videoInfo);
            return out;
        }

        avio::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time)
        {
            return std::unique_ptr<File>(new File(fileName))->read(fileName, time);
        }
    }
}
