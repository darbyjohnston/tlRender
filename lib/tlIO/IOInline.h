// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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

        inline bool VideoData::operator == (const VideoData& other) const
        {
            return
                time::compareExact(time, other.time) &&
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

        inline bool AudioData::operator == (const AudioData& other) const
        {
            return
                time::compareExact(time, other.time) &&
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

        inline const file::Path& IIO::getPath() const
        {
            return _path;
        }
    }
}
