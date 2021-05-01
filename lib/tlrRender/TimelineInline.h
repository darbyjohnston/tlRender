// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace timeline
    {
        inline const otime::RationalTime& Timeline::getDuration() const
        {
            return _duration;
        }

        inline const imaging::Info& Timeline::getImageInfo() const
        {
            return _imageInfo;
        }

        inline const otime::RationalTime& Timeline::getCurrentTime() const
        {
            return _currentTime;
        }

        inline Playback Timeline::getPlayback() const
        {
            return _playback;
        }

        inline Loop Timeline::getLoop() const
        {
            return _loop;
        }

        inline const std::shared_ptr<imaging::Image>& Timeline::getCurrentImage() const
        {
            return _currentImage;
        }
    }
}