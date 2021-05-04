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

        inline std::shared_ptr<Observer::IValueSubject<otime::RationalTime> > Timeline::observeCurrentTime() const
        {
            return _currentTime;
        }

        inline std::shared_ptr<Observer::IValueSubject<Playback> > Timeline::observePlayback() const
        {
            return _playback;
        }

        inline std::shared_ptr<Observer::IValueSubject<Loop> > Timeline::observeLoop() const
        {
            return _loop;
        }

        inline std::shared_ptr<Observer::IValueSubject<std::shared_ptr<imaging::Image> > > Timeline::observeCurrentImage() const
        {
            return _currentImage;
        }
    }
}