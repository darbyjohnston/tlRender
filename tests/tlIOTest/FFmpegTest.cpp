// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/FFmpegTest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/FFmpeg.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <array>
#include <sstream>

using namespace tl::core;
using namespace tl::io;

namespace tl
{
    namespace tests
    {
        namespace io_test
        {
            FFmpegTest::FFmpegTest(const std::shared_ptr<core::Context>& context) :
                ITest("io_test::FFmpegTest", context)
            {}

            std::shared_ptr<FFmpegTest> FFmpegTest::create(const std::shared_ptr<core::Context>& context)
            {
                return std::shared_ptr<FFmpegTest>(new FFmpegTest(context));
            }

            void FFmpegTest::run()
            {
                _enums();
                _util();
                _io();
            }

            void FFmpegTest::_enums()
            {
                _enum<ffmpeg::Profile>("Profile", ffmpeg::getProfileEnums);
            }

            void FFmpegTest::_util()
            {
                {
                    const AVRational r = { 1, 2 };
                    const AVRational rs = ffmpeg::swap(r);
                    TLRENDER_ASSERT(r.num == rs.den);
                    TLRENDER_ASSERT(r.den == rs.num);
                }
            }

            void FFmpegTest::_io()
            {
                auto plugin = _context->getSystem<System>()->getPlugin<ffmpeg::Plugin>();
                const std::map<std::string, std::string> tags =
                {
                    { "artist", "artist" },
                    { "comment", "comment" },
                    { "title", "title" }
                };
                for (const auto& fileName : std::vector<std::string>(
                    {
                        "FFmpegTest",
                        "大平原"
                    }))
                {
                    for (const auto& size : std::vector<imaging::Size>(
                        {
                            imaging::Size(16, 16),
                            imaging::Size(1, 1),
                            imaging::Size(0, 0)
                        }))
                    {
                        for (const auto& pixelType : imaging::getPixelTypeEnums())
                        {
                            auto imageInfo = plugin->getWriteInfo(imaging::Info(size, pixelType));
                            if (imageInfo.isValid())
                            {
                                file::Path path;
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".mov";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = imaging::Image::create(imageInfo);
                                image->setTags(tags);
                                const otime::RationalTime duration(24.0, 24.0);
                                try
                                {
                                    {
                                        Info info;
                                        info.video.push_back(imageInfo);
                                        info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), duration);
                                        info.tags = tags;
                                        auto write = plugin->write(path, info);
                                        for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                                        {
                                            write->writeVideo(otime::RationalTime(i, 24.0), image);
                                        }
                                    }
                                    {
                                        auto read = plugin->read(path);
                                        for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                                        {
                                            const auto videoData = read->readVideo(otime::RationalTime(i, 24.0)).get();
                                            TLRENDER_ASSERT(videoData.image);
                                            //! \todo Compare image information.
                                            //TLRENDER_ASSERT(videoData.image->getInfo() == image->getInfo());
                                            //std::stringstream ss;
                                            //ss << "Video time: " << videoData.time;
                                            //_print(ss.str());
                                            const auto frameTags = videoData.image->getTags();
                                            for (const auto& j : tags)
                                            {
                                                const auto k = frameTags.find(j.first);
                                                TLRENDER_ASSERT(k != frameTags.end());
                                                TLRENDER_ASSERT(k->second == j.second);
                                            }
                                        }
                                        for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                                        {
                                            const auto videoData = read->readVideo(otime::RationalTime(i, 24.0)).get();
                                            //std::stringstream ss;
                                            //ss << "Video time: " << videoData.time;
                                            //_print(ss.str());
                                        }
                                    }
                                    {
                                        auto io = file::FileIO::create();
                                        io->open(path.get(), file::Mode::Read);
                                        const size_t size = io->getSize();
                                        io->close();
                                        file::truncate(path.get(), size / 2);
                                        auto read = plugin->read(path);
                                        //! \bug This causes the test to hang.
                                        //const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
                                    }
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
