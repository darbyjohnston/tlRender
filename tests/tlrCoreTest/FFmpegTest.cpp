// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/FFmpegTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/FFmpeg.h>

#include <array>
#include <sstream>

namespace tlr
{
    namespace CoreTest
    {
        FFmpegTest::FFmpegTest() :
            ITest("CoreTest::FFmpegTest")
        {}

        std::shared_ptr<FFmpegTest> FFmpegTest::create()
        {
            return std::shared_ptr<FFmpegTest>(new FFmpegTest);
        }

        void FFmpegTest::run()
        {
            _enums();
            _rational();
            _io();
        }

        void FFmpegTest::_enums()
        {
            _enum<ffmpeg::VideoCodec>("VideoCodec", ffmpeg::getVideoCodecEnums);
        }
        
        void FFmpegTest::_rational()
        {
            struct Data
            {
                double rate = 0.0;
                AVRational rational;
            };
            const std::array<Data, 10> data =
            {
                Data({ 0.0, { 0, 1 }}),
                Data({ 24.0, { 24, 1 }}),
                Data({ 30.0, { 30, 1 }}),
                Data({ 60.0, { 60, 1 }}),
                Data({ 23.97602397602398, { 24000, 1001 }}),
                Data({ 29.97002997002997, { 30000, 1001 }}),
                Data({ 59.94005994005994, { 60000, 1001 }}),
                Data({ 23.98, { 24000, 1001 }}),
                Data({ 29.97, { 30000, 1001 }}),
                Data({ 59.94, { 60000, 1001 }})
            };
            for (const auto& i : data)
            {
                const auto rational = ffmpeg::toRational(i.rate);
                TLR_ASSERT(rational.num == i.rational.num && rational.den == i.rational.den);
            }
        }

        void FFmpegTest::_io()
        {
            auto plugin = ffmpeg::Plugin::create();
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
                        ss << "FFmpegTest_" << size << '_' << pixelType << ".mov";
                        fileName = ss.str();
                        _print(fileName);
                    }
                    const auto imageInfo = imaging::Info(size, pixelType);
                    const otime::RationalTime duration(24.0, 24.0);
                    try
                    {
                        {
                            avio::Info info;
                            info.video.push_back(imageInfo);
                            info.videoDuration = duration;
                            auto write = plugin->write(fileName, info);
                            const auto imageWrite = imaging::Image::create(imageInfo);
                            for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                            {
                                write->writeVideoFrame(otime::RationalTime(i, 24.0), imageWrite);
                            }
                        }
                        auto read = plugin->read(fileName);
                        for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                        {
                            const auto videoFrame = read->readVideoFrame(otime::RationalTime(i, 24.0)).get();
                            //std::stringstream ss;
                            //ss << "Video frame: " << videoFrame.time;
                            //_print(ss.str());
                        }
                        for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                        {
                            const auto videoFrame = read->readVideoFrame(otime::RationalTime(i, 24.0)).get();
                            //std::stringstream ss;
                            //ss << "Video frame: " << videoFrame.time;
                            //_print(ss.str());
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
