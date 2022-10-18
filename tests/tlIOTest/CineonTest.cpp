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

            const imaging::Tags tags =
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
            const std::vector<imaging::Size> sizes =
            {
                imaging::Size(16, 16),
                imaging::Size(1, 1),
                imaging::Size(0, 0)
            };

            for (const auto& fileName : fileNames)
            {
                for (const auto memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
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
                                    _write(plugin, image, path, imageInfo, tags);
                                    _read(plugin, image, path, memoryIO, tags);
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

        void CineonTest::_write(
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

        void CineonTest::_read(
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

        void CineonTest::_readError(
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
}
