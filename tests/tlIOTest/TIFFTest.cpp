// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/TIFFTest.h>

#include <tlIO/System.h>
#include <tlIO/TIFF.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        TIFFTest::TIFFTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "io_test::TIFFTest")
        {}

        std::shared_ptr<TIFFTest> TIFFTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<TIFFTest>(new TIFFTest(context));
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path,
                const image::Info& imageInfo,
                const image::Tags& tags)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                info.tags = tags;
                auto write = plugin->write(path, info);
                write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const image::Tags& tags)
            {
                std::vector<uint8_t> memoryData;
                std::vector<dtk::InMemoryFile> memory;
                std::shared_ptr<io::IRead> read;
                if (memoryIO)
                {
                    auto fileIO = dtk::FileIO::create(path.get(), dtk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(dtk::InMemoryFile(memoryData.data(), memoryData.size()));
                    read = plugin->read(path, memory);
                }
                else
                {
                    read = plugin->read(path);
                }
                const auto ioInfo = read->getInfo().get();
                DTK_ASSERT(!ioInfo.video.empty());
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
                DTK_ASSERT(videoData.image);
                DTK_ASSERT(videoData.image->getSize() == image->getSize());
                //! \todo Compare image data.
                //DTK_ASSERT(0 == memcmp(
                //    videoData.image->getData(),
                //    image->getData(),
                //    image->getDataByteCount()));
                const auto frameTags = videoData.image->getTags();
                for (const auto& j : tags)
                {
                    const auto k = frameTags.find(j.first);
                    DTK_ASSERT(k != frameTags.end());
                    DTK_ASSERT(k->second == j.second);
                }
            }

            void readError(
                const std::shared_ptr<io::IPlugin>& plugin,
                const std::shared_ptr<image::Image>& image,
                const file::Path& path,
                bool memoryIO)
            {
                {
                    auto fileIO = dtk::FileIO::create(path.get(), dtk::FileMode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    dtk::truncateFile(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<dtk::InMemoryFile> memory;
                if (memoryIO)
                {
                    auto fileIO = dtk::FileIO::create(path.get(), dtk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(dtk::InMemoryFile(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory);
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
            }
        }

        void TIFFTest::run()
        {
            auto system = _context->getSystem<System>();
            auto plugin = system->getPlugin<tiff::Plugin>();

            const image::Tags tags =
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
            const std::vector<image::Size> sizes =
            {
                image::Size(16, 16),
                image::Size(1, 1),
                image::Size(0, 0)
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto& pixelType : image::getPixelTypeEnums())
                        {
                            const auto imageInfo = plugin->getWriteInfo(image::Info(size, pixelType));
                            if (imageInfo.isValid())
                            {
                                file::Path path;
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.tif";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = image::Image::create(imageInfo);
                                image->zero();
                                image->setTags(tags);
                                try
                                {
                                    write(plugin, image, path, imageInfo, tags);
                                    read(plugin, image, path, memoryIO, tags);
                                    system->getCache()->clear();
                                    readError(plugin, image, path, memoryIO);
                                    system->getCache()->clear();
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
