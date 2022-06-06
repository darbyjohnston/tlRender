// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/PPMTest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/PPM.h>

#include <tlCore/Assert.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        PPMTest::PPMTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::PPMTest", context)
        {}

        std::shared_ptr<PPMTest> PPMTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PPMTest>(new PPMTest(context));
        }

        void PPMTest::run()
        {
            _enums();
            _io();
        }

        void PPMTest::_enums()
        {
            _enum<ppm::Data>("Data", ppm::getDataEnums);
        }

        void PPMTest::_io()
        {
            auto plugin = _context->getSystem<System>()->getPlugin<ppm::Plugin>();
            for (const auto& fileName : std::vector<std::string>(
                {
                    "PPMTest",
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
                        for (const auto& option : std::vector<std::pair<std::string, std::string> >(
                            {
                                { "ppm/Data", "Binary" },
                                { "ppm/Data", "ASCII" }
                            }))
                        {
                            Options options;
                            options[option.first] = option.second;
                            auto imageInfo = plugin->getWriteInfo(imaging::Info(size, pixelType), options);
                            if (imageInfo.isValid())
                            {
                                file::Path path;
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.ppm";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = imaging::Image::create(imageInfo);
                                image->zero();
                                try
                                {
                                    {
                                        Info info;
                                        info.video.push_back(imageInfo);
                                        info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                        auto write = plugin->write(path, info, options);
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
