// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDUtil.h>

#include <array>

namespace tl
{
    namespace device
    {
        BMDPixelFormat toBMD(PixelType value)
        {
            const std::array<
                BMDPixelFormat,
                static_cast<size_t>(PixelType::Count)> data =
            {
                bmdFormatUnspecified,
                bmdFormat8BitBGRA,
                bmdFormat8BitYUV,
                bmdFormat10BitRGB,
                bmdFormat10BitRGBX,
                bmdFormat10BitRGBXLE,
                bmdFormat10BitYUV,
                bmdFormat12BitRGB,
                bmdFormat12BitRGBLE
            };
            return data[static_cast<size_t>(value)];
        }

        PixelType fromBMD(BMDPixelFormat value)
        {
            PixelType out = PixelType::None;
            switch (value)
            {
            case bmdFormat8BitBGRA: out = PixelType::_8BitBGRA; break;
            case bmdFormat8BitYUV: out = PixelType::_8BitYUV; break;
            case bmdFormat10BitRGB: out = PixelType::_10BitRGB; break;
            case bmdFormat10BitRGBX: out = PixelType::_10BitRGBX; break;
            case bmdFormat10BitRGBXLE: out = PixelType::_10BitRGBXLE; break;
            case bmdFormat10BitYUV: out = PixelType::_10BitYUV; break;
            case bmdFormat12BitRGB: out = PixelType::_12BitRGB; break;
            case bmdFormat12BitRGBLE: out = PixelType::_12BitRGBLE; break;
            }
            return out;
        }

        std::string getVideoConnectionLabel(BMDVideoConnection value)
        {
            const std::array<std::string, 7> data =
            {
                "Unspecified",
                "SDI",
                "HDMI",
                "OpticalSDI",
                "Component",
                "Composite",
                "SVideo"
            };
            return data[static_cast<size_t>(value)];
        }

        std::string getAudioConnectionLabel(BMDAudioConnection value)
        {
            const std::array<std::string, 7> data =
            {
                "Embedded",
                "AESEBU",
                "Analog",
                "AnalogXLR",
                "AnalogRCA",
                "Microphone",
                "Headphones"
            };
            return data[static_cast<size_t>(value)];
        }

        std::string getDisplayModeLabel(BMDDisplayMode value)
        {
            const std::array<std::string, 111> data =
            {
                "NTSC",
                "NTSC2398",
                "PAL",
                "NTSCp",
                "PALp",
                "HD1080p2398",
                "HD1080p24",
                "HD1080p25",
                "HD1080p2997",
                "HD1080p30",
                "HD1080p4795",
                "HD1080p48",
                "HD1080p50",
                "HD1080p5994",
                "HD1080p6000",
                "HD1080p9590",
                "HD1080p96",
                "HD1080p100",
                "HD1080p11988",
                "HD1080p120",
                "HD1080i50",
                "HD1080i5994",
                "HD1080i6000",
                "HD720p50",
                "HD720p5994",
                "HD720p60",
                "2k2398",
                "2k24",
                "2k25",
                "2kDCI2398",
                "2kDCI24",
                "2kDCI25",
                "2kDCI2997",
                "2kDCI30",
                "2kDCI4795",
                "2kDCI48",
                "2kDCI50",
                "2kDCI5994",
                "2kDCI60",
                "2kDCI9590",
                "2kDCI96",
                "2kDCI100",
                "2kDCI11988",
                "2kDCI120",
                "4K2160p2398",
                "4K2160p24",
                "4K2160p25",
                "4K2160p2997",
                "4K2160p30",
                "4K2160p4795",
                "4K2160p48",
                "4K2160p50",
                "4K2160p5994",
                "4K2160p60",
                "4K2160p9590",
                "4K2160p96",
                "4K2160p100",
                "4K2160p11988",
                "4K2160p120",
                "4kDCI2398",
                "4kDCI24",
                "4kDCI25",
                "4kDCI2997",
                "4kDCI30",
                "4kDCI4795",
                "4kDCI48",
                "4kDCI50",
                "4kDCI5994",
                "4kDCI60",
                "4kDCI9590",
                "4kDCI96",
                "4kDCI100",
                "4kDCI11988",
                "4kDCI120",
                "8K4320p2398",
                "8K4320p24",
                "8K4320p25",
                "8K4320p2997",
                "8K4320p30",
                "8K4320p4795",
                "8K4320p48",
                "8K4320p50",
                "8K4320p5994",
                "8K4320p60",
                "8kDCI2398",
                "8kDCI24",
                "8kDCI25",
                "8kDCI2997",
                "8kDCI30",
                "8kDCI4795",
                "8kDCI48",
                "8kDCI50",
                "8kDCI5994",
                "8kDCI60",
                "640x480p60",
                "800x600p60",
                "1440x900p50",
                "1440x900p60",
                "1440x1080p50",
                "1440x1080p60",
                "1600x1200p50",
                "1600x1200p60",
                "1920x1200p50",
                "1920x1200p60",
                "1920x1440p50",
                "1920x1440p60",
                "2560x1440p50",
                "2560x1440p60",
                "2560x1600p50",
                "2560x1600p60",
                "Unknown"
            };
            return data[static_cast<size_t>(value)];
        }

        std::string getPixelFormatLabel(BMDPixelFormat value)
        {
            const std::array<std::string, 111> data =
            {
                "bmdFormatUnspecified",
                "bmdFormat8BitYUV",
                "bmdFormat10BitYUV",
                "bmdFormat8BitARGB",
                "bmdFormat8BitBGRA",
                "bmdFormat10BitRGB",
                "bmdFormat12BitRGB",
                "bmdFormat12BitRGBLE",
                "bmdFormat10BitRGBXLE",
                "bmdFormat10BitRGBX",
                "bmdFormatH265",
                "bmdFormatDNxHR"
            };
            return data[static_cast<size_t>(value)];
        }

        std::string getOutputFrameCompletionResultLabel(BMDOutputFrameCompletionResult value)
        {
            const std::array<std::string, 4> data =
            {
                "Completed",
                "Displayed Late",
                "Dropped",
                "Flushed"
            };
            return data[value];
        }

        image::PixelType getOffscreenType(device::PixelType value)
        {
            const std::array<image::PixelType, static_cast<size_t>(device::PixelType::Count)> data =
            {
                image::PixelType::None,
                image::PixelType::RGBA_U8,
                image::PixelType::RGBA_U8,
                image::PixelType::RGB_U10,
                image::PixelType::RGB_U10,
                image::PixelType::RGB_U10,
                image::PixelType::RGB_U10,
                image::PixelType::RGB_U10,
                image::PixelType::RGB_U10
            };
            return data[static_cast<size_t>(value)];
        }

        GLenum getReadPixelsFormat(device::PixelType value)
        {
            const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
            {
                GL_NONE,
                GL_BGRA,
                GL_BGRA,
                GL_BGRA,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA
            };
            return data[static_cast<size_t>(value)];
        }

        GLenum getReadPixelsType(device::PixelType value)
        {
            const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
            {
                GL_NONE,
                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_INT_2_10_10_10_REV,
                GL_UNSIGNED_INT_10_10_10_2,
                GL_UNSIGNED_INT_10_10_10_2,
                GL_UNSIGNED_INT_10_10_10_2,
                GL_UNSIGNED_INT_10_10_10_2,
                GL_UNSIGNED_INT_10_10_10_2
            };
            return data[static_cast<size_t>(value)];
        }

        GLint getReadPixelsAlign(device::PixelType value)
        {
            const std::array<GLint, static_cast<size_t>(device::PixelType::Count)> data =
            {
                0,
                4,
                4,
                256,
                256,
                256,
                128,
                256,
                256
            };
            return data[static_cast<size_t>(value)];
        }

        GLint getReadPixelsSwap(device::PixelType value)
        {
            const std::array<GLint, static_cast<size_t>(device::PixelType::Count)> data =
            {
                GL_FALSE,
                GL_FALSE,
                GL_FALSE,
                GL_TRUE,
                GL_TRUE,
                GL_FALSE,
                GL_FALSE,
                GL_FALSE,
                GL_FALSE
            };
            return data[static_cast<size_t>(value)];
        }
    }
}
