// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/CineonTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Cineon.h>

#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        CineonTest::CineonTest() :
            ITest("CoreTest::CineonTest")
        {}

        std::shared_ptr<CineonTest> CineonTest::create()
        {
            return std::shared_ptr<CineonTest>(new CineonTest);
        }

        void CineonTest::run()
        {
            auto plugin = cineon::Plugin::create();
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
                        ss << "CineonTest_" << size << '_' << pixelType << ".0.cin";
                        fileName = ss.str();
                        _print(fileName);
                    }
                    auto imageInfo = imaging::Info(size, pixelType);
                    imageInfo.layout.alignment = plugin->getWriteAlignment();
                    imageInfo.layout.endian = plugin->getWriteEndian();
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
                    catch (const std::exception& e)
                    {
                        _printError(e.what());
                    }
                }
            }
        }
    }
}
