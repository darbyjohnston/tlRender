// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/JPEGTest.h>

#include <tlrCore/AVIOSystem.h>
#include <tlrCore/Assert.h>
#include <tlrCore/JPEG.h>

#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        JPEGTest::JPEGTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::JPEGTest", context)
        {}

        std::shared_ptr<JPEGTest> JPEGTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<JPEGTest>(new JPEGTest(context));
        }

        void JPEGTest::run()
        {
            auto plugin = _context->getSystem<avio::System>()->getPlugin<jpeg::Plugin>();
            const std::map<std::string, std::string> tags =
            {
                { "Description", "Description" }
            };
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
                        ss << "JPEGTest_" << size << '_' << pixelType << ".0.jpg";
                        _print(ss.str());
                        path = file::Path(ss.str());
                    }
                    auto imageInfo = imaging::Info(size, pixelType);
                    imageInfo.layout.alignment = plugin->getWriteAlignment(pixelType);
                    imageInfo.layout.endian = plugin->getWriteEndian();
                    auto image = imaging::Image::create(imageInfo);
                    image->setTags(tags);
                    try
                    {
                        {
                            avio::Info info;
                            info.video.push_back(imageInfo);
                            info.videoTimeRange = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                            info.tags = tags;
                            auto write = plugin->write(path, info);
                            write->writeVideoFrame(otime::RationalTime(0.0, 24.0), image);
                        }
                        auto read = plugin->read(path);
                        const auto videoFrame = read->readVideoFrame(otime::RationalTime(0.0, 24.0)).get();
                        const auto frameTags = videoFrame.image->getTags();
                        for (const auto& j : tags)
                        {
                            const auto k = frameTags.find(j.first);
                            TLR_ASSERT(k != frameTags.end());
                            TLR_ASSERT(k->second == j.second);
                        }
                    }
                    catch(const std::exception& e)
                    {
                        _printError(e.what());
                    }
                }
            }
        }
    }
}
