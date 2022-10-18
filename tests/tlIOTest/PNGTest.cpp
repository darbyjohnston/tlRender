// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIOTest/PNGTest.h>

#include <tlIO/IOSystem.h>

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
            
            const std::vector<std::string> fileNames =
            {
                "PNGTest",
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
            const auto pixelTypes = imaging::getPixelTypeEnums();

            for (const auto& fileName : fileNames)
            {
                for (auto memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto& pixelType : pixelTypes)
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
                                image->zero();
                                try
                                {
                                    _write(plugin, image, path, imageInfo);
                                    _read(plugin, image, path, memoryIO);
                                    _readError(plugin, image, path, memoryIO);
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

        void PNGTest::_write(
            const std::shared_ptr<png::Plugin>& plugin,
            const std::shared_ptr<imaging::Image>& image,
            const file::Path& path,
            const imaging::Info& imageInfo)
        {
            Info info;
            info.video.push_back(imageInfo);
            info.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
            auto write = plugin->write(path, info);
            write->writeVideo(otime::RationalTime(0.0, 24.0), image);
        }

        void PNGTest::_read(
            const std::shared_ptr<png::Plugin>& plugin,
            const std::shared_ptr<imaging::Image>& image,
            const file::Path& path,
            bool memoryIO)
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
        }

        void PNGTest::_readError(
            const std::shared_ptr<png::Plugin>& plugin,
            const std::shared_ptr<imaging::Image>& image,
            const file::Path& path,
            bool memoryIO)
        {
            auto io = file::FileIO::create(path.get(), file::Mode::Read);
            const size_t size = io->getSize();
            io.reset();
            file::truncate(path.get(), size / 2);
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
}
