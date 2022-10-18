// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/TIFFTest.h>

#include <tlIO/IOSystem.h>
#include <tlIO/TIFF.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        TIFFTest::TIFFTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_test::TIFFTest", context)
        {}

        std::shared_ptr<TIFFTest> TIFFTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TIFFTest>(new TIFFTest(context));
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<imaging::Image>& image,
                const file::Path& path,
                const imaging::Info& imageInfo,
                const imaging::Tags& tags)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                info.tags = tags;
                auto write = plugin->write(path, info);
                write->writeVideo(otime::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<imaging::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const imaging::Tags& tags)
            {
                std::vector<uint8_t> memoryData;
                std::vector<file::MemoryRead> memory;
                if (memoryIO)
                {
                    auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(file::MemoryRead(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory);
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

            void readError(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<imaging::Image>& image,
                const file::Path& path,
                bool memoryIO)
            {
                {
                    auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    file::truncate(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<file::MemoryRead> memory;
                if (memoryIO)
                {
                    auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(file::MemoryRead(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory);
                const auto videoData = read->readVideo(otime::RationalTime(0.0, 24.0)).get();
            }
        }

        void TIFFTest::run()
        {
            auto plugin = _context->getSystem<System>()->getPlugin<tiff::Plugin>();

            const imaging::Tags tags =
            {
                { "Creator", "Creator" },
                { "Description", "Description" },
                { "Copyright", "Copyright" },
                { "Time", "Time" }
            };
            const std::vector<std::string> fileNames =
            {
                "TIFFTest",
                "大平原"
            };
            const std::vector<bool> memoryIOList =
            {
                false,
                true
            };
            const std::vector<imaging::Size> sizes =
            {
                imaging::Size(16, 16),
                imaging::Size(1, 1),
                imaging::Size(0, 0)
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto& pixelType : imaging::getPixelTypeEnums())
                        {
                            const auto imageInfo = plugin->getWriteInfo(imaging::Info(size, pixelType));
                            if (imageInfo.isValid())
                            {
                                file::Path path;
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.tif";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = imaging::Image::create(imageInfo);
                                image->zero();
                                image->setTags(tags);
                                try
                                {
                                    write(plugin, image, path, imageInfo, tags);
                                    read(plugin, image, path, memoryIO, tags);
                                    readError(plugin, image, path, memoryIO);
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
