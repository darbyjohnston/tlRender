// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/SGITest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/SGI.h>

#include <tlCore/Assert.h>

#include <sstream>

using namespace tl::core;
using namespace tl::io;

namespace tl
{
    namespace tests
    {
        namespace io_test
        {
            SGITest::SGITest(const std::shared_ptr<system::Context>& context) :
                ITest("io_test::SGITest", context)
            {}

            std::shared_ptr<SGITest> SGITest::create(const std::shared_ptr<system::Context>& context)
            {
                return std::shared_ptr<SGITest>(new SGITest(context));
            }

            void SGITest::run()
            {
                _io();
            }

            void SGITest::_io()
            {
                auto plugin = _context->getSystem<System>()->getPlugin<sgi::Plugin>();
                for (const auto& fileName : std::vector<std::string>(
                    {
                        "SGITest",
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
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.sgi";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = imaging::Image::create(imageInfo);
                                try
                                {
                                    {
                                        Info info;
                                        info.video.push_back(imageInfo);
                                        info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                        auto write = plugin->write(path, info);
                                        _print(path.get());
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
}
