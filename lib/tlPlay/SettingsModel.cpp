// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/SettingsModel.h>

#include <dtk/ui/FileBrowser.h>
#include <dtk/ui/Settings.h>

namespace tl
{
    namespace play
    {
        bool CacheOptions::operator == (const CacheOptions& other) const
        {
            return
                sizeGB == other.sizeGB &&
                readAhead == other.readAhead &&
                readBehind == other.readBehind;
        }

        bool CacheOptions::operator != (const CacheOptions& other) const
        {
            return !(*this == other);
        }
        bool WindowOptions::operator == (const WindowOptions& other) const
        {
            return
                fileToolBar == other.fileToolBar &&
                compareToolBar == other.compareToolBar &&
                windowToolBar == other.windowToolBar &&
                viewToolBar == other.viewToolBar &&
                toolsToolBar == other.toolsToolBar &&
                timeline == other.timeline &&
                bottomToolBar == other.bottomToolBar &&
                statusToolBar == other.statusToolBar &&
                splitter == other.splitter &&
                splitter2 == other.splitter2;
        }

        bool WindowOptions::operator != (const WindowOptions& other) const
        {
            return !(*this == other);
        }

        bool FileSequenceOptions::operator == (const FileSequenceOptions& other) const
        {
            return
                audio == other.audio &&
                audioFileName == other.audioFileName &&
                audioDirectory == other.audioDirectory &&
                maxDigits == other.maxDigits;
        }

        bool FileSequenceOptions::operator != (const FileSequenceOptions& other) const
        {
            return !(*this == other);
        }

        bool PerformanceOptions::operator == (const PerformanceOptions& other) const
        {
            return
                audioBufferFrameCount == other.audioBufferFrameCount &&
                videoRequestCount == other.videoRequestCount &&
                audioRequestCount == other.audioRequestCount;
        }

        bool PerformanceOptions::operator != (const PerformanceOptions& other) const
        {
            return !(*this == other);
        }

        bool TimelineOptions::operator == (const TimelineOptions& other) const
        {
            return
                editable == other.editable &&
                frameView == other.frameView &&
                scroll == other.scroll &&
                stopOnScrub == other.stopOnScrub;
        }

        bool TimelineOptions::operator != (const TimelineOptions& other) const
        {
            return !(*this == other);
        }

        struct SettingsModel::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::Settings> settings;

            std::shared_ptr<dtk::ObservableValue<CacheOptions> > cache;

            std::shared_ptr<dtk::ObservableValue<WindowOptions> > window;
            std::shared_ptr<dtk::ObservableValue<dtk::Size2I> > windowSize;

            std::shared_ptr<dtk::ObservableValue<FileSequenceOptions> > fileSequence;
            std::shared_ptr<dtk::ObservableValue<io::SequenceOptions> > sequenceIO;
#if defined(TLRENDER_FFMPEG)
            std::shared_ptr<dtk::ObservableValue<ffmpeg::Options> > ffmpeg;
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            std::shared_ptr<dtk::ObservableValue<usd::Options> > usd;
#endif // TLRENDER_USD

            std::shared_ptr<dtk::ObservableValue<bool> > nativeFileDialog;

            std::shared_ptr<dtk::ObservableValue<PerformanceOptions> > performance;

            std::shared_ptr<dtk::ObservableValue<TimelineOptions> > timeline;
            std::shared_ptr<dtk::ObservableValue<timelineui::ItemOptions> > timelineItem;
            std::shared_ptr<dtk::ObservableValue<timelineui::DisplayOptions> > timelineDisplay;
            std::shared_ptr<dtk::ObservableValue<bool> > timelineFirstTrack;

            std::shared_ptr<dtk::ObservableValue<bool> > tooltipsEnabled;
        };

        void SettingsModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            DTK_P();

            p.context = context;
            p.settings = settings;

            {
                std::string path;
                dtk::FileBrowserOptions options;
                p.settings->get("FileBrowser/Path", path);
                p.settings->getT("FileBrowser/Options", options);
                auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
                fileBrowserSystem->setPath(path);
                fileBrowserSystem->setOptions(options);
            }

            CacheOptions cache;
            settings->getT("Cache", cache);
            p.cache = dtk::ObservableValue<CacheOptions>::create(cache);

            WindowOptions window;
            settings->getT("Window", window);
            p.window = dtk::ObservableValue<WindowOptions>::create(window);
            dtk::Size2I windowSize(1920, 1080);
            settings->getT("Window/Size", windowSize);
            p.windowSize = dtk::ObservableValue<dtk::Size2I>::create(windowSize);

            FileSequenceOptions fileSequence;
            settings->getT("FileSequence", fileSequence);
            p.fileSequence = dtk::ObservableValue<FileSequenceOptions>::create(fileSequence);
            io::SequenceOptions sequenceIO;
            settings->getT("SequenceIO", sequenceIO);
            p.sequenceIO = dtk::ObservableValue<io::SequenceOptions>::create(sequenceIO);
#if defined(TLRENDER_FFMPEG)
            ffmpeg::Options ffmpeg;
            settings->getT("FFmpeg", ffmpeg);
            p.ffmpeg = dtk::ObservableValue<ffmpeg::Options>::create(ffmpeg);
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            usd::Options usd;
            settings->getT("USD", usd);
            p.usd = dtk::ObservableValue<usd::Options>::create(usd);
#endif // TLRENDER_USD

            bool nativeFileDialog = true;
            settings->get("FileBrowser/NativeFileDialog", nativeFileDialog);
            p.nativeFileDialog = dtk::ObservableValue<bool>::create(nativeFileDialog);

            PerformanceOptions performance;
            settings->getT("Performance", performance);
            p.performance = dtk::ObservableValue<PerformanceOptions>::create(performance);

            TimelineOptions timeline;
            settings->getT("Timeline", timeline);
            p.timeline = dtk::ObservableValue<TimelineOptions>::create(timeline);
            timelineui::ItemOptions timelineItem;
            settings->getT("TimelineItem", timelineItem);
            p.timelineItem = dtk::ObservableValue<timelineui::ItemOptions>::create(timelineItem);
            timelineui::DisplayOptions timelineDisplay;
            settings->getT("TimelineDisplay", timelineDisplay);
            p.timelineDisplay = dtk::ObservableValue<timelineui::DisplayOptions>::create(timelineDisplay);
            p.timelineFirstTrack = dtk::ObservableValue<bool>::create(false);

            bool tooltipsEnabled = true;
            settings->get("Misc/Tooltips", tooltipsEnabled);
            p.tooltipsEnabled = dtk::ObservableValue<bool>::create(tooltipsEnabled);
        }

        SettingsModel::SettingsModel() :
            _p(new Private)
        {}

        SettingsModel::~SettingsModel()
        {
            DTK_P();
            if (auto context = p.context.lock())
            {
                auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
                p.settings->set("FileBrowser/Path", fileBrowserSystem->getPath().u8string());
                p.settings->setT("FileBrowser/Options", fileBrowserSystem->getOptions());
            }

            p.settings->setT("Cache", p.cache->get());

            p.settings->setT("Window/Size", p.windowSize->get());

            p.settings->setT("FileSequence", p.fileSequence->get());
            p.settings->setT("SequenceIO", p.sequenceIO->get());
#if defined(TLRENDER_FFMPEG)
            p.settings->setT("FFmpeg", p.ffmpeg->get());
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            p.settings->setT("USD", p.usd->get());
#endif // TLRENDER_USD

            p.settings->set("FileBrowser/NativeFileDialog", p.nativeFileDialog->get());

            p.settings->setT("Performance", p.performance->get());

            p.settings->set("Misc/Tooltips", p.tooltipsEnabled->get());
        }

        std::shared_ptr<SettingsModel> SettingsModel::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            auto out = std::shared_ptr<SettingsModel>(new SettingsModel);
            out->_init(context, settings);
            return out;
        }

        void SettingsModel::reset()
        {
            setCache(CacheOptions());
            setWindowSize(dtk::Size2I(1920, 1080));
            setWindow(WindowOptions());
            setFileSequence(FileSequenceOptions());
            setSequenceIO(io::SequenceOptions());
#if defined(TLRENDER_FFMPEG)
            setFFmpeg(ffmpeg::Options());
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            setUSD(usd::Options());
#endif // TLRENDER_USD
            setNativeFileDialog(true);
            setPerformance(PerformanceOptions());
            setTimeline(TimelineOptions());
            setTimelineItem(timelineui::ItemOptions());
            setTimelineDisplay(timelineui::DisplayOptions());
            setTimelineFirstTrack(false);
            setTooltipsEnabled(true);
        }

        const CacheOptions& SettingsModel::getCache() const
        {
            return _p->cache->get();
        }

        std::shared_ptr<dtk::IObservableValue<CacheOptions> > SettingsModel::observeCache() const
        {
            return _p->cache;
        }

        void SettingsModel::setCache(const CacheOptions& value)
        {
            _p->cache->setIfChanged(value);
        }

        const WindowOptions& SettingsModel::getWindow() const
        {
            return _p->window->get();
        }

        const dtk::Size2I& SettingsModel::getWindowSize() const
        {
            return _p->windowSize->get();
        }

        std::shared_ptr<dtk::IObservableValue<WindowOptions> > SettingsModel::observeWindow() const
        {
            return _p->window;
        }

        std::shared_ptr<dtk::IObservableValue<dtk::Size2I> > SettingsModel::observeWindowSize() const
        {
            return _p->windowSize;
        }

        void SettingsModel::setWindow(const WindowOptions& value)
        {
            _p->window->setIfChanged(value);
        }

        void SettingsModel::setWindowSize(const dtk::Size2I& value)
        {
            _p->windowSize->setIfChanged(value);
        }

        const FileSequenceOptions& SettingsModel::getFileSequence() const
        {
            return _p->fileSequence->get();
        }

        const io::SequenceOptions& SettingsModel::getSequenceIO() const
        {
            return _p->sequenceIO->get();
        }

        std::shared_ptr<dtk::IObservableValue<FileSequenceOptions> > SettingsModel::observeFileSequence() const
        {
            return _p->fileSequence;
        }

        std::shared_ptr<dtk::IObservableValue<io::SequenceOptions> > SettingsModel::observeSequenceIO() const
        {
            return _p->sequenceIO;
        }

        void SettingsModel::setFileSequence(const FileSequenceOptions& value)
        {
            _p->fileSequence->setIfChanged(value);
        }

        void SettingsModel::setSequenceIO(const io::SequenceOptions& value)
        {
            _p->sequenceIO->setIfChanged(value);
        }

#if defined(TLRENDER_FFMPEG)
        const ffmpeg::Options& SettingsModel::getFFmpeg() const
        {
            return _p->ffmpeg->get();
        }

        std::shared_ptr<dtk::IObservableValue<ffmpeg::Options> > SettingsModel::observeFFmpeg() const
        {
            return _p->ffmpeg;
        }

        void SettingsModel::setFFmpeg(const ffmpeg::Options& value)
        {
            _p->ffmpeg->setIfChanged(value);
        }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        const usd::Options& SettingsModel::getUSD() const
        {
            return _p->usd->get();
        }

        std::shared_ptr<dtk::IObservableValue<usd::Options> > SettingsModel::observeUSD() const
        {
            return _p->usd;
        }

        void SettingsModel::setUSD(const usd::Options& value)
        {
            _p->usd->setIfChanged(value);
        }
#endif // TLRENDER_USD

        bool SettingsModel::getNativeFileDialog() const
        {
            return _p->nativeFileDialog->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > SettingsModel::observeNativeFileDialog() const
        {
            return _p->nativeFileDialog;
        }

        void SettingsModel::setNativeFileDialog(bool value)
        {
            _p->nativeFileDialog->setIfChanged(value);
        }

        const PerformanceOptions& SettingsModel::getPerformance() const
        {
            return _p->performance->get();
        }

        std::shared_ptr<dtk::IObservableValue<PerformanceOptions> > SettingsModel::observePerformance() const
        {
            return _p->performance;
        }

        void SettingsModel::setPerformance(const PerformanceOptions& value)
        {
            _p->performance->setIfChanged(value);
        }

        const TimelineOptions& SettingsModel::getTimeline() const
        {
            return _p->timeline->get();
        }

        const timelineui::ItemOptions& SettingsModel::getTimelineItem() const
        {
            return _p->timelineItem->get();
        }

        const timelineui::DisplayOptions& SettingsModel::getTimelineDisplay() const
        {
            return _p->timelineDisplay->get();
        }

        bool SettingsModel::getTimelineFirstTrack() const
        {
            return _p->timelineFirstTrack->get();
        }

        std::shared_ptr<dtk::IObservableValue<TimelineOptions> > SettingsModel::observeTimeline() const
        {
            return _p->timeline;
        }

        std::shared_ptr<dtk::IObservableValue<timelineui::ItemOptions> > SettingsModel::observeTimelineItem() const
        {
            return _p->timelineItem;
        }

        std::shared_ptr<dtk::IObservableValue<timelineui::DisplayOptions> > SettingsModel::observeTimelineDisplay() const
        {
            return _p->timelineDisplay;
        }
        
        std::shared_ptr<dtk::IObservableValue<bool> > SettingsModel::observeTimelineFirstTrack() const
        {
            return _p->timelineFirstTrack;
        }

        void SettingsModel::setTimeline(const TimelineOptions& value)
        {
            _p->timeline->setIfChanged(value);
        }

        void SettingsModel::setTimelineItem(const timelineui::ItemOptions& value)
        {
            _p->timelineItem->setIfChanged(value);
        }

        void SettingsModel::setTimelineDisplay(const timelineui::DisplayOptions& value)
        {
            _p->timelineDisplay->setIfChanged(value);
        }

        void SettingsModel::setTimelineFirstTrack(bool value)
        {
            _p->timelineFirstTrack->setIfChanged(value);
        }

        bool SettingsModel::getTooltipsEnabled() const
        {
            return _p->tooltipsEnabled->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > SettingsModel::observeTooltipsEnabled() const
        {
            return _p->tooltipsEnabled;
        }

        void SettingsModel::setTooltipsEnabled(bool value)
        {
            _p->tooltipsEnabled->setIfChanged(value);
        }

        void to_json(nlohmann::json& json, const CacheOptions& value)
        {
            json["sizeGB"] = value.sizeGB;
            json["readAhead"] = value.readAhead;
            json["readBehind"] = value.readBehind;
        }

        void to_json(nlohmann::json& json, const WindowOptions& in)
        {
            json = nlohmann::json
            {
                { "fileToolBar", in.fileToolBar },
                { "compareToolBar", in.compareToolBar },
                { "windowToolBar", in.windowToolBar },
                { "viewToolBar", in.viewToolBar },
                { "toolsToolBar", in.toolsToolBar },
                { "timeline", in.timeline },
                { "bottomToolBar", in.bottomToolBar },
                { "statusToolBar", in.statusToolBar },
                { "splitter", in.splitter },
                { "splitter2", in.splitter2 }
            };
        }

        void to_json(nlohmann::json& json, const FileSequenceOptions& value)
        {
            json["audio"] = timeline::to_string(value.audio);
            json["audioFileName"] = value.audioFileName;
            json["audioDirectory"] = value.audioDirectory;
            json["maxDigits"] = value.maxDigits;
        }

        void to_json(nlohmann::json& json, const PerformanceOptions& value)
        {
            json["audioBufferFrameCount"] = value.audioBufferFrameCount;
            json["videoRequestCount"] = value.videoRequestCount;
            json["audioRequestCount"] = value.audioRequestCount;
        }

        void to_json(nlohmann::json& json, const TimelineOptions& value)
        {
            json["editable"] = value.editable;
            json["frameView"] = value.frameView;
            json["scroll"] = value.scroll;
            json["stopOnScrub"] = value.stopOnScrub;
        }

        void from_json(const nlohmann::json& json, CacheOptions& value)
        {
            json["sizeGB"].get_to(value.sizeGB);
            json["readAhead"].get_to(value.readAhead);
            json["readBehind"].get_to(value.readBehind);
        }

        void from_json(const nlohmann::json& json, WindowOptions& out)
        {
            json.at("fileToolBar").get_to(out.fileToolBar);
            json.at("compareToolBar").get_to(out.compareToolBar);
            json.at("windowToolBar").get_to(out.windowToolBar);
            json.at("viewToolBar").get_to(out.viewToolBar);
            json.at("toolsToolBar").get_to(out.toolsToolBar);
            json.at("timeline").get_to(out.timeline);
            json.at("bottomToolBar").get_to(out.bottomToolBar);
            json.at("statusToolBar").get_to(out.statusToolBar);
            json.at("splitter").get_to(out.splitter);
            json.at("splitter2").get_to(out.splitter2);
        }

        void from_json(const nlohmann::json& json, FileSequenceOptions& value)
        {
            timeline::from_string(json["audio"].get<std::string>(), value.audio);
            json["audioFileName"].get_to(value.audioFileName);
            json["audioDirectory"].get_to(value.audioDirectory);
            json["maxDigits"].get_to(value.maxDigits);
        }

        void from_json(const nlohmann::json& json, PerformanceOptions& value)
        {
            json["audioBufferFrameCount"].get_to(value.audioBufferFrameCount);
            json["videoRequestCount"].get_to(value.videoRequestCount);
            json["audioRequestCount"].get_to(value.audioRequestCount);
        }

        void from_json(const nlohmann::json& json, TimelineOptions& value)
        {
            json["editable"].get_to(value.editable);
            json["frameView"].get_to(value.frameView);
            json["scroll"].get_to(value.scroll);
            json["stopOnScrub"].get_to(value.stopOnScrub);
        }
    }
}
