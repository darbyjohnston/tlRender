// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PPMTest.h>

#include <tlCore/AVIOSystem.h>
#include <tlCore/Assert.h>
#include <tlCore/PPM.h>

#include <sstream>

namespace tl
{
    namespace CoreTest
    {
        PPMTest::PPMTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::PPMTest", context)
        {}

        std::shared_ptr<PPMTest> PPMTest::create(const std::shared_ptr<core::Context>& context)
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
            auto plugin = _context->getSystem<avio::System>()->getPlugin<ppm::Plugin>();
            const std::map<std::string, std::string> tags =
            {
            };
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
                    for (const auto& pixelType : plugin->getWritePixelTypes())
                    {
                        for (const auto& option : std::vector<std::pair<std::string, std::string> >(
                            {
                                { "ppm/Data", "Binary" },
                                { "ppm/Data", "ASCII" }
                            }))
                        {
                            file::Path path;
                            {
                                std::stringstream ss;
                                ss << fileName << '_' << size << '_' << pixelType << ".0.ppm";
                                _print(ss.str());
                                path = file::Path(ss.str());
                            }
                            auto imageInfo = imaging::Info(size, pixelType);
                            imageInfo.layout.alignment = plugin->getWriteAlignment(pixelType);
                            imageInfo.layout.endian = plugin->getWriteEndian();
                            auto image = imaging::Image::create(imageInfo);
                            image->setTags(tags);
                            try
                            {
                                {
                                    avio::Info info;
                                    info.video.push_back(imageInfo);
                                    info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                                    info.tags = tags;
                                    avio::Options options;
                                    options[option.first] = option.second;
                                    auto write = plugin->write(path, info, options);
                                    write->writeVideo(otime::RationalTime(0.0, 24.0), image);
                                }
                                auto read = plugin->read(path);
                                const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
                                const auto frameTags = videoData.image->getTags();
                                for (const auto& j : tags)
                                {
                                    const auto k = frameTags.find(j.first);
                                    TLRENDER_ASSERT(k != frameTags.end());
                                    TLRENDER_ASSERT(k->second == j.second);
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
