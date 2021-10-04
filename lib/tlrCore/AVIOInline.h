// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace avio
    {
        inline bool Info::operator == (const Info& other) const
        {
            return
                this->video == other.video &&
                this->videoType == other.videoType &&
                this->videoTimeRange == other.videoTimeRange &&
                this->audio == other.audio &&
                this->audioSampleCount == other.audioSampleCount &&
                this->tags == other.tags;
        }

        inline bool Info::operator != (const Info& other) const
        {
            return !(*this == other);
        }

        inline bool VideoFrame::operator == (const VideoFrame& other) const
        {
            return
                this->time == other.time &&
                this->layer == other.layer &&
                this->image == other.image;
        }

        inline bool VideoFrame::operator != (const VideoFrame& other) const
        {
            return !(*this == other);
        }

        inline bool VideoFrame::operator < (const VideoFrame& other) const
        {
            return time < other.time;
        }

        inline const file::Path& IIO::getPath() const
        {
            return _path;
        }
    }
}