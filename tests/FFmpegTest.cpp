// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "FFmpegTest.h"

#include <tlrCore/Assert.h>
#include <tlrCore/FFmpeg.h>

#include <array>

namespace tlr
{
    std::shared_ptr<FFmpegTest> FFmpegTest::create()
    {
        return std::shared_ptr<FFmpegTest>(new FFmpegTest);
    }

    void FFmpegTest::run()
    {
        _toRational();
    }

    void FFmpegTest::_toRational()
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
}
