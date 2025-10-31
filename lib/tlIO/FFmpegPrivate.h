// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/FFmpeg.h>

#include <tlCore/HDR.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace tl
{
    namespace ffmpeg
    {
        //! Software scaler flags.
        const int swsScaleFlags = SWS_FAST_BILINEAR;

        //! Swap the numerator and denominator.
        AVRational swap(AVRational);

        //! Convert to HDR data.
        void toHDRData(AVFrameSideData**, int size, image::HDRData&);

        //! Convert from FFmpeg.
        audio::DataType toAudioType(AVSampleFormat);

        //! Convert to FFmpeg.
        AVSampleFormat fromAudioType(audio::DataType);

        //! Get the timecode from a data stream if it exists.
        std::string getTimecodeFromDataStream(AVFormatContext*);

        //! RAII class for FFmpeg packets.
        class Packet
        {
        public:
            Packet();
            ~Packet();

            AVPacket* p = nullptr;
        };

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);
    }
}
