// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/OpenEXRTest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/OpenEXR.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        OpenEXRTest::OpenEXRTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::OpenEXRTest", context)
        {}

        std::shared_ptr<OpenEXRTest> OpenEXRTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<OpenEXRTest>(new OpenEXRTest(context));
        }

        void OpenEXRTest::run()
        {
            auto plugin = _context->getSystem<System>()->getPlugin<exr::Plugin>();
            const std::map<std::string, std::string> tags =
            {
                { "Chromaticities", "1.2 2.3 3.4 4.5 5.6 6.7 7.8 8.9" },
                { "White Luminance", "1.2" },
                { "X Density", "1.2" },
                { "Owner", "Owner" },
                { "Comments", "Comments" },
                { "Capture Date", "Capture Date" },
                { "UTC Offset", "1.2" },
                { "Longitude", "1.2" },
                { "Latitude", "1.2" },
                { "Altitude", "1.2" },
                { "Focus", "1.2" },
                { "Exposure Time", "1.2" },
                { "Aperture", "1.2" },
                { "ISO Speed", "1.2" },
                { "Keycode", "1:2:3:4:5" },
                { "Timecode", "01:02:03:04" }
            };
            for (const auto& fileName : std::vector<std::string>(
                {
                    "OpenEXRTest",
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
                                ss << fileName << '_' << size << '_' << pixelType << ".0.exr";
                                _print(ss.str());
                                path = file::Path(ss.str());
                            }
                            auto image = imaging::Image::create(imageInfo);
                            image->zero();
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
                                    TLRENDER_ASSERT(videoData.image->getInfo().size == image->getInfo().size);
                                    TLRENDER_ASSERT(videoData.image->getInfo().pixelAspectRatio == image->getInfo().pixelAspectRatio);
                                    TLRENDER_ASSERT(videoData.image->getInfo().pixelType == image->getInfo().pixelType);
                                    TLRENDER_ASSERT(videoData.image->getInfo().yuvRange == image->getInfo().yuvRange);
                                    TLRENDER_ASSERT(videoData.image->getInfo().layout == image->getInfo().layout);
                                    //! \todo Compare image data.
                                    //TLRENDER_ASSERT(0 == memcmp(
                                    //    videoData.image->getData(),
                                    //    image->getData(),
                                    //    image->getDataByteCount()));
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
