// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/JPEG.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <cstring>

namespace tl
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
#if defined(_WINDOWS)
                    if (_wfopen_s(&_f, string::toWide(fileName).c_str(), L"rb") != 0)
                    {
                        _f = nullptr;
                    }
#else
                    _f = fopen(fileName.c_str(), "rb");
#endif
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

                    imaging::Info imageInfo(_decompress.output_width, _decompress.output_height, pixelType);
                    imageInfo.layout.mirror.y = true;
                    _info.video.push_back(imageInfo);

                    const jpeg_saved_marker_ptr marker = _decompress.marker_list;
                    if (marker)
                    {
                        _info.tags["Description"] = std::string((const char*)marker->data, marker->data_length);
                    }
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

                const io::Info& getInfo() const
                {
                    return _info;
                }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    const auto& info = _info.video[0];
                    out.image = imaging::Image::create(info);
                    out.image->setTags(_info.tags);

                    std::size_t scanlineByteCount = 0;
                    switch (info.pixelType)
                    {
                    case imaging::PixelType::L_U8:
                        scanlineByteCount = info.size.w;
                        break;
                    case imaging::PixelType::RGB_U8:
                        scanlineByteCount = info.size.w * 3;
                        break;
                    default: break;
                    }
                    uint8_t* p = out.image->getData();
                    for (uint16_t y = 0; y < info.size.h; ++y, p += scanlineByteCount)
                    {
                        if (!jpegScanline(&_decompress, p, &_error))
                        {
                            break;
                        }
                    }

                    jpegEnd(&_decompress, &_error);

                    return out;
                }

            private:
                FILE* _f = nullptr;
                jpeg_decompress_struct _decompress;
                bool _init = false;
                ErrorStruct _error;
                io::Info _info;
            };
        }

        void Read::_init(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
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
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(const std::string& fileName)
        {
            io::Info out = File(fileName).getInfo();
            out.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, _defaultSpeed),
                otime::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            return File(fileName).read(fileName, time);
        }
    }
}
