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
            bool jpegInit(jpeg_compress_struct* jpeg, ErrorStruct* error)
            {
                if (setjmp(error->jump))
                {
                    return false;
                }
                jpeg_create_compress(jpeg);
                return true;
            }

            bool jpegOpen(
                FILE* f,
                jpeg_compress_struct* jpeg,
                const imaging::Info& info,
                const std::map<std::string, std::string>& tags,
                ErrorStruct* error)
            {
                if (setjmp(error->jump))
                {
                    return false;
                }
                jpeg_stdio_dest(jpeg, f);
                jpeg->image_width = info.size.w;
                jpeg->image_height = info.size.h;
                if (imaging::PixelType::L_U8 == info.pixelType)
                {
                    jpeg->input_components = 1;
                    jpeg->in_color_space = JCS_GRAYSCALE;
                }
                else if (imaging::PixelType::RGB_U8 == info.pixelType)
                {
                    jpeg->input_components = 3;
                    jpeg->in_color_space = JCS_RGB;
                }
                jpeg_set_defaults(jpeg);
                jpeg_set_quality(jpeg, 90, static_cast<boolean>(1));
                jpeg_start_compress(jpeg, static_cast<boolean>(1));
                const auto i = tags.find("Description");
                if (i != tags.end())
                {
                    jpeg_write_marker(
                        jpeg,
                        JPEG_COM,
                        (JOCTET*)i->second.c_str(),
                        static_cast<unsigned int>(i->second.size()));
                }
                return true;
            }

            bool jpegScanline(
                jpeg_compress_struct* jpeg,
                const uint8_t* in,
                ErrorStruct* error)
            {
                if (::setjmp(error->jump))
                {
                    return false;
                }
                JSAMPROW p[] = { (JSAMPLE*)(in) };
                if (!jpeg_write_scanlines(jpeg, p, 1))
                {
                    return false;
                }
                return true;
            }

            bool jpegEnd(jpeg_compress_struct* jpeg, ErrorStruct* error)
            {
                if (::setjmp(error->jump))
                {
                    return false;
                }
                jpeg_finish_compress(jpeg);
                return true;
            }

            class File
            {
            public:
                File(
                    const std::string& fileName,
                    const std::shared_ptr<imaging::Image>& image)
                {
                    memset(&_jpeg, 0, sizeof(jpeg_compress_struct));

                    _jpeg.err = jpeg_std_error(&_error.pub);
                    _error.pub.error_exit = errorFunc;
                    _error.pub.emit_message = warningFunc;
                    if (!jpegInit(&_jpeg, &_error))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }
                    _init = true;
                    _f = fopen(fileName.c_str(), "wb");
                    if (!_f)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }
                    const auto& info = image->getInfo();
                    if (!jpegOpen(_f, &_jpeg, info, image->getTags(), &_error))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot open").arg(fileName));
                    }

                    size_t scanlineSize = 0;
                    switch (info.pixelType)
                    {
                    case imaging::PixelType::L_U8: scanlineSize = info.size.w; break;
                    case imaging::PixelType::RGB_U8: scanlineSize = info.size.w * 3; break;
                    default: break;
                    }
                    for (uint16_t y = 0; y < info.size.h; ++y)
                    {
                        uint8_t* p = image->getData() + (y * scanlineSize);
                        if (!jpegScanline(&_jpeg, p, &_error))
                        {
                            throw std::runtime_error(string::Format("{0}: Cannot write scanline: {1}").arg(fileName).arg(y));
                        }
                    }
                    if (!jpegEnd(&_jpeg, &_error))
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot close").arg(fileName));
                    }
                }

                ~File()
                {
                    if (_init)
                    {
                        jpeg_destroy_compress(&_jpeg);
                    }
                    if (_f)
                    {
                        fclose(_f);
                    }
                }

            private:
                FILE*                _f = nullptr;
                jpeg_compress_struct _jpeg;
                bool                 _init = false;
                ErrorStruct          _error;
            };
        }

        void Write::_init(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            ISequenceWrite::_init(fileName, info, options);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(fileName, info, options);
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
