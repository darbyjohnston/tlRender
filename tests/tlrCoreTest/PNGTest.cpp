// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/PNGTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/PNG.h>

#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        PNGTest::PNGTest() :
            ITest("CoreTest::PNGTest")
        {}

        std::shared_ptr<PNGTest> PNGTest::create()
        {
            return std::shared_ptr<PNGTest>(new PNGTest);
        }

        void PNGTest::run()
        {
            auto plugin = png::Plugin::create();
            for (const auto& size : std::vector<imaging::Size>(
                {
                    imaging::Size(16, 16),
                    imaging::Size(1, 1),
                    imaging::Size(0, 0)
                }))
            {
                for (const auto& pixelType : plugin->getWritePixelTypes())
                {
                    std::string fileName;
                    {
                        std::stringstream ss;
                        ss << "PNGTest_" << size << '_' << pixelType << ".0.png";
                        fileName = ss.str();
                        _print(fileName);
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
                            info.videoDuration = otime::RationalTime(1.0, 24.0);
                            auto write = plugin->write(fileName, info);
                            write->writeVideoFrame(otime::RationalTime(0.0, 24.0), image);
                        }
                        auto read = plugin->read(fileName);
                        const auto videoFrame = read->readVideoFrame(otime::RationalTime(0.0, 24.0)).get();
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
