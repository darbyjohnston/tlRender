// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

#include <dtk/core/ObservableValue.h>

#include <tlIO/SequenceIO.h>
#if defined(TLRENDER_FFMPEG)
#include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <nlohmann/json.hpp>

namespace dtk
{
    class Context;
    class Settings;
}

namespace tl
{
    namespace play
    {
        //! Cache options.
        struct CacheOptions
        {
            size_t sizeGB = 4;
            double readAhead = 4.F;
            double readBehind = .5F;

            bool operator == (const CacheOptions&) const;
            bool operator != (const CacheOptions&) const;
        };

        //! Window options.
        struct WindowOptions
        {
            bool  fileToolBar = true;
            bool  compareToolBar = true;
            bool  windowToolBar = true;
            bool  viewToolBar = true;
            bool  toolsToolBar = true;
            bool  timeline = true;
            bool  bottomToolBar = true;
            bool  statusToolBar = true;
            float splitter = .7F;
            float splitter2 = .7F;

            bool operator == (const WindowOptions&) const;
            bool operator != (const WindowOptions&) const;
        };

        //! File sequence options.
        struct FileSequenceOptions
        {
            timeline::FileSequenceAudio audio = timeline::FileSequenceAudio::BaseName;
            std::string audioFileName;
            std::string audioDirectory;
            size_t maxDigits = 9;

            bool operator == (const FileSequenceOptions&) const;
            bool operator != (const FileSequenceOptions&) const;
        };

        //! Performance options.
        struct PerformanceOptions
        {
            size_t audioBufferFrameCount = timeline::PlayerOptions().audioBufferFrameCount;
            size_t videoRequestCount = 16;
            size_t audioRequestCount = 16;

            bool operator == (const PerformanceOptions&) const;
            bool operator != (const PerformanceOptions&) const;
        };

        //! Timeline options.
        struct TimelineOptions
        {
            bool editable = false;
            bool frameView = true;
            bool scroll = true;
            bool stopOnScrub = false;

            bool operator == (const TimelineOptions&) const;
            bool operator != (const TimelineOptions&) const;
        };

        //! Settings model.
        class SettingsModel : public std::enable_shared_from_this<SettingsModel>
        {
            DTK_NON_COPYABLE(SettingsModel);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            SettingsModel();

        public:
            ~SettingsModel();

            //! Create a new model.
            static std::shared_ptr<SettingsModel> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            //! Reset to default values.
            void reset();

            //! \name Cache
            ///@{

            const CacheOptions& getCache() const;

            std::shared_ptr<dtk::IObservableValue<CacheOptions> > observeCache() const;

            void setCache(const CacheOptions&);

            ///@}

            //! \name Window
            ///@{

            const WindowOptions& getWindow() const;
            const dtk::Size2I& getWindowSize() const;

            std::shared_ptr<dtk::IObservableValue<WindowOptions> > observeWindow() const;
            std::shared_ptr<dtk::IObservableValue<dtk::Size2I> > observeWindowSize() const;

            void setWindowSize(const dtk::Size2I&);
            void setWindow(const WindowOptions&);

            ///@}

            //! \name Sequences
            ///@{

            const FileSequenceOptions& getFileSequence() const;
            const io::SequenceOptions& getSequenceIO() const;

            std::shared_ptr<dtk::IObservableValue<FileSequenceOptions> > observeFileSequence() const;
            std::shared_ptr<dtk::IObservableValue<io::SequenceOptions> > observeSequenceIO() const;

            void setFileSequence(const FileSequenceOptions&);
            void setSequenceIO(const io::SequenceOptions&);

            ///@}

#if defined(TLRENDER_FFMPEG)
            //! \name FFmpeg
            ///@{

            const ffmpeg::Options& getFFmpeg() const;

            std::shared_ptr<dtk::IObservableValue<ffmpeg::Options> > observeFFmpeg() const;

            void setFFmpeg(const ffmpeg::Options&);

            ///@}
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            //! \name USD
            ///@{

            const usd::Options& getUSD() const;

            std::shared_ptr<dtk::IObservableValue<usd::Options> > observeUSD() const;

            void setUSD(const usd::Options&);

            ///@}
#endif // TLRENDER_USD

            //! \name File Browser
            ///@{

            bool getNativeFileDialog() const;

            std::shared_ptr<dtk::IObservableValue<bool> > observeNativeFileDialog() const;

            void setNativeFileDialog(bool);

            ///@}

            //! \name Performance
            ///@{

            const PerformanceOptions& getPerformance() const;

            std::shared_ptr<dtk::IObservableValue<PerformanceOptions> > observePerformance() const;

            void setPerformance(const PerformanceOptions&);

            ///@}

            //! \name Timeline
            ///@{

            const TimelineOptions& getTimeline() const;
            const timelineui::ItemOptions& getTimelineItem() const;
            const timelineui::DisplayOptions& getTimelineDisplay() const;
            bool getTimelineFirstTrack() const;

            std::shared_ptr<dtk::IObservableValue<TimelineOptions> > observeTimeline() const;
            std::shared_ptr<dtk::IObservableValue<timelineui::ItemOptions> > observeTimelineItem() const;
            std::shared_ptr<dtk::IObservableValue<timelineui::DisplayOptions> > observeTimelineDisplay() const;
            std::shared_ptr<dtk::IObservableValue<bool> > observeTimelineFirstTrack() const;

            void setTimeline(const TimelineOptions&);
            void setTimelineItem(const timelineui::ItemOptions&);
            void setTimelineDisplay(const timelineui::DisplayOptions&);
            void setTimelineFirstTrack(bool);

            ///@}

            //! \name Miscellaneous
            ///@{

            bool getTooltipsEnabled() const;

            std::shared_ptr<dtk::IObservableValue<bool> > observeTooltipsEnabled() const;

            void setTooltipsEnabled(bool);

            ///@}

        private:
            DTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const CacheOptions&);
        void to_json(nlohmann::json&, const WindowOptions&);
        void to_json(nlohmann::json&, const FileSequenceOptions&);
        void to_json(nlohmann::json&, const PerformanceOptions&);
        void to_json(nlohmann::json&, const TimelineOptions&);

        void from_json(const nlohmann::json&, CacheOptions&);
        void from_json(const nlohmann::json&, WindowOptions&);
        void from_json(const nlohmann::json&, FileSequenceOptions&);
        void from_json(const nlohmann::json&, PerformanceOptions&);
        void from_json(const nlohmann::json&, TimelineOptions&);

        ///@}
    }
}
