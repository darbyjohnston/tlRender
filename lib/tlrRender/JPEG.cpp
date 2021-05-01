// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrRender/JPEG.h>

#include <tlrRender/Assert.h>

extern "C"
{
#include <jpeglib.h>

} // extern "C"

#include <cstring>
#include <sstream>

#include <setjmp.h>

namespace tlr
{
    namespace jpeg
    {
        namespace
        {
            struct ErrorStruct
            {
                struct jpeg_error_mgr pub;
                std::vector<std::string> messages;
                jmp_buf jump;
            };

            void jpegError(j_common_ptr in)
            {
                auto error = reinterpret_cast<ErrorStruct*>(in->err);
                char message[JMSG_LENGTH_MAX] = "";
                in->err->format_message(in, message);
                error->messages.push_back(message);
                ::longjmp(error->jump, 1);
            }

            void jpegWarning(j_common_ptr in, int level)
            {
                if (level > 0)
                {
                    return;
                }
                auto error = reinterpret_cast<ErrorStruct*>(in->err);
                char message[JMSG_LENGTH_MAX] = "";
                in->err->format_message(in, message);
                error->messages.push_back(message);
            }

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
                TLR_NON_COPYABLE(File);
                File()
                {
                    memset(&decompress, 0, sizeof(jpeg_decompress_struct));
                }
                void _init(const std::string& fileName)
                {
                    decompress.err = jpeg_std_error(&error.pub);
                    error.pub.error_exit = jpegError;
                    error.pub.emit_message = jpegWarning;
                    if (!jpegCreate(&decompress, &error))
                    {
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }
                    init = true;
                    f = fopen(fileName.c_str(), "rb");
                    if (!f)
                    {
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }
                    if (!jpegOpen(f, &decompress, &error))
                    {
                        std::stringstream ss;
                        ss << fileName << ": Cannot open";
                        throw std::runtime_error(ss.str());
                    }

                    imaging::PixelType pixelType = imaging::getIntType(decompress.out_color_components, 8);
                    if (imaging::PixelType::None == pixelType)
                    {
                        std::stringstream ss;
                        ss << fileName << ": File not supported";
                        throw std::runtime_error(ss.str());
                    }

                    info = imaging::Info(decompress.output_width, decompress.output_height, pixelType);
                }

            public:
                ~File()
                {
                    if (init)
                    {
                        jpeg_destroy_decompress(&decompress);
                        init = false;
                    }
                    if (f)
                    {
                        fclose(f);
                        f = nullptr;
                    }
                }

                static std::shared_ptr<File> create(const std::string& fileName)
                {
                    auto out = std::shared_ptr<File>(new File);
                    out->_init(fileName);
                    return out;
                }

                FILE* f = nullptr;
                jpeg_decompress_struct decompress;
                bool                   init = false;
                ErrorStruct            error;
                imaging::Info          info;
            };
        }

        void Read::_init(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed,
            size_t videoQueueSize)
        {
            ISequenceRead::_init(fileName, defaultSpeed, videoQueueSize);

            io::VideoInfo info;
            info.info = File::create(fileName)->info;
            info.duration = _defaultSpeed;
            info.codec = "JPEG";
            _info.video.push_back(info);
        }

        Read::Read()
        {}

        Read::~Read()
        {}

        std::shared_ptr<Read> Read::create(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed,
            size_t videoQueueSize)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, defaultSpeed, videoQueueSize);
            return out;
        }

        void Read::tick()
        {
            if (_hasSeek)
            {
                _currentTime = _seekTime.rescaled_to(_info.video[0].duration.rate());
                while (_videoQueue.size())
                {
                    _videoQueue.pop();
                }
            }

            if (_videoQueue.size() < _videoQueueSize)
            {
                io::VideoFrame frame;

                try
                {
                    const std::string fileName = _getFileName(_currentTime);
                    auto f = File::create(fileName);

                    imaging::PixelType pixelType = imaging::getIntType(f->decompress.out_color_components, 8);
                    if (imaging::PixelType::None == pixelType)
                    {
                        std::stringstream ss;
                        ss << fileName << ": File not supported";
                        throw std::runtime_error(ss.str());
                    }

                    const imaging::Info info(f->decompress.output_width, f->decompress.output_height, pixelType);
                    frame.time = _currentTime;
                    frame.image = imaging::Image::create(info);

                    for (uint16_t y = 0; y < info.size.h; ++y)
                    {
                        if (!jpegScanline(&f->decompress, frame.image->getData(y), &f->error))
                        {
                            break;
                        }
                    }

                    jpegEnd(&f->decompress, &f->error);
                }
                catch (const std::exception&)
                {}

                _videoQueue.push(frame);
                _currentTime += otime::RationalTime(1, _info.video[0].duration.rate());
            }

            _hasSeek = false;
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init();
            return out;
        }

        bool Plugin::canRead(const std::string& fileName)
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
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            return Read::create(fileName, defaultSpeed, _videoQueueSize);
        }
    }
}
