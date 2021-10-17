// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/FFmpegTest.h>

#include <tlrCore/AVIOSystem.h>
#include <tlrCore/Assert.h>
#include <tlrCore/FFmpeg.h>

#include <array>
#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        FFmpegTest::FFmpegTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::FFmpegTest", context)
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
                TLR_ASSERT(r.num == rs.den);
                TLR_ASSERT(r.den == rs.num);
            }
        }

        void FFmpegTest::_io()
        {
            auto plugin = _context->getSystem<avio::System>()->getPlugin<ffmpeg::Plugin>();
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
                    for (const auto& pixelType : plugin->getWritePixelTypes())
                    {
                        file::Path path;
                        {
                            std::stringstream ss;
                            ss << fileName << '_' << size << '_' << pixelType << ".mov";
                            _print(ss.str());
                            path = file::Path(ss.str());
                        }
                        auto imageInfo = imaging::Info(size, pixelType);
                        imageInfo.layout.alignment = plugin->getWriteAlignment(pixelType);
                        imageInfo.layout.endian = plugin->getWriteEndian();
                        const otime::RationalTime duration(24.0, 24.0);
                        try
                        {
                            {
                                avio::Info info;
                                info.video.push_back(imageInfo);
                                info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), duration);
                                info.tags = tags;
                                auto write = plugin->write(path, info);
                                auto image = imaging::Image::create(imageInfo);
                                image->setTags(tags);
                                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                                {
                                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                                }
                            }
                            auto read = plugin->read(path);
                            for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                            {
                                const auto videoData = read->readVideo(otime::RationalTime(i, 24.0)).get();
                                //std::stringstream ss;
                                //ss << "Video time: " << videoData.time;
                                //_print(ss.str());
                                const auto frameTags = videoData.image->getTags();
                                for (const auto& j : tags)
                                {
                                    const auto k = frameTags.find(j.first);
                                    TLR_ASSERT(k != frameTags.end());
                                    TLR_ASSERT(k->second == j.second);
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
