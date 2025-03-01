// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/App.h>
#include <dtk/ui/FileBrowser.h>
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

        //! Export render size.
        enum class ExportRenderSize
        {
            Default,
            _1920_1080,
            _3840_2160,
            _4096_2160,
            Custom,

            Count,
            First = Default
        };
        DTK_ENUM(ExportRenderSize);

        //! Get an export render size.
        const dtk::Size2I& getSize(ExportRenderSize);

        //! Export file type.
        enum class ExportFileType
        {
            Images,
            Movie,

            Count,
            First = Images
        };
        DTK_ENUM(ExportFileType);

        //! Export options.
        struct ExportOptions
        {
            std::string directory;
            ExportRenderSize renderSize = ExportRenderSize::Default;
            dtk::Size2I customRenderSize = dtk::Size2I(1920, 1080);
            ExportFileType fileType = ExportFileType::Images;
            std::string imageBaseName;
            size_t imagePad = 0;
            std::string imageExtension;
            std::string movieBaseName;
            std::string movieExtension;
            std::string movieCodec;

            bool operator == (const ExportOptions&) const;
            bool operator != (const ExportOptions&) const;
        };

        //! File browser options.
        struct FileBrowserOptions
        {
            bool nativeFileDialog = true;
            std::string path;
            dtk::FileBrowserOptions options;

            bool operator == (const FileBrowserOptions&) const;
            bool operator != (const FileBrowserOptions&) const;
        };

        //! File sequence options.
        struct FileSequenceOptions
        {
            timeline::FileSequenceAudio audio = timeline::FileSequenceAudio::BaseName;
            std::string audioFileName;
            std::string audioDirectory;
            size_t maxDigits = 9;
            io::SequenceOptions io;

            bool operator == (const FileSequenceOptions&) const;
            bool operator != (const FileSequenceOptions&) const;
        };

        //! Miscellaneous options.
        struct MiscOptions
        {
            bool tooltipsEnabled = true;

            bool operator == (const MiscOptions&) const;
            bool operator != (const MiscOptions&) const;
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

        //! Style options.
        struct StyleOptions
        {
            dtk::ColorStyle colorStyle = dtk::ColorStyle::Dark;
            float displayScale = 0.F;

            bool operator == (const StyleOptions&) const;
            bool operator != (const StyleOptions&) const;
        };

        //! Timeline options.
        struct TimelineOptions
        {
            bool editable = false;
            bool frameView = true;
            bool scroll = true;
            bool stopOnScrub = false;
            timelineui::ItemOptions item;
            timelineui::DisplayOptions display;
            bool firstTrack = false;

            bool operator == (const TimelineOptions&) const;
            bool operator != (const TimelineOptions&) const;
        };

        //! Window options.
        struct WindowOptions
        {
            dtk::Size2I size = dtk::Size2I(1920, 1080);
            bool fileToolBar = true;
            bool compareToolBar = true;
            bool windowToolBar = true;
            bool viewToolBar = true;
            bool toolsToolBar = true;
            bool timeline = true;
            bool bottomToolBar = true;
            bool statusToolBar = true;
            float splitter = .7F;
            float splitter2 = .7F;

            bool operator == (const WindowOptions&) const;
            bool operator != (const WindowOptions&) const;
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

            //! \name Export
            ///@{

            const ExportOptions& getExport() const;
            std::shared_ptr<dtk::IObservableValue<ExportOptions> > observeExport() const;
            void setExport(const ExportOptions&);

            ///@}

            //! \name File Browser
            ///@{

            const FileBrowserOptions& getFileBrowser() const;
            std::shared_ptr<dtk::IObservableValue<FileBrowserOptions> > observeFileBrowser() const;
            void setFileBrowser(const FileBrowserOptions&);

            ///@}

            //! \name File Sequences
            ///@{

            const FileSequenceOptions& getFileSequence() const;
            std::shared_ptr<dtk::IObservableValue<FileSequenceOptions> > observeFileSequence() const;
            void setFileSequence(const FileSequenceOptions&);

            ///@}

            //! \name Miscellaneous
            ///@{

            const MiscOptions& getMisc() const;
            std::shared_ptr<dtk::IObservableValue<MiscOptions> > observeMisc() const;
            void setMisc(const MiscOptions&);

            ///@}

            //! \name Performance
            ///@{

            const PerformanceOptions& getPerformance() const;
            std::shared_ptr<dtk::IObservableValue<PerformanceOptions> > observePerformance() const;
            void setPerformance(const PerformanceOptions&);

            ///@}

            //! \name Style
            ///@{

            const StyleOptions& getStyle() const;
            std::shared_ptr<dtk::IObservableValue<StyleOptions> > observeStyle() const;
            void setStyle(const StyleOptions&);

            ///@}

            //! \name Timeline
            ///@{

            const TimelineOptions& getTimeline() const;
            std::shared_ptr<dtk::IObservableValue<TimelineOptions> > observeTimeline() const;
            void setTimeline(const TimelineOptions&);

            ///@}

            //! \name Window
            ///@{

            const WindowOptions& getWindow() const;
            std::shared_ptr<dtk::IObservableValue<WindowOptions> > observeWindow() const;
            void setWindow(const WindowOptions&);

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

        private:
            DTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const CacheOptions&);
        void to_json(nlohmann::json&, const ExportOptions&);
        void to_json(nlohmann::json&, const FileBrowserOptions&);
        void to_json(nlohmann::json&, const FileSequenceOptions&);
        void to_json(nlohmann::json&, const MiscOptions&);
        void to_json(nlohmann::json&, const PerformanceOptions&);
        void to_json(nlohmann::json&, const StyleOptions&);
        void to_json(nlohmann::json&, const TimelineOptions&);
        void to_json(nlohmann::json&, const WindowOptions&);

        void from_json(const nlohmann::json&, CacheOptions&);
        void from_json(const nlohmann::json&, ExportOptions&);
        void from_json(const nlohmann::json&, FileBrowserOptions&);
        void from_json(const nlohmann::json&, FileSequenceOptions&);
        void from_json(const nlohmann::json&, MiscOptions&);
        void from_json(const nlohmann::json&, PerformanceOptions&);
        void from_json(const nlohmann::json&, StyleOptions&);
        void from_json(const nlohmann::json&, TimelineOptions&);
        void from_json(const nlohmann::json&, WindowOptions&);

        ///@}
    }
}
