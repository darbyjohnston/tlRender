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

        inline std::vector<otime::TimeRange> Timeline::getClipRanges() const
        {
            return _clipRanges;
        }

        inline std::shared_ptr<Observer::IValueSubject<Playback> > Timeline::observePlayback() const
        {
            return _playback;
        }

        inline std::shared_ptr<Observer::IValueSubject<Loop> > Timeline::observeLoop() const
        {
            return _loop;
        }

        inline std::shared_ptr<Observer::IValueSubject<otime::RationalTime> > Timeline::observeCurrentTime() const
        {
            return _currentTime;
        }

        inline std::shared_ptr<Observer::IValueSubject<otime::TimeRange> > Timeline::observeInOutRange() const
        {
            return _inOutRange;
        }

        inline std::shared_ptr<Observer::IValueSubject<io::VideoFrame> > Timeline::observeFrame() const
        {
            return _frame;
        }

        inline int Timeline::getFrameCacheReadAhead() const
        {
            return _frameCacheReadAhead;
        }

        inline int Timeline::getFrameCacheReadBehind() const
        {
            return _frameCacheReadBehind;
        }

        inline std::shared_ptr<Observer::IListSubject<otime::TimeRange> > Timeline::observeCachedFrames() const
        {
            return _cachedFrames;
        }
    }
}