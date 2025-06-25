// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/PNGPrivate.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/Memory.h>
#include <feather-tk/core/String.h>

namespace tl
{
    namespace png
    {
        namespace
        {
            void pngMemoryRead(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
            {
                png_voidp io_ptr = png_get_io_ptr(png_ptr);
                feather_tk::InMemoryFile* memory = static_cast<feather_tk::InMemoryFile*>(io_ptr);
                if (byteCountToRead > memory->size)
                {
                    png_error(png_ptr, "Cannot read");
                }
                memcpy(outBytes, memory->p, byteCountToRead);
                memory->p += byteCountToRead;
                memory->size -= byteCountToRead;
            }

            bool pngOpen(
                FILE* f,
                feather_tk::InMemoryFile* memory,
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
                if (memory->p)
                {
                    if (memory->size < 8)
                    {
                        return false;
                    }
                    memcpy(tmp, memory->p, 8);
                    memory->p += 8;
                    memory->size -= 8;
                }
                else
                {
                    if (fread(tmp, 8, 1, f) != 1)
                    {
                        return false;
                    }
                }
                if (png_sig_cmp(tmp, 0, 8))
                {
                    return false;
                }

                if (memory->p)
                {
                    png_set_read_fn(png, memory, pngMemoryRead);
                }
                else
                {
                    png_init_io(png, f);
                }
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

                if (bitDepth >= 16 && feather_tk::Endian::LSB == feather_tk::getEndian())
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
                File(
                    const std::string& fileName,
                    const feather_tk::InMemoryFile* memory)
                {
                    _png.p = png_create_read_struct(
                        PNG_LIBPNG_VER_STRING,
                        &_error,
                        errorFunc,
                        warningFunc);

                    if (memory)
                    {
                        _memory.p = memory->p;
                        _memory.size = memory->size;
                    }
                    else
                    {
#if defined(_WINDOWS)
                        if (_wfopen_s(&_f.p, feather_tk::toWide(fileName).c_str(), L"rb") != 0)
                        {
                            _f.p = nullptr;
                        }
#else // _WINDOWS
                        _f.p = fopen(fileName.c_str(), "rb");
#endif // _WINDOWS
                        if (!_f.p)
                        {
                            throw std::runtime_error(feather_tk::Format("Cannot open: \"{0}\"").arg(fileName));
                        }
                    }

                    uint16_t width = 0;
                    uint16_t height = 0;
                    uint8_t  channels = 0;
                    uint8_t  bitDepth = 0;
                    if (!pngOpen(
                        _f.p,
                        &_memory,
                        _png.p,
                        &_png.info,
                        &_png.infoEnd,
                        width,
                        height,
                        channels,
                        bitDepth))
                    {
                        throw std::runtime_error(feather_tk::Format("Cannot open: \"{0}\"").arg(fileName));
                    }
                    _scanlineSize = width * channels * bitDepth / 8;

                    feather_tk::ImageType pixelType = io::getIntType(channels, bitDepth);
                    if (feather_tk::ImageType::None == pixelType)
                    {
                        throw std::runtime_error(feather_tk::Format("Cannot open: \"{0}\"").arg(fileName));
                    }

                    _info = feather_tk::ImageInfo(width, height, pixelType);
                    _info.layout.mirror.y = true;
                }

                const feather_tk::ImageInfo& getInfo() const
                {
                    return _info;
                }

                std::shared_ptr<feather_tk::Image> read()
                {
                    auto out = feather_tk::Image::create(_info);
                    uint8_t* p = out->getData();
                    for (uint16_t y = 0; y < _info.size.h; ++y, p += _scanlineSize)
                    {
                        if (!pngScanline(_png.p, p))
                        {
                            break;
                        }
                    }
                    pngEnd(_png.p, _png.infoEnd);
                    return out;
                }

            private:
                struct PNGData
                {
                    ~PNGData()
                    {
                        if (p || info || infoEnd)
                        {
                            png_destroy_read_struct(
                                p ? &p : nullptr,
                                info ? &info : nullptr,
                                infoEnd ? &infoEnd : nullptr);
                        }
                    }
                    png_structp p = nullptr;
                    png_infop   info = nullptr;
                    png_infop   infoEnd = nullptr;
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

                PNGData          _png;
                FilePointer      _f;
                feather_tk::InMemoryFile _memory;
                ErrorStruct      _error;
                size_t           _scanlineSize = 0;
                feather_tk::ImageInfo    _info;
            };
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, logSystem);
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
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const feather_tk::InMemoryFile* memory)
        {
            io::Info out;
            out.video.push_back(File(fileName, memory).getInfo());
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, _defaultSpeed),
                OTIO_NS::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const feather_tk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            io::VideoData out;
            out.time = time;
            out.image = File(fileName, memory).read();
            return out;
        }
    }
}
