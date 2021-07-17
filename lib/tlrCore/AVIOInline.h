// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace avio
    {
        inline Info::Info() :
            videoDuration(time::invalidTime)
        {}

        inline bool VideoFrame::operator == (const VideoFrame& other) const
        {
            return this->image == other.image && this->time == other.time;
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