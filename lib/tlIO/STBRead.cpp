// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#define STBI_NO_JPEG
#define STBI_NO_PNG
#define STBI_NO_HDR
#define STBI_NO_PNM
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <tlIO/STB.h>

#include <feather-tk/core/Format.h>

namespace tl
{
    namespace stb
    {
        namespace
        {
            
            class File
            {
            public:
                File(const std::string& fileName, const ftk::InMemoryFile* memory)
                {
                    int res = 0, w = 0, h = 0, n = 0, bits = 8;

                    _memory = memory;
                        
                    if (memory)
                    {
                        res = stbi_info_from_memory(memory->p, memory->size, &w,
                                                    &h, &n);
                        if (res == 0)
                            throw std::runtime_error(
                                ftk::Format("Corrupted image type: \"{0}\"")
                                    .arg(fileName));
                            
                        _info.size.w = w;
                        _info.size.h = h;
                            
                        res = stbi_is_16_bit_from_memory(memory->p, memory->size);
                        if (res) bits = 16;
                            
                        _info.type = io::getIntType(n, bits);
                            
                        if (ftk::ImageType::None == _info.type)
                        {
                            throw std::runtime_error(
                                ftk::Format("Unsupported image type: \"{0}\"")
                                    .arg(fileName));
                        }
                        _info.layout.endian = ftk::Endian::MSB;
                    }
                    else
                    {

                        res = stbi_info(fileName.c_str(), &w, &h, &n);
                        if (res == 0)
                            throw std::runtime_error(
                                ftk::Format("Corrupted image type: \"{0}\"")
                                    .arg(fileName));

                        _info.size.w = w;
                        _info.size.h = h;
                    
                        res = stbi_is_16_bit(fileName.c_str());
                        if (res) bits = 16;
                    
                        _info.type = io::getIntType(n, bits);
                        if (ftk::ImageType::None == _info.type)
                        {
                            throw std::runtime_error(
                                ftk::Format("Unsupported image type: \"{0}\"")
                                    .arg(fileName));
                        }
                        _info.layout.endian = ftk::Endian::MSB;
                    }
                }

                const ftk::ImageInfo& getInfo() const
                {
                    return _info;
                }

                io::VideoData read(
                    const std::string& fileName,
                    const OTIO_NS::RationalTime& time)
                {
                    io::VideoData out;
                    out.time = time;
                    out.image = ftk::Image::create(_info);

                    const int channels = ftk::getChannelCount(_info.type);
                    const size_t bytes = ftk::getBitDepth(_info.type) / 8;
                    
                    stbi_set_flip_vertically_on_load(1);

                    int x = 0, y = 0, n = 1;
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
                ftk::ImageInfo _info;
                const ftk::InMemoryFile* _memory;
            };
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
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
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const ftk::InMemoryFile* memory)
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
            const ftk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            return File(fileName, memory).read(fileName, time);
        }
    }
}
