// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace timeline
    {
        bool PlayerOptions::operator == (const PlayerOptions& other) const
        {
            return timer == other.timer &&
                audioBufferFrameCount == other.audioBufferFrameCount &&
                muteTimeout == other.muteTimeout &&
                sleepTimeout == other.sleepTimeout;
        }

        bool PlayerOptions::operator != (const PlayerOptions& other) const
        {
            return !(*this == other);
        }

        inline const file::Path& TimelinePlayer::getPath() const
        {
            return _timeline->getPath();
        }

        inline const otime::RationalTime& TimelinePlayer::getGlobalStartTime() const
        {
            return _timeline->getGlobalStartTime();
        }

        inline const otime::RationalTime& TimelinePlayer::getDuration() const
        {
            return _timeline->getDuration();
        }

        inline const imaging::Info& TimelinePlayer::getImageInfo() const
        {
            return _timeline->getImageInfo();
        }

        inline std::shared_ptr<observer::IValue<Playback> > TimelinePlayer::observePlayback() const
        {
            return _playback;
        }

        inline std::shared_ptr<observer::IValue<Loop> > TimelinePlayer::observeLoop() const
        {
            return _loop;
        }

        inline std::shared_ptr<observer::IValue<otime::RationalTime> > TimelinePlayer::observeCurrentTime() const
        {
            return _currentTime;
        }

        inline std::shared_ptr<observer::IValue<otime::TimeRange> > TimelinePlayer::observeInOutRange() const
        {
            return _inOutRange;
        }

        inline std::shared_ptr<observer::IValue<Frame> > TimelinePlayer::observeFrame() const
        {
            return _frame;
        }

        inline std::shared_ptr<observer::IList<otime::TimeRange> > TimelinePlayer::observeCachedFrames() const
        {
            return _cachedFrames;
        }
    }
}