// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/JPEGPrivate.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <cstring>

namespace tl
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
                const ftk::ImageInfo& info,
                const ftk::ImageTags& tags,
                int quality,
                ErrorStruct* error)
            {
                if (setjmp(error->jump))
                {
                    return false;
                }
                jpeg_stdio_dest(jpeg, f);
                jpeg->image_width = info.size.w;
                jpeg->image_height = info.size.h;
                if (ftk::ImageType::L_U8 == info.type)
                {
                    jpeg->input_components = 1;
                    jpeg->in_color_space = JCS_GRAYSCALE;
                }
                else if (ftk::ImageType::RGB_U8 == info.type)
                {
                    jpeg->input_components = 3;
                    jpeg->in_color_space = JCS_RGB;
                }
                jpeg_set_defaults(jpeg);
                jpeg_set_quality(jpeg, quality, static_cast<boolean>(1));
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
                    const std::shared_ptr<ftk::Image>& image,
                    int quality)
                {
                    std::memset(&_jpeg.compress, 0, sizeof(jpeg_compress_struct));

                    _jpeg.compress.err = jpeg_std_error(&_error.pub);
                    _error.pub.error_exit = errorFunc;
                    _error.pub.emit_message = warningFunc;
                    if (!jpegInit(&_jpeg.compress, &_error))
                    {
                        throw std::runtime_error(ftk::Format("Cannot open: \"{0}\"").arg(fileName));
                    }
#if defined(_WINDOWS)
                    if (_wfopen_s(&_f.p, ftk::toWide(fileName).c_str(), L"wb") != 0)
                    {
                        _f.p = nullptr;
                    }
#else // _WINDOWS
                    _f.p = fopen(fileName.c_str(), "wb");
#endif // _WINDOWS
                    if (!_f.p)
                    {
                        throw std::runtime_error(ftk::Format("Cannot open: \"{0}\"").arg(fileName));
                    }
                    const auto& info = image->getInfo();
                    if (!jpegOpen(_f.p, &_jpeg.compress, info, image->getTags(), quality, &_error))
                    {
                        throw std::runtime_error(ftk::Format("Cannot open: \"{0}\"").arg(fileName));
                    }

                    size_t scanlineByteCount = 0;
                    switch (info.type)
                    {
                    case ftk::ImageType::L_U8: scanlineByteCount = info.size.w; break;
                    case ftk::ImageType::RGB_U8: scanlineByteCount = static_cast<size_t>(info.size.w) * 3; break;
                    default: break;
                    }
                    scanlineByteCount = ftk::getAlignedByteCount(scanlineByteCount, info.layout.alignment);
                    const uint8_t* imageP = image->getData() + (info.size.h - 1) * scanlineByteCount;
                    for (uint16_t y = 0; y < info.size.h; ++y, imageP -= scanlineByteCount)
                    {
                        if (!jpegScanline(&_jpeg.compress, imageP, &_error))
                        {
                            throw std::runtime_error(ftk::Format("Cannot write scanline: \"{0}\": {1}").arg(fileName).arg(y));
                        }
                    }
                    if (!jpegEnd(&_jpeg.compress, &_error))
                    {
                        throw std::runtime_error(ftk::Format("Cannot close: \"{0}\"").arg(fileName));
                    }
                }

            private:
                struct JPEGData
                {
                    ~JPEGData()
                    {
                        jpeg_destroy_compress(&compress);
                    }
                    jpeg_compress_struct compress;
                };

                struct FilePointer
                {
                    ~FilePointer()
                    {
                        if (p)
                        {
                            fclose(p);
                        }
                    }
                    FILE* p = nullptr;
                };

                JPEGData    _jpeg;
                FilePointer _f;
                ErrorStruct _error;
            };
        }

        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            ISequenceWrite::_init(path, info, options, logSystem);

            auto option = options.find("JPEG/Quality");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> _quality;
            }
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeVideo(
            const std::string& fileName,
            const OTIO_NS::RationalTime&,
            const std::shared_ptr<ftk::Image>& image,
            const io::Options&)
        {
            const auto f = File(fileName, image, _quality);
        }
    }
}
