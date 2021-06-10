// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TIFFTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/TIFF.h>

#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        TIFFTest::TIFFTest() :
            ITest("CoreTest::TIFFTest")
        {}

        std::shared_ptr<TIFFTest> TIFFTest::create()
        {
            return std::shared_ptr<TIFFTest>(new TIFFTest);
        }

        void TIFFTest::run()
        {
            auto plugin = tiff::Plugin::create();
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
                        ss << size << '_' << pixelType << ".0.tif";
                        fileName = ss.str();
                        _print(fileName);
                    }

                    const auto imageInfo = imaging::Info(size, pixelType);
                    const auto imageWrite = imaging::Image::create(imageInfo);

                    io::Info info;
                    info.video.push_back(io::VideoInfo(imageInfo, otime::RationalTime(1.0, 24.0)));
                    auto write = plugin->write(fileName, info);
                    write->writeVideoFrame(otime::RationalTime(0.0, 24.0), imageWrite);

                    auto read = plugin->read(fileName);
                    const auto imageRead = read->readVideoFrame(otime::RationalTime(0.0, 24.0)).get();
                }
            }
        }
    }
}
