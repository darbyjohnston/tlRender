// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#define STBI_NO_JPEG
#define STBI_NO_PNG
#define STBI_NO_HDR
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <tlIO/STB.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace stb
    {
        namespace
        {
            
            class File
            {
            public:
                File(const std::string& fileName, const file::MemoryRead* memory)
                    {
                        int res = 0, w = 0, h = 0, n = 0, bits = 8;

                        _memory = memory;
                        
                        if (memory)
                        {
                            res = stbi_info_from_memory(memory->p, memory->size, &w,
                                                        &h, &n);
                            if (res == 0)
                                throw std::runtime_error(
                                    string::Format("{0}: {1}")
                                        .arg(fileName)
                                        .arg("Corrupted image type"));
                            
                            _info.size.w = w;
                            _info.size.h = h;
                            
                            res = stbi_is_16_bit_from_memory(memory->p, memory->size);
                            if (res) bits = 16;
                            
                            _info.pixelType = imaging::getIntType(n, bits);
                            
                            if (imaging::PixelType::None == _info.pixelType)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: {1}")
                                        .arg(fileName)
                                        .arg("Unsupported image type"));
                            }
                            _info.layout.endian = memory::Endian::MSB;
                        }
                        else
                        {

                            res = stbi_info(fileName.c_str(), &w, &h, &n);
                            if (res == 0)
                                throw std::runtime_error(
                                    string::Format("{0}: {1}")
                                        .arg(fileName)
                                        .arg("Corrupted image type"));

                            _info.size.w = w;
                            _info.size.h = h;
                    
                            res = stbi_is_16_bit(fileName.c_str());
                            if (res) bits = 16;
                    
                            _info.pixelType = imaging::getIntType(n, bits);
                            if (imaging::PixelType::None == _info.pixelType)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: {1}")
                                        .arg(fileName)
                                        .arg("Unsupported image type"));
                            }
                            _info.layout.endian = memory::Endian::MSB;
                        }
                }

                const imaging::Info& getInfo() const
                {
                    return _info;
                }

                io::VideoData read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    out.image = imaging::Image::create(_info);

                    const int channels = static_cast<int>(
                        imaging::getChannelCount(_info.pixelType));
                    const size_t bytes = imaging::getBitDepth(_info.pixelType) / 8;
                    
                    stbi_set_flip_vertically_on_load(1);

                    int x, y, n;
                    stbi_uc* data = nullptr;

                    if (_memory)
                    {
                        if (bytes == 1)
                            data = stbi_load_from_memory(_memory->p, _memory->size,
                                                         &x, &y, &n, 0);
                        else if (bytes == 2)
                            data = reinterpret_cast<stbi_uc*>(
                                stbi_load_16_from_memory(_memory->p, _memory->size,
                                                         &x, &y, &n, 0));
                    }
                    else
                    {
                        if (bytes == 1)
                            data = stbi_load(fileName.c_str(), &x, &y, &n, 0);
                        else if (bytes == 2)
                            data = reinterpret_cast<stbi_uc*>(
                                stbi_load_16(fileName.c_str(), &x, &y, &n, 0));
                    }
                                                       
                    memcpy(
                        out.image->getData(), data,
                        _info.size.w * _info.size.h * channels * bytes);

                    stbi_image_free(data);

                    return out;
                }

            private:
                imaging::Info _info;
                const file::MemoryRead* _memory;
            };
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
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
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const file::MemoryRead* memory)
        {
            io::Info out;
            out.video.push_back(File(fileName, memory).getInfo());
            out.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(_startFrame, _defaultSpeed),
                otime::RationalTime(_endFrame, _defaultSpeed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const file::MemoryRead* memory,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            return File(fileName, memory).read(fileName, time);
        }
    }
}
