// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/OpenEXRTest.h>

#include <tlIO/Cache.h>
#include <tlIO/OpenEXR.h>
#include <tlIO/System.h>

#include <dtk/core/Assert.h>
#include <dtk/core/FileIO.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        OpenEXRTest::OpenEXRTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "io_tests::OpenEXRTest")
        {}

        std::shared_ptr<OpenEXRTest> OpenEXRTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<OpenEXRTest>(new OpenEXRTest(context));
        }

        void OpenEXRTest::run()
        {
            _enums();
            _util();
            _io();
        }

        void OpenEXRTest::_enums()
        {
            _enum<exr::Compression>("Compression", exr::getCompressionEnums);
        }

        void OpenEXRTest::_util()
        {
            {
                const std::set<std::string> data =
                {
                    "R",
                    ".G",
                    "B.",
                    "A",
                    "diffuse.R",
                    "diffuse.left.R"
                };
                const auto defaultChannels = exr::getDefaultChannels(data);
                const std::set<std::string> result = { ".G", "A", "B.", "R" };
                DTK_ASSERT(defaultChannels == result);
            }
            {
                std::vector<std::string> data = { "A", "b", "g", "r" };
                exr::reorderChannels(data);
                const std::vector<std::string> result = { "r", "g", "b", "A" };
                DTK_ASSERT(data = result);
            }
            {
                std::vector<std::string> data = { "z", "b", "G", "r" };
                exr::reorderChannels(data);
                const std::vector<std::string> result = { "r", "G", "b", "z" };
                DTK_ASSERT(data == result);
            }
            {
                std::vector<std::string> data = { "diffuse.B", "diffuse.G", "diffuse.R" };
                exr::reorderChannels(data);
                const std::vector<std::string> result = { "diffuse.R", "diffuse.G", "diffuse.B" };
                DTK_ASSERT(data == result);
            }
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                const dtk::ImageInfo& imageInfo,
                const dtk::ImageTags& tags,
                const Options& options)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                info.tags = tags;
                auto write = plugin->write(path, info, options);
                write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<dtk::Image>& image,
                const file::Path& path,
                bool memoryIO,
                const dtk::ImageTags& tags,
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
                const auto frameTags = videoData.image->getTags();
                for (const auto& j : frameTags)
                {
                    const auto k = tags.find(j.first);
                    if (k != tags.end())
                    {
                        DTK_ASSERT(k->second == j.second);
                    }
                }
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

        void OpenEXRTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<exr::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<exr::WritePlugin>();

            const dtk::ImageTags tags =
            {
                //{ "Name", "Name" },
                //{ "Type", "scanlineimage" },
                //{ "Version", "1" },
                //{ "Chunk Count", "1" },
                //{ "View", "View" },
                //{ "Tile", "1 2 1 1" },
                { "AdoptedNeutral", "0 1" },
                { "Altitude", "1" },
                { "Aperture", "1" },
                { "AscFramingDecisionList", "AscFramingDecisionList" },
                { "CameraCCTSetting", "1" },
                { "CameraColorBalance", "1 2" },
                { "CameraFirmwareVersion", "CameraFirmwareVersion" },
                { "CameraLabel", "CameraLabel" },
                { "CameraMake", "CameraMake" },
                { "CameraModel", "CameraModel" },
                { "CameraSerialNumber", "CameraSerialNumber" },
                { "CameraTintSetting", "1" },
                { "CameraTintSetting", "CameraTintSetting" },
                { "CapDate", "CapDate" },
                { "CaptureRate", "24 1" },
                { "Chromaticities", "0 1 2 3 4 5 6 7" },
                { "Comments", "Comments" },
                { "EffectiveFocalLength", "1" },
                { "EntrancePupilOffset", "1" },
                { "Envmap", "1" },
                { "ExpTime", "1" },
                { "Focus", "1" },
                { "FramesPerSecond", "24 1" },
                { "ImageCounter", "1" },
                { "IsoSpeed", "1" },
                { "KeyCode", "1:2:3:4:5:6:20" },
                { "Latitude", "1" },
                { "LensFirmwareVersion", "LensFirmwareVersion" },
                { "LensMake", "LensMake" },
                { "LensModel", "LensModel" },
                { "LensSerialNumber", "LensSerialNumber" },
                { "Longitude", "1" },
                { "NominalFocalLength", "1" },
                { "OriginalDataWindow", "0 1 2 3" },
                { "Owner", "Owner" },
                { "PinholeFocalLength", "1" },
                { "ReelName", "ReelName" },
                { "SensorAcquisitionRectangle", "0 1 2 3" },
                { "SensorCenterOffset", "0 1" },
                { "SensorPhotositePitch", "1" },
                { "ShutterAngle", "1" },
                { "TStop", "1" },
                { "TimeCode", "01:00:00:00" },
                { "UtcOffset", "1" },
                { "WhiteLuminance", "1" },
                { "WorldToCamera", "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15" },
                { "WorldToNDC", "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15" },
                { "XDensity", "1" },

                { "Wrapmodes", "Wrapmodes" },
                { "MultiView", "5:hello0:5:world" },
                { "DeepImageState", "1" },

                /*{ "X Density", "1" },
                { "Owner", "Owner" },
                { "Comments", "Comments" },
                { "Capture Date", "Capture Date" },
                { "UTC Offset", "1" },
                { "Longitude", "1" },
                { "Latitude", "1" },
                { "Altitude", "1" },
                { "Focus", "1" },
                { "Exposure Time", "1" },
                { "Aperture", "1" },
                { "ISO Speed", "1" },
                { "Environment Map", "1" },
                { "Keycode", "1 2 3 4 5 6 7" },
                { "Timecode", "01:02:03:04" },
                { "Wrap Modes", "Wrap Modes" }*/
            };
            const std::vector<std::string> fileNames =
            {
                "OpenEXRTest",
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
                { "OpenEXR/ChannelGrouing", "None" },
                { "OpenEXR/ChannelGrouing", "Known" },
                { "OpenEXR/ChannelGrouing", "All" },
                { "OpenEXR/Compression", "None" },
                { "OpenEXR/Compression", "RLE" },
                { "OpenEXR/Compression", "ZIPS" },
                { "OpenEXR/Compression", "ZIP" },
                { "OpenEXR/Compression", "PIZ" },
                { "OpenEXR/Compression", "PXR24" },
                { "OpenEXR/Compression", "B44" },
                { "OpenEXR/Compression", "B44A" },
                { "OpenEXR/Compression", "DWAA" },
                { "OpenEXR/Compression", "DWAB" },
                { "OpenEXR/DWACompressionLevel", "45" },
                { "OpenEXR/DWACompressionLevel", "100" }
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
                                const auto imageInfo = writePlugin->getInfo(dtk::ImageInfo(size, pixelType));
                                if (imageInfo.isValid())
                                {
                                    file::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << '_' << size << '_' << pixelType << ".0.exr";
                                        _print(ss.str());
                                        path = file::Path(ss.str());
                                    }
                                    auto image = dtk::Image::create(imageInfo);
                                    image->zero();
                                    image->setTags(tags);
                                    try
                                    {
                                        write(writePlugin, image, path, imageInfo, tags, options);
                                        read(readPlugin, image, path, memoryIO, tags, options);
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
