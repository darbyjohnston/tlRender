// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/DPXTest.h>

#include <tlCore/AVIOSystem.h>
#include <tlCore/Assert.h>
#include <tlCore/DPX.h>

#include <sstream>

namespace tl
{
    namespace CoreTest
    {
        DPXTest::DPXTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::DPXTest", context)
        {}

        std::shared_ptr<DPXTest> DPXTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<DPXTest>(new DPXTest(context));
        }

        void DPXTest::run()
        {
            _enums();
            _io();
        }

        void DPXTest::_enums()
        {
            _enum<dpx::Version>("Version", dpx::getVersionEnums);
            _enum<dpx::Endian>("Endian", dpx::getEndianEnums);
        }
        
        void DPXTest::_io()
        {
            auto plugin = _context->getSystem<avio::System>()->getPlugin<dpx::Plugin>();
            const std::map<std::string, std::string> tags =
            {
            };
            for (const auto& fileName : std::vector<std::string>(
                {
                    "DPXTest",
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
                                ss << fileName << '_' << size << '_' << pixelType << ".0.dpx";
                                _print(ss.str());
                                path = file::Path(ss.str());
                            }
                            auto image = imaging::Image::create(imageInfo);
                            image->setTags(tags);
                            try
                            {
                                {
                                    avio::Info info;
                                    info.video.push_back(imageInfo);
                                    info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                    info.tags = tags;
                                    auto write = plugin->write(path, info);
                                    write->writeVideo(otime::RationalTime(0.0, 24.0), image);
                                }
                                {
                                    auto read = plugin->read(path);
                                    const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();                                TLRENDER_ASSERT(videoData.image);
                                    TLRENDER_ASSERT(videoData.image->getInfo() == image->getInfo());
                                    TLRENDER_ASSERT(0 == memcmp(
                                        videoData.image->getData(),
                                        image->getData(),
                                        image->getDataByteCount()));
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
