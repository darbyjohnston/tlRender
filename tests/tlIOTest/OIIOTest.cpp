// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIOTest/OIIOTest.h>

#include <tlIO/OIIO.h>
#include <tlIO/System.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        OIIOTest::OIIOTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "io_tests::OIIOTest")
        {
        }

        std::shared_ptr<OIIOTest> OIIOTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<OIIOTest>(new OIIOTest(context));
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const file::Path& path,
                const ftk::ImageInfo& imageInfo,
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
                const std::shared_ptr<ftk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const Options& options)
            {
                std::vector<uint8_t> memoryData;
                std::vector<ftk::InMemoryFile> memory;
                std::shared_ptr<io::IRead> read;
                if (memoryIO)
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(ftk::InMemoryFile(memoryData.data(), memoryData.size()));
                    read = plugin->read(path, memory, options);
                }
                else
                {
                    read = plugin->read(path, options);
                }
                const auto ioInfo = read->getInfo().get();
                FTK_ASSERT(!ioInfo.video.empty());
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
                FTK_ASSERT(videoData.image);
                FTK_ASSERT(videoData.image->getSize() == image->getSize());
                //! \todo Compare image data.
                //FTK_ASSERT(0 == memcmp(
                //    videoData.image->getData(),
                //    image->getData(),
                //    image->getDataByteCount()));
            }

            void readError(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const Options& options)
            {
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    ftk::truncateFile(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<ftk::InMemoryFile> memory;
                if (memoryIO)
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(ftk::InMemoryFile(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory, options);
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
            }
        }

        void OIIOTest::run()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<oiio::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<oiio::WritePlugin>();

            const std::vector<std::string> fileNames =
            {
                "OIIOTest",
                "大平原"
            };
            const std::vector<std::string> extensions =
            {
                ".png",
                ".exr"
            };
            const std::vector<bool> memoryIOList =
            {
                false,
                true
            };
            const std::vector<ftk::Size2I> sizes =
            {
                ftk::Size2I(16, 16),
                ftk::Size2I(1, 1),
                ftk::Size2I(0, 0)
            };
            const std::vector<io::Options> optionsList =
            {
                {},
                {
                    { "OpenEXR/Compression", "none" }
                },
                {
                    { "OpenEXR/Compression", "zip" }
                },
                {
                    { "OpenEXR/Compression", "dwaa" },
                    { "OpenEXR/DWACompressionLevel", "50" },
                }
            };
            size_t count = 0;
            for (const auto& fileName : fileNames)
            {
                for (const auto& extension : extensions)
                {
                    for (const bool memoryIO : memoryIOList)
                    {
                        for (const auto& size : sizes)
                        {
                            for (const auto pixelType : ftk::getImageTypeEnums())
                            {
                                for (const auto& options : optionsList)
                                {
                                    const auto imageInfo = writePlugin->getInfo(ftk::ImageInfo(size, pixelType));
                                    if (imageInfo.isValid())
                                    {
                                        file::Path path;
                                        {
                                            std::stringstream ss;
                                            ss << fileName << '_' << count << '_' << size.w << "x" << size.h << '_' << pixelType << ".0" << extension;
                                            _print(ss.str());
                                            path = file::Path(ss.str());
                                        }
                                        const auto image = ftk::Image::create(imageInfo);
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

                                        ++count;
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
