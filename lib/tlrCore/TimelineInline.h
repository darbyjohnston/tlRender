// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace timeline
    {
        inline const std::string& Timeline::getFileName() const
        {
            return _fileName;
        }

        inline const otime::RationalTime& Timeline::getGlobalStartTime() const
        {
            return _globalStartTime;
        }

        inline const otime::RationalTime& Timeline::getDuration() const
        {
            return _duration;
        }

        inline const imaging::Info& Timeline::getImageInfo() const
        {
            return _imageInfo;
        }

        inline const std::vector<otime::TimeRange>& Timeline::getClipRanges() const
        {
            return _clipRanges;
        }
    }
}