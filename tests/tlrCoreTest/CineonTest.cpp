// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/CineonTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Cineon.h>

#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        CineonTest::CineonTest() :
            ITest("CoreTest::CineonTest")
        {}

        std::shared_ptr<CineonTest> CineonTest::create()
        {
            return std::shared_ptr<CineonTest>(new CineonTest);
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
            auto plugin = cineon::Plugin::create();
            const std::map<std::string, std::string> tags =
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
            for (const auto& size : std::vector<imaging::Size>(
                {
                    imaging::Size(16, 16),
                    imaging::Size(1, 1),
                    imaging::Size(0, 0)
                }))
            {
                for (const auto& pixelType : plugin->getWritePixelTypes())
                {
                    std::string fileName;
                    {
                        std::stringstream ss;
                        ss << "CineonTest_" << size << '_' << pixelType << ".0.cin";
                        fileName = ss.str();
                        _print(fileName);
                    }
                    auto imageInfo = imaging::Info(size, pixelType);
                    imageInfo.layout.alignment = plugin->getWriteAlignment();
                    imageInfo.layout.endian = plugin->getWriteEndian();
                    auto imageWrite = imaging::Image::create(imageInfo);
                    imageWrite->setTags(tags);
                    try
                    {
                        {
                            avio::Info info;
                            info.video.push_back(imageInfo);
                            info.videoDuration = otime::RationalTime(1.0, 24.0);
                            auto write = plugin->write(fileName, info);
                            write->writeVideoFrame(otime::RationalTime(0.0, 24.0), imageWrite);
                        }
                        auto read = plugin->read(fileName);
                        const auto imageRead = read->readVideoFrame(otime::RationalTime(0.0, 24.0)).get();
                        for (const auto& i : tags)
                        {
                            std::stringstream ss;
                            ss << "1 " << i.first << ": " << i.second;
                            _print(ss.str());
                        }
                        for (const auto& i : imageRead.image->getTags())
                        {
                            std::stringstream ss;
                            ss << "2 " << i.first << ": " << i.second;
                            _print(ss.str());
                        }
                        TLR_ASSERT(imageRead.image->getTags() == tags);
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
