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
            case bmdFormat8BitBGRA:    out = PixelType::_8BitBGRA;    break;
            case bmdFormat8BitYUV:     out = PixelType::_8BitYUV;     break;
            case bmdFormat10BitRGB:    out = PixelType::_10BitRGB;    break;
            case bmdFormat10BitRGBX:   out = PixelType::_10BitRGBX;   break;
            case bmdFormat10BitRGBXLE: out = PixelType::_10BitRGBXLE; break;
            case bmdFormat10BitYUV:    out = PixelType::_10BitYUV;    break;
            case bmdFormat12BitRGB:    out = PixelType::_12BitRGB;    break;
            case bmdFormat12BitRGBLE:  out = PixelType::_12BitRGBLE;  break;
            default: break;
            }
            return out;
        }

        std::string getVideoConnectionLabel(BMDVideoConnection value)
        {
            std::string out;
            switch (value)
            {
            case bmdVideoConnectionUnspecified:     out = "Unspecified";     break;
            case bmdVideoConnectionSDI:             out = "SDI";             break;
            case bmdVideoConnectionHDMI:            out = "HDMI";            break;
            case bmdVideoConnectionOpticalSDI:      out = "OpticalSDI";      break;
            case bmdVideoConnectionComponent:       out = "Component";       break;
            case bmdVideoConnectionComposite:       out = "Composite";       break;
            case bmdVideoConnectionSVideo:          out = "SVideo";          break;
            case bmdVideoConnectionEthernet:        out = "Ethernet";        break;
            case bmdVideoConnectionOpticalEthernet: out = "OpticalEthernet"; break;
            default: break;
            };
            return out;
        }

        std::string getAudioConnectionLabel(BMDAudioConnection value)
        {
            std::string out;
            switch (value)
            {
            case bmdAudioConnectionEmbedded:   out = "Embedded";   break;
            case bmdAudioConnectionAESEBU:     out = "AESEBU";     break;
            case bmdAudioConnectionAnalog:     out = "Analog";     break;
            case bmdAudioConnectionAnalogXLR:  out = "AnalogXLR";  break;
            case bmdAudioConnectionAnalogRCA:  out = "AnalogRCA";  break;
            case bmdAudioConnectionMicrophone: out = "Microphone"; break;
            case bmdAudioConnectionHeadphones: out = "Headphones"; break;
            default: break;
            }
            return out;
        }

        std::string getDisplayModeLabel(BMDDisplayMode value)
        {
            std::string out;
            switch (value)
            {
            case bmdModeNTSC:     out = "NTSC";     break;
            case bmdModeNTSC2398: out = "NTSC2398"; break;
            case bmdModePAL:      out = "PAL";      break;
            case bmdModeNTSCp:    out = "NTSCp";    break;
            case bmdModePALp:     out = "PALp";     break;

            case bmdModeHD1080p2398:  out = "HD1080p2398";  break;
            case bmdModeHD1080p24:    out = "HD1080p24";    break;
            case bmdModeHD1080p25:    out = "HD1080p25";    break;
            case bmdModeHD1080p2997:  out = "HD1080p2997";  break;
            case bmdModeHD1080p30:    out = "HD1080p30";    break;
            case bmdModeHD1080p4795:  out = "HD1080p4795";  break;
            case bmdModeHD1080p48:    out = "HD1080p48";    break;
            case bmdModeHD1080p50:    out = "HD1080p50";    break;
            case bmdModeHD1080p5994:  out = "HD1080p5994";  break;
            case bmdModeHD1080p6000:  out = "HD1080p6000";  break;
            case bmdModeHD1080p9590:  out = "HD1080p9590";  break;
            case bmdModeHD1080p96:    out = "HD1080p96";    break;
            case bmdModeHD1080p100:   out = "HD1080p100";   break;
            case bmdModeHD1080p11988: out = "HD1080p11988"; break;
            case bmdModeHD1080p120:   out = "HD1080p120";   break;
            case bmdModeHD1080i50:    out = "HD1080i50";    break;
            case bmdModeHD1080i5994:  out = "HD1080i5994";  break;
            case bmdModeHD1080i6000:  out = "HD1080i6000";  break;

            case bmdModeHD720p50:   out = "HD720p50";   break;
            case bmdModeHD720p5994: out = "HD720p5994"; break;
            case bmdModeHD720p60:   out = "HD720p60";   break;

            case bmdMode2k2398: out = "2k2398"; break;
            case bmdMode2k24:   out = "2k24";   break;
            case bmdMode2k25:   out = "2k25";   break;

            case bmdMode2kDCI2398:  out = "2kDCI2398";  break;
            case bmdMode2kDCI24:    out = "2kDCI24";    break;
            case bmdMode2kDCI25:    out = "2kDCI25";    break;
            case bmdMode2kDCI2997:  out = "2kDCI2997";  break;
            case bmdMode2kDCI30:    out = "2kDCI30";    break;
            case bmdMode2kDCI4795:  out = "2kDCI4795";  break;
            case bmdMode2kDCI48:    out = "2kDCI48";    break;
            case bmdMode2kDCI50:    out = "2kDCI50";    break;
            case bmdMode2kDCI5994:  out = "2kDCI5994";  break;
            case bmdMode2kDCI60:    out = "2kDCI60";    break;
            case bmdMode2kDCI9590:  out = "2kDCI9590";  break;
            case bmdMode2kDCI96:    out = "2kDCI96";    break;
            case bmdMode2kDCI100:   out = "2kDCI100";   break;
            case bmdMode2kDCI11988: out = "2kDCI11988"; break;
            case bmdMode2kDCI120:   out = "2kDCI120";   break;

            case bmdMode4K2160p2398:  out = "4K2160p2398";  break;
            case bmdMode4K2160p24:    out = "4K2160p24";    break;
            case bmdMode4K2160p25:    out = "4K2160p25";    break;
            case bmdMode4K2160p2997:  out = "4K2160p2997";  break;
            case bmdMode4K2160p30:    out = "4K2160p30";    break;
            case bmdMode4K2160p4795:  out = "4K2160p4795";  break;
            case bmdMode4K2160p48:    out = "4K2160p48";    break;
            case bmdMode4K2160p50:    out = "4K2160p50";    break;
            case bmdMode4K2160p5994:  out = "4K2160p5994";  break;
            case bmdMode4K2160p60:    out = "4K2160p60";    break;
            case bmdMode4K2160p9590:  out = "4K2160p9590";  break;
            case bmdMode4K2160p96:    out = "4K2160p96";    break;
            case bmdMode4K2160p100:   out = "4K2160p100";   break;
            case bmdMode4K2160p11988: out = "4K2160p11988"; break;
            case bmdMode4K2160p120:   out = "4K2160p120";   break;

            case bmdMode4kDCI2398:  out = "4kDCI2398";  break;
            case bmdMode4kDCI24:    out = "4kDCI24";    break;
            case bmdMode4kDCI25:    out = "4kDCI25";    break;
            case bmdMode4kDCI2997:  out = "4kDCI2997";  break;
            case bmdMode4kDCI30:    out = "4kDCI30";    break;
            case bmdMode4kDCI4795:  out = "4kDCI4795";  break;
            case bmdMode4kDCI48:    out = "4kDCI48";    break;
            case bmdMode4kDCI50:    out = "4kDCI50";    break;
            case bmdMode4kDCI5994:  out = "4kDCI5994";  break;
            case bmdMode4kDCI60:    out = "4kDCI60";    break;
            case bmdMode4kDCI9590:  out = "4kDCI9590";  break;
            case bmdMode4kDCI96:    out = "4kDCI96";    break;
            case bmdMode4kDCI100:   out = "4kDCI100";   break;
            case bmdMode4kDCI11988: out = "4kDCI11988"; break;
            case bmdMode4kDCI120:   out = "4kDCI120";   break;

            case bmdMode8K4320p2398: out = "8K4320p2398"; break;
            case bmdMode8K4320p24:   out = "8K4320p24";   break;
            case bmdMode8K4320p25:   out = "8K4320p25";   break;
            case bmdMode8K4320p2997: out = "8K4320p2997"; break;
            case bmdMode8K4320p30:   out = "8K4320p30";   break;
            case bmdMode8K4320p4795: out = "8K4320p4795"; break;
            case bmdMode8K4320p48:   out = "8K4320p48";   break;
            case bmdMode8K4320p50:   out = "8K4320p50";   break;
            case bmdMode8K4320p5994: out = "8K4320p5994"; break;
            case bmdMode8K4320p60:   out = "8K4320p60";   break;

            case bmdMode8kDCI2398: out = "8kDCI2398"; break;
            case bmdMode8kDCI24:   out = "8kDCI24";   break;
            case bmdMode8kDCI25:   out = "8kDCI25";   break;
            case bmdMode8kDCI2997: out = "8kDCI2997"; break;
            case bmdMode8kDCI30:   out = "8kDCI30";   break;
            case bmdMode8kDCI4795: out = "8kDCI4795"; break;
            case bmdMode8kDCI48:   out = "8kDCI48";   break;
            case bmdMode8kDCI50:   out = "8kDCI50";   break;
            case bmdMode8kDCI5994: out = "8kDCI5994"; break;
            case bmdMode8kDCI60:   out = "8kDCI60";   break;

            case bmdMode640x480p60:   out = "640x480p60";   break;
            case bmdMode800x600p60:   out = "800x600p60";   break;
            case bmdMode1440x900p50:  out = "1440x900p50";  break;
            case bmdMode1440x900p60:  out = "1440x900p60";  break;
            case bmdMode1440x1080p50: out = "1440x1080p50"; break;
            case bmdMode1440x1080p60: out = "1440x1080p60"; break;
            case bmdMode1600x1200p50: out = "1600x1200p50"; break;
            case bmdMode1600x1200p60: out = "1600x1200p60"; break;
            case bmdMode1920x1200p50: out = "1920x1200p50"; break;
            case bmdMode1920x1200p60: out = "1920x1200p60"; break;
            case bmdMode1920x1440p50: out = "1920x1440p50"; break;
            case bmdMode1920x1440p60: out = "1920x1440p60"; break;
            case bmdMode2560x1440p50: out = "2560x1440p50"; break;
            case bmdMode2560x1440p60: out = "2560x1440p60"; break;
            case bmdMode2560x1600p50: out = "2560x1600p50"; break;
            case bmdMode2560x1600p60: out = "2560x1600p60"; break;

            case bmdModeUnknown: out = "Unknown"; break;
            default: break;
            }
            return out;
        }

        std::string getPixelFormatLabel(BMDPixelFormat value)
        {
            std::string out;
            switch (value)
            {
            case bmdFormatUnspecified: out = "bmdFormatUnspecified"; break;
            case bmdFormat8BitYUV:     out = "bmdFormat8BitYUV";     break;
            case bmdFormat10BitYUV:    out = "bmdFormat10BitYUV";    break;
            case bmdFormat8BitARGB:    out = "bmdFormat8BitARGB";    break;
            case bmdFormat8BitBGRA:    out = "bmdFormat8BitBGRA";    break;
            case bmdFormat10BitRGB:    out = "bmdFormat10BitRGB";    break;
            case bmdFormat12BitRGB:    out = "bmdFormat12BitRGB";    break;
            case bmdFormat12BitRGBLE:  out = "bmdFormat12BitRGBLE";  break;
            case bmdFormat10BitRGBXLE: out = "bmdFormat10BitRGBXLE"; break;
            case bmdFormat10BitRGBX:   out = "bmdFormat10BitRGBX";   break;
            case bmdFormatH265:        out = "bmdFormatH265";        break;
            case bmdFormatDNxHR:       out = "bmdFormatDNxHR";       break;
            default: break;
            };
            return out;
        }

        std::string getOutputFrameCompletionResultLabel(BMDOutputFrameCompletionResult value)
        {
            std::string out;
            switch (value)
            {
            case bmdOutputFrameCompleted:     out = "Completed";      break;
            case bmdOutputFrameDisplayedLate: out = "Displayed Late"; break;
            case bmdOutputFrameDropped:       out = "Dropped";        break;
            case bmdOutputFrameFlushed:       out = "Flushed";        break;
            };
            return out;
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
