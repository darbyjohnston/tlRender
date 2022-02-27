// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/JPEGTest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/JPEG.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        JPEGTest::JPEGTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::JPEGTest", context)
        {}

        std::shared_ptr<JPEGTest> JPEGTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<JPEGTest>(new JPEGTest(context));
        }

        void JPEGTest::run()
        {
            auto plugin = _context->getSystem<System>()->getPlugin<jpeg::Plugin>();
            const std::map<std::string, std::string> tags =
            {
                { "Description", "Description" }
            };
            for (const auto& fileName : std::vector<std::string>(
                {
                    "JPEGTest",
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
                                ss << fileName << '_' << size << '_' << pixelType << ".0.jpg";
                                _print(ss.str());
                                path = file::Path(ss.str());
                            }
                            auto image = imaging::Image::create(imageInfo);
                            image->setTags(tags);
                            try
                            {
                                {
                                    Info info;
                                    info.video.push_back(imageInfo);
                                    info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                    info.tags = tags;
                                    auto write = plugin->write(path, info);
                                    write->writeVideo(otime::RationalTime(0.0, 24.0), image);
                                }
                                {
                                    auto read = plugin->read(path);
                                    const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
                                    TLRENDER_ASSERT(videoData.image);
                                    TLRENDER_ASSERT(videoData.image->getInfo() == image->getInfo());
                                    const auto frameTags = videoData.image->getTags();
                                    for (const auto& j : tags)
                                    {
                                        const auto k = frameTags.find(j.first);
                                        TLRENDER_ASSERT(k != frameTags.end());
                                        TLRENDER_ASSERT(k->second == j.second);
                                    }
                                }
                                {
                                    auto io = file::FileIO::create();
                                    io->open(path.get(), file::Mode::Read);
                                    const size_t size = io->getSize();
                                    io->close();
                                    file::truncate(path.get(), size / 2);
                                    auto read = plugin->read(path);
                                    const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
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
