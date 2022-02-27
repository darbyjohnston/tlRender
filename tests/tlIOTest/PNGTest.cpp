// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/PNGTest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/PNG.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        PNGTest::PNGTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::PNGTest", context)
        {}

        std::shared_ptr<PNGTest> PNGTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PNGTest>(new PNGTest(context));
        }

        void PNGTest::run()
        {
            auto plugin = _context->getSystem<System>()->getPlugin<png::Plugin>();
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
                    for (const auto& pixelType : imaging::getPixelTypeEnums())
                    {
                        auto imageInfo = plugin->getWriteInfo(imaging::Info(size, pixelType));
                        if (imageInfo.isValid())
                        {
                            file::Path path;
                            {
                                std::stringstream ss;
                                ss << fileName << '_' << size << '_' << pixelType << ".0.png";
                                _print(ss.str());
                                path = file::Path(ss.str());
                            }
                            const auto image = imaging::Image::create(imageInfo);
                            try
                            {
                                {
                                    Info info;
                                    info.video.push_back(imageInfo);
                                    info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                    auto write = plugin->write(path, info);
                                    write->writeVideo(otime::RationalTime(0.0, 24.0), image);
                                }
                                {
                                    auto read = plugin->read(path);
                                    const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
                                    TLRENDER_ASSERT(videoData.image);
                                    TLRENDER_ASSERT(videoData.image->getInfo() == image->getInfo());
                                    //! \todo Compare image data.
                                    //TLRENDER_ASSERT(0 == memcmp(
                                    //    videoData.image->getData(),
                                    //    image->getData(),
                                    //    image->getDataByteCount()));
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
