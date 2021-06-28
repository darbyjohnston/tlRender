// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/OpenEXRTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/OpenEXR.h>

#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        OpenEXRTest::OpenEXRTest() :
            ITest("CoreTest::OpenEXRTest")
        {}

        std::shared_ptr<OpenEXRTest> OpenEXRTest::create()
        {
            return std::shared_ptr<OpenEXRTest>(new OpenEXRTest);
        }

        void OpenEXRTest::run()
        {
            auto plugin = exr::Plugin::create();
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
                        ss << "OpenEXRTest_" << size << '_' << pixelType << ".0.exr";
                        fileName = ss.str();
                        _print(fileName);
                    }
                    const auto imageInfo = imaging::Info(size, pixelType);
                    const auto imageWrite = imaging::Image::create(imageInfo);
                    try
                    {
                        {
                            avio::Info info;
                            info.video.push_back(imageInfo);
                            info.videoDuration = otime::RationalTime(1.0, 24.0);
                            auto write = plugin->write(fileName, info);
                            write->writeVideoFrame(otime::RationalTime(0.0, 24.0), imageWrite);
                        }
                        auto read = plugin->read(fileName);
                        const auto imageRead = read->readVideoFrame(otime::RationalTime(0.0, 24.0)).get();
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
