// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/CineonTest.h>

#include <tlIO/Cineon.h>
#include <tlIO/IOSystem.h>

#include <tlCore/Assert.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        CineonTest::CineonTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::CineonTest", context)
        {}

        std::shared_ptr<CineonTest> CineonTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<CineonTest>(new CineonTest(context));
        }

        void CineonTest::run()
        {
            _enums();
            _io();
        }

        void CineonTest::_enums()
        {
            _enum<cineon::Orient>("Orient", cineon::getOrientEnums);
            _enum<cineon::Descriptor>("Descriptor", cineon::getDescriptorEnums);
        }

        void CineonTest::_io()
        {
            auto plugin = _context->getSystem<System>()->getPlugin<cineon::Plugin>();
            const std::map<std::string, std::string> tags =
            {
                { "Time", "Time" },
                { "Source Offset", "1 2" },
                { "Source File", "Source File" },
                { "Source Time", "Source Time" },
                { "Source Input Device", "Source Input Device" },
                { "Source Input Model", "Source Input Model" },
                { "Source Input Serial", "Source Input Serial" },
                { "Source Input Pitch", "1.2 3.4" },
                { "Source Gamma", "2.1" },
                { "Keycode", "1:2:3:4:5" },
                { "Film Format", "Film Format" },
                { "Film Frame", "24" },
                { "Film Frame Rate", "23.98" },
                { "Film Frame ID", "Film Frame ID" },
                { "Film Slate", "Film Slate" }
            };
            for (const auto& fileName : std::vector<std::string>(
                {
                    "CineonTest",
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
                                ss << fileName << '_' << size << '_' << pixelType << ".0.cin";
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
                                    TLRENDER_ASSERT(videoData.image->getSize() == image->getSize());
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
                                    auto io = file::FileIO::create(path.get(), file::Mode::Read);
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
