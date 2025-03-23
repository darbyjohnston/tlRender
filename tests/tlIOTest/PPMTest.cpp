// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/PPMTest.h>

#include <tlIO/Cache.h>
#include <tlIO/PPM.h>
#include <tlIO/System.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        PPMTest::PPMTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "io_tests::PPMTest")
        {}

        std::shared_ptr<PPMTest> PPMTest::create(const std::shared_ptr<dtk::Context>& context)
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

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                const dtk::ImageInfo& imageInfo,
                const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                auto write = plugin->write(path, info, options);
                write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const Options& options)
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
                    read = plugin->read(path, memory, options);
                }
                else
                {
                    read = plugin->read(path, options);
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
            }

            void readError(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const Options& options)
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
                auto read = plugin->read(path, memory, options);
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
            }
        }

        void PPMTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<ppm::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<ppm::WritePlugin>();

            const std::vector<std::string> fileNames =
            {
                "PPMTest",
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
            const std::vector<std::pair<std::string, std::string> > options =
            {
                { "PPM/Data", "Binary" },
                { "PPM/Data", "ASCII" }
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto pixelType : dtk::getImageTypeEnums())
                        {
                            for (const auto& option : options)
                            {
                                Options options;
                                options[option.first] = option.second;
                                const auto imageInfo = writePlugin->getInfo(dtk::ImageInfo(size, pixelType), options);
                                if (imageInfo.isValid())
                                {
                                    file::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << '_' << size << '_' << pixelType << ".0.ppm";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image = dtk::Image::create(imageInfo);
                                    image->zero();
                                    try
                                    {
                                        write(writePlugin, image, path, imageInfo, options);
                                        read(readPlugin, image, path, memoryIO, options);
                                        readError(readPlugin, image, path, memoryIO, options);
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
}
