// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/CineonTest.h>

#include <tlIO/Cache.h>
#include <tlIO/Cineon.h>
#include <tlIO/System.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        CineonTest::CineonTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "io_tests::CineonTest")
        {}

        std::shared_ptr<CineonTest> CineonTest::create(const std::shared_ptr<dtk::Context>& context)
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

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                const dtk::ImageInfo& imageInfo,
                const dtk::ImageTags& tags)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                info.tags = tags;
                auto write = plugin->write(path, info);
                write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const dtk::ImageTags& tags)
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
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
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

        void CineonTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<cineon::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<cineon::WritePlugin>();

            const dtk::ImageTags tags =
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
            const std::vector<std::string> fileNames =
            {
                "CineonTest",
                "大平原"
            };
            const std::vector<bool> memoryIOList =
            {
                false,
                true
            };
            const std::vector<dtk::Size2I> sizes =
            {
                dtk::Size2I(16, 16),
                dtk::Size2I(1, 1),
                dtk::Size2I(0, 0)
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto pixelType : dtk::getImageTypeEnums())
                        {
                            const auto imageInfo = writePlugin->getInfo(dtk::ImageInfo(size, pixelType));
                            if (imageInfo.isValid())
                            {
                                file::Path path;
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.cin";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = dtk::Image::create(imageInfo);
                                image->zero();
                                image->setTags(tags);
                                try
                                {
                                    write(writePlugin, image, path, imageInfo, tags);
                                    read(readPlugin, image, path, memoryIO, tags);
                                    readSystem->getCache()->clear();
                                    readError(readPlugin, image, path, memoryIO);
                                    readSystem->getCache()->clear();
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
