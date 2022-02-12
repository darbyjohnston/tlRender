// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PNGTest.h>

#include <tlCore/AVIOSystem.h>
#include <tlCore/Assert.h>
#include <tlCore/PNG.h>

#include <sstream>

namespace tl
{
    namespace CoreTest
    {
        PNGTest::PNGTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::PNGTest", context)
        {}

        std::shared_ptr<PNGTest> PNGTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<PNGTest>(new PNGTest(context));
        }

        void PNGTest::run()
        {
            auto plugin = _context->getSystem<avio::System>()->getPlugin<png::Plugin>();
            for (const auto& fileName : std::vector<std::string>(
                {
                    "PNGTest",
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
                            ss << fileName << '_' << size << '_' << pixelType << ".0.png";
                            _print(ss.str());
                            path = file::Path(ss.str());
                        }
                        auto imageInfo = imaging::Info(size, pixelType);
                        imageInfo.layout.alignment = plugin->getWriteAlignment(pixelType);
                        imageInfo.layout.endian = plugin->getWriteEndian();
                        const auto image = imaging::Image::create(imageInfo);
                        try
                        {
                            {
                                avio::Info info;
                                info.video.push_back(imageInfo);
                                info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                auto write = plugin->write(path, info);
                                write->writeVideo(otime::RationalTime(0.0, 24.0), image);
                            }
                            auto read = plugin->read(path);
                            const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
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
