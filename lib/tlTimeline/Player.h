// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/PlayerOptions.h>
#include <tlTimeline/Timeline.h>

#include <ftk/Core/ObservableList.h>

namespace tl
{
    namespace timeline
    {
        //! Timeline player cache information.
        struct PlayerCacheInfo
        {
            //! Percentage used of the video cache.
            float videoPercentage = 0.F;

            //! Percentage used of the audio cache.
            float audioPercentage = 0.F;

            //! Cached video.
            std::vector<OTIO_NS::TimeRange> video;

            //! Cached audio.
            std::vector<OTIO_NS::TimeRange> audio;

            bool operator == (const PlayerCacheInfo&) const;
            bool operator != (const PlayerCacheInfo&) const;
        };

        //! Playback modes.
        enum class Playback
        {
            Stop,
            Forward,
            Reverse,

            Count,
            First = Stop
        };
        FTK_ENUM(Playback);

        //! Playback loop modes.
        enum class Loop
        {
            Loop,
            Once,
            PingPong,

            Count,
            First = Loop
        };
        FTK_ENUM(Loop);

        //! Time actions.
        enum class TimeAction
        {
            Start,
            End,
            FramePrev,
            FramePrevX10,
            FramePrevX100,
            FrameNext,
            FrameNextX10,
            FrameNextX100,
            JumpBack1s,
            JumpBack10s,
            JumpForward1s,
            JumpForward10s,

            Count,
            First = Start
        };
        FTK_ENUM(TimeAction);

        //! Timeline player.
        class Player : public std::enable_shared_from_this<Player>
        {
            FTK_NON_COPYABLE(Player);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<Timeline>&,
                const PlayerOptions&);

            Player();

        public:
            ~Player();

            //! Create a new timeline player.
            static std::shared_ptr<Player> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<Timeline>&,
                const PlayerOptions& = PlayerOptions());

            //! Get the context.
            std::shared_ptr<ftk::Context> getContext() const;

            //! Get the timeline.
            const std::shared_ptr<Timeline>& getTimeline() const;

            //! Get the path.
            const file::Path& getPath() const;

            //! Get the audio path.
            const file::Path& getAudioPath() const;

            //! Get the timeline player options.
            const PlayerOptions& getPlayerOptions() const;

            //! Get the timeline options.
            const Options& getOptions() const;

            //! \name Information
            ///@{

            //! Get the time range.
            const OTIO_NS::TimeRange& getTimeRange() const;

            //! Get the I/O information. The information is retrieved from
            //! the first clip in the timeline.
            const io::Info& getIOInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            double getDefaultSpeed() const;

            //! Get the playback speed.
            double getSpeed() const;

            //! Observe the playback speed.
            std::shared_ptr<ftk::IObservableValue<double> > observeSpeed() const;

            //! Set the playback speed.
            void setSpeed(double);

            //! Get the playback mode.
            Playback getPlayback() const;

            //! Observe the playback mode.
            std::shared_ptr<ftk::IObservableValue<Playback> > observePlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Get whether playback is stopped.
            bool isStopped() const;

            //! Stop playback.
            void stop();

            //! Start forward playback.
            void forward();

            //! Start reverse playback.
            void reverse();

            //! Get the playback loop.
            Loop getLoop() const;

            //! Observe the playback loop mode.
            std::shared_ptr<ftk::IObservableValue<Loop> > observeLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            ///@}

            //! \name Time
            ///@{

            //! Get the current time.
            const OTIO_NS::RationalTime& getCurrentTime() const;

            //! Observe the current time.
            std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > observeCurrentTime() const;

            //! Observe seeking.
            std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > observeSeek() const;

            //! Seek to the given time.
            void seek(const OTIO_NS::RationalTime&);

            //! Time action.
            void timeAction(TimeAction);

            //! Go to the start time.
            void gotoStart();

            //! Go to the end time.
            void gotoEnd();

            //! Go to the previous frame.
            void framePrev();

            //! Go to the next frame.
            void frameNext();

            ///@}

            //! \name In/Out Points
            ///@{

            //! Get the in/out points range.
            const OTIO_NS::TimeRange& getInOutRange() const;

            //! Observe the in/out points range.
            std::shared_ptr<ftk::IObservableValue<OTIO_NS::TimeRange> > observeInOutRange() const;

            //! Set the in/out points range.
            void setInOutRange(const OTIO_NS::TimeRange&);

            //! Set the in point to the current time.
            void setInPoint();

            //! Reset the in point
            void resetInPoint();

            //! Set the out point to the current time.
            void setOutPoint();

            //! Reset the out point
            void resetOutPoint();

            ///@}

            //! \name Comparison
            ///@{

            //! Get the timelines for comparison.
            const std::vector<std::shared_ptr<Timeline> >& getCompare() const;

            //! Observe the timelines for comparison.
            std::shared_ptr<ftk::IObservableList<std::shared_ptr<Timeline> > > observeCompare() const;

            //! Set the timelines for comparison.
            void setCompare(const std::vector<std::shared_ptr<Timeline> >&);

            //! Get the comparison time mode.
            CompareTime getCompareTime() const;

            //! Observe the comparison time mode.
            std::shared_ptr<ftk::IObservableValue<CompareTime> > observeCompareTime() const;

            //! Set the comparison time mode.
            void setCompareTime(CompareTime);

            ///@}

            //! \name I/O
            ///@{

            //! Get the I/O options.
            const io::Options& getIOOptions() const;

            //! Observe the I/O options.
            std::shared_ptr<ftk::IObservableValue<io::Options> > observeIOOptions() const;

            //! Set the I/O options.
            void setIOOptions(const io::Options&);

            ///@}

            //! \name Video
            ///@{

            //! Get the video layer.
            int getVideoLayer() const;

            //! Observer the video layer.
            std::shared_ptr<ftk::IObservableValue<int> > observeVideoLayer() const;

            //! Set the video layer.
            void setVideoLayer(int);

            //! Get the comparison video layers.
            const std::vector<int>& getCompareVideoLayers() const;

            //! Observe the comparison video layers.
            std::shared_ptr<ftk::IObservableList<int> > observeCompareVideoLayers() const;

            //! Set the comparison video layers.
            void setCompareVideoLayers(const std::vector<int>&);

            //! Get the current video data.
            const std::vector<VideoData>& getCurrentVideo() const;

            //! Observe the current video data.
            std::shared_ptr<ftk::IObservableList<VideoData> > observeCurrentVideo() const;

            ///@}

            //! \name Audio
            ///@{

            //! Get the audio device.
            const audio::DeviceID& getAudioDevice() const;

            //! Observe the audio devices.
            std::shared_ptr<ftk::IObservableValue<audio::DeviceID> > observeAudioDevice() const;

            //! Set the audio device.
            void setAudioDevice(const audio::DeviceID&);

            //! Get the volume.
            float getVolume() const;

            //! Observe the audio volume.
            std::shared_ptr<ftk::IObservableValue<float> > observeVolume() const;

            //! Set the audio volume.
            void setVolume(float);

            //! Get the audio mute.
            bool isMuted() const;

            //! Observe the audio mute.
            std::shared_ptr<ftk::IObservableValue<bool> > observeMute() const;

            //! Set the audio mute.
            void setMute(bool);

            //! Get the audio channels mute.
            const std::vector<bool>& getChannelMute() const;

            //! Observe the audio channels mute.
            std::shared_ptr<ftk::IObservableList<bool> > observeChannelMute() const;

            //! Set the audio channels mute.
            void setChannelMute(const std::vector<bool>&);

            //! Get the audio sync offset (in seconds).
            double getAudioOffset() const;

            //! Observe the audio sync offset (in seconds).
            std::shared_ptr<ftk::IObservableValue<double> > observeAudioOffset() const;

            //! Set the audio sync offset (in seconds).
            void setAudioOffset(double);

            //! Get the current audio data.
            const std::vector<AudioData>& getCurrentAudio() const;

            //! Observe the current audio data.
            std::shared_ptr<ftk::IObservableList<AudioData> > observeCurrentAudio() const;

            ///@}

            //! \name Cache
            ///@{

            //! Get the cache options.
            const PlayerCacheOptions& getCacheOptions() const;

            //! Observe the cache options.
            std::shared_ptr<ftk::IObservableValue<PlayerCacheOptions> > observeCacheOptions() const;

            //! Set the cache options.
            void setCacheOptions(const PlayerCacheOptions&);

            //! Observe the cache information.
            std::shared_ptr<ftk::IObservableValue<PlayerCacheInfo> > observeCacheInfo() const;

            //! Clear the cache.
            void clearCache();

            ///@}

            //! Tick the timeline player.
            void tick();

        private:
            void _thread();

            FTK_PRIVATE();
        };
    }
}
