// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/FFmpegTest.h>

#include <tlIO/FFmpeg.h>
#include <tlIO/System.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/FileIO.h>

#include <array>
#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        FFmpegTest::FFmpegTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "io_tests::FFmpegTest")
        {}

        std::shared_ptr<FFmpegTest> FFmpegTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<FFmpegTest>(new FFmpegTest(context));
        }

        void FFmpegTest::run()
        {
            _io();
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const file::Path& path,
                const ftk::ImageInfo& imageInfo,
                const ftk::ImageTags& tags,
                const OTIO_NS::RationalTime& duration,
                const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), duration);
                info.tags = tags;
                auto write = plugin->write(path, info, options);
                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                {
                    write->writeVideo(OTIO_NS::RationalTime(i, 24.0), image);
                }
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const ftk::ImageTags& tags,
                const OTIO_NS::RationalTime& duration,
                const Options& options)
            {
                std::vector<uint8_t> memoryData;
                std::vector<ftk::InMemoryFile> memory;
                std::shared_ptr<io::IRead> read;
                if (memoryIO)
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(ftk::InMemoryFile(memoryData.data(), memoryData.size()));
                    read = plugin->read(path, memory, options);
                }
                else
                {
                    read = plugin->read(path, options);
                }
                const auto ioInfo = read->getInfo().get();
                FTK_ASSERT(!ioInfo.video.empty());
                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                {
                    const auto videoData = read->readVideo(OTIO_NS::RationalTime(i, 24.0)).get();
                    FTK_ASSERT(videoData.image);
                    FTK_ASSERT(videoData.image->getSize() == image->getSize());
                    const auto frameTags = videoData.image->getTags();
                    for (const auto& j : tags)
                    {
                        const auto k = frameTags.find(j.first);
                        FTK_ASSERT(k != frameTags.end());
                        FTK_ASSERT(k->second == j.second);
                    }
                }
                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                {
                    const auto videoData = read->readVideo(OTIO_NS::RationalTime(i, 24.0)).get();
                }
            }

            void readError(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const Options& options)
            {
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    ftk::truncateFile(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<ftk::InMemoryFile> memory;
                if (memoryIO)
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(ftk::InMemoryFile(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory, options);
                //! \bug This causes the test to hang.
                //const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
            }
        }

        void FFmpegTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<ffmpeg::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<ffmpeg::WritePlugin>();

            const ftk::ImageTags tags =
            {
                { "artist", "artist" },
                { "comment", "comment" },
                { "title", "title" }
            };
            const std::vector<std::string> fileNames =
            {
                "FFmpegTest",
                "大平原"
            };
            const std::vector<bool> memoryIOList =
            {
                false,
                true
            };
            const std::vector<ftk::Size2I> sizes =
            {
                ftk::Size2I(640, 480),
                ftk::Size2I(80, 60),
                ftk::Size2I(0, 0)
            };
            const std::vector<std::pair<std::string, std::string> > options =
            {
                { "FFmpeg/YUVToRGB", "1" },
                { "FFmpeg/ThreadCount", "1" },
                { "FFmpeg/RequestTimeout", "1" },
                { "FFmpeg/VideoBufferSize", "1" },
                { "FFmpeg/AudioBufferSize", "1/1" },
                { "FFmpeg/Codec", "mjpeg" },
                { "FFmpeg/Codec", "v210" },
                { "FFmpeg/Codec", "v410" }
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto pixelType : ftk::getImageTypeEnums())
                        {
                            for (const auto& option : options)
                            {
                                Options options;
                                options[option.first] = option.second;
                                const auto imageInfo = writePlugin->getInfo(ftk::ImageInfo(size, pixelType));
                                if (imageInfo.isValid())
                                {
                                    file::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << '_' << size << '_' << pixelType << ".mp4";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image = ftk::Image::create(imageInfo);
                                    image->zero();
                                    image->setTags(tags);
                                    const OTIO_NS::RationalTime duration(24.0, 24.0);
                                    try
                                    {
                                        write(writePlugin, image, path, imageInfo, tags, duration, options);
                                        read(readPlugin, image, path, memoryIO, tags, duration, options);
                                        readError(readPlugin, image, path, memoryIO, options);
                                    }
                                    catch (const std::exception& e)
                                    {
                                        _printError(e.what());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
