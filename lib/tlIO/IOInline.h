// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace io
    {
        inline bool Info::operator == (const Info& other) const
        {
            return
                video == other.video &&
                time::compareExact(videoTime, other.videoTime) &&
                audio == other.audio &&
                time::compareExact(audioTime, other.audioTime) &&
                tags == other.tags;
        }

        inline bool Info::operator != (const Info& other) const
        {
            return !(*this == other);
        }

        inline VideoData::VideoData()
        {}

        inline VideoData::VideoData(
            const OTIO_NS::RationalTime& time,
            uint16_t layer,
            const std::shared_ptr<feather_tk::Image>& image) :
            time(time),
            layer(layer),
            image(image)
        {}

        inline bool VideoData::operator == (const VideoData& other) const
        {
            return
                time.strictly_equal(other.time) &&
                layer == other.layer &&
                image == other.image;
        }

        inline bool VideoData::operator != (const VideoData& other) const
        {
            return !(*this == other);
        }

        inline bool VideoData::operator < (const VideoData& other) const
        {
            return time < other.time;
        }

        inline AudioData::AudioData()
        {}

        inline AudioData::AudioData(
            const OTIO_NS::RationalTime& time,
            const std::shared_ptr<audio::Audio>& audio) :
            time(time),
            audio(audio)
        {}

        inline bool AudioData::operator == (const AudioData& other) const
        {
            return
                time.strictly_equal(other.time) &&
                audio == other.audio;
        }

        inline bool AudioData::operator != (const AudioData& other) const
        {
            return !(*this == other);
        }

        inline bool AudioData::operator < (const AudioData& other) const
        {
            return time < other.time;
        }
    }
}
