// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Models/SettingsModel.h>

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

        bool FileBrowserOptions::operator == (const FileBrowserOptions& other) const
        {
            return
                nativeFileDialog == other.nativeFileDialog &&
                path == other.path &&
                options == other.options;
        }

        bool FileBrowserOptions::operator != (const FileBrowserOptions& other) const
        {
            return !(*this == other);
        }

        bool FileSequenceOptions::operator == (const FileSequenceOptions& other) const
        {
            return
                audio == other.audio &&
                audioFileName == other.audioFileName &&
                audioDirectory == other.audioDirectory &&
                maxDigits == other.maxDigits &&
                io == other.io;
        }

        bool FileSequenceOptions::operator != (const FileSequenceOptions& other) const
        {
            return !(*this == other);
        }

        bool MiscOptions::operator == (const MiscOptions& other) const
        {
            return
                tooltipsEnabled == other.tooltipsEnabled;
        }

        bool MiscOptions::operator != (const MiscOptions& other) const
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

        bool StyleOptions::operator == (const StyleOptions& other) const
        {
            return
                colorStyle == other.colorStyle &&
                displayScale == other.displayScale;
        }

        bool StyleOptions::operator != (const StyleOptions& other) const
        {
            return !(*this == other);
        }

        bool TimelineOptions::operator == (const TimelineOptions& other) const
        {
            return
                editable == other.editable &&
                frameView == other.frameView &&
                scroll == other.scroll &&
                stopOnScrub == other.stopOnScrub &&
                item == other.item &&
                display == other.display &&
                firstTrack == other.firstTrack;
        }

        bool TimelineOptions::operator != (const TimelineOptions& other) const
        {
            return !(*this == other);
        }

        bool WindowOptions::operator == (const WindowOptions& other) const
        {
            return
                size == other.size &&
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

        struct SettingsModel::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::Settings> settings;

            std::shared_ptr<dtk::ObservableValue<CacheOptions> > cache;
            std::shared_ptr<dtk::ObservableValue<FileBrowserOptions> > fileBrowser;
            std::shared_ptr<dtk::ObservableValue<FileSequenceOptions> > fileSequence;
            std::shared_ptr<dtk::ObservableValue<MiscOptions> > misc;
            std::shared_ptr<dtk::ObservableValue<PerformanceOptions> > performance;
            std::shared_ptr<dtk::ObservableValue<StyleOptions> > style;
            std::shared_ptr<dtk::ObservableValue<TimelineOptions> > timeline;
            std::shared_ptr<dtk::ObservableValue<WindowOptions> > window;
#if defined(TLRENDER_FFMPEG)
            std::shared_ptr<dtk::ObservableValue<ffmpeg::Options> > ffmpeg;
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            std::shared_ptr<dtk::ObservableValue<usd::Options> > usd;
#endif // TLRENDER_USD
        };

        void SettingsModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Settings>& settings)
        {
            DTK_P();

            p.context = context;
            p.settings = settings;

            CacheOptions cache;
            settings->getT("Cache", cache);
            p.cache = dtk::ObservableValue<CacheOptions>::create(cache);

            FileBrowserOptions fileBrowser;
            settings->getT("FileBrowser", fileBrowser);
            p.fileBrowser = dtk::ObservableValue<FileBrowserOptions>::create(fileBrowser);
            auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
            fileBrowserSystem->setPath(fileBrowser.path);
            fileBrowserSystem->setOptions(fileBrowser.options);
            fileBrowserSystem->setNativeFileDialog(fileBrowser.nativeFileDialog);

            FileSequenceOptions fileSequence;
            settings->getT("FileSequence", fileSequence);
            p.fileSequence = dtk::ObservableValue<FileSequenceOptions>::create(fileSequence);

            MiscOptions misc;
            settings->getT("Misc", misc);
            p.misc = dtk::ObservableValue<MiscOptions>::create(misc);

            PerformanceOptions performance;
            settings->getT("Performance", performance);
            p.performance = dtk::ObservableValue<PerformanceOptions>::create(performance);

            StyleOptions style;
            settings->getT("Style", style);
            p.style = dtk::ObservableValue<StyleOptions>::create(style);

            TimelineOptions timeline;
            settings->getT("Timeline", timeline);
            p.timeline = dtk::ObservableValue<TimelineOptions>::create(timeline);

            WindowOptions window;
            settings->getT("Window", window);
            p.window = dtk::ObservableValue<WindowOptions>::create(window);

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
        }

        SettingsModel::SettingsModel() :
            _p(new Private)
        {}

        SettingsModel::~SettingsModel()
        {
            DTK_P();
            p.settings->setT("Cache", p.cache->get());

            FileBrowserOptions fileBrowser = p.fileBrowser->get();
            if (auto context = p.context.lock())
            {
                auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
                fileBrowser.path = fileBrowserSystem->getPath().u8string();
                fileBrowser.options = fileBrowserSystem->getOptions();
            }
            p.settings->setT("FileBrowser", fileBrowser);

            p.settings->setT("FileSequence", p.fileSequence->get());

            p.settings->setT("Misc", p.misc->get());

            p.settings->setT("Performance", p.performance->get());

            p.settings->setT("Style", p.style->get());

            p.settings->setT("Timeline", p.timeline->get());

            p.settings->setT("Window", p.window->get());

#if defined(TLRENDER_FFMPEG)
            p.settings->setT("FFmpeg", p.ffmpeg->get());
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            p.settings->setT("USD", p.usd->get());
#endif // TLRENDER_USD
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
            setFileBrowser(FileBrowserOptions());
            setFileSequence(FileSequenceOptions());
            setMisc(MiscOptions());
            setPerformance(PerformanceOptions());
            setStyle(StyleOptions());
            setTimeline(TimelineOptions());
            setWindow(WindowOptions());
#if defined(TLRENDER_FFMPEG)
            setFFmpeg(ffmpeg::Options());
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            setUSD(usd::Options());
#endif // TLRENDER_USD
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

        const FileBrowserOptions& SettingsModel::getFileBrowser() const
        {
            return _p->fileBrowser->get();
        }

        std::shared_ptr<dtk::IObservableValue<FileBrowserOptions> > SettingsModel::observeFileBrowser() const
        {
            return _p->fileBrowser;
        }

        void SettingsModel::setFileBrowser(const FileBrowserOptions& value)
        {
            DTK_P();
            if (p.fileBrowser->setIfChanged(value))
            {
                if (auto context = p.context.lock())
                {
                    auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
                    fileBrowserSystem->setNativeFileDialog(value.nativeFileDialog);
                }
            }
        }

        const FileSequenceOptions& SettingsModel::getFileSequence() const
        {
            return _p->fileSequence->get();
        }

        std::shared_ptr<dtk::IObservableValue<FileSequenceOptions> > SettingsModel::observeFileSequence() const
        {
            return _p->fileSequence;
        }

        void SettingsModel::setFileSequence(const FileSequenceOptions& value)
        {
            _p->fileSequence->setIfChanged(value);
        }

        const MiscOptions& SettingsModel::getMisc() const
        {
            return _p->misc->get();
        }

        std::shared_ptr<dtk::IObservableValue<MiscOptions> > SettingsModel::observeMisc() const
        {
            return _p->misc;
        }

        void SettingsModel::setMisc(const MiscOptions& value)
        {
            _p->misc->setIfChanged(value);
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

        const StyleOptions& SettingsModel::getStyle() const
        {
            return _p->style->get();
        }

        std::shared_ptr<dtk::IObservableValue<StyleOptions> > SettingsModel::observeStyle() const
        {
            return _p->style;
        }

        void SettingsModel::setStyle(const StyleOptions& value)
        {
            _p->style->setIfChanged(value);
        }

        const TimelineOptions& SettingsModel::getTimeline() const
        {
            return _p->timeline->get();
        }

        std::shared_ptr<dtk::IObservableValue<TimelineOptions> > SettingsModel::observeTimeline() const
        {
            return _p->timeline;
        }

        void SettingsModel::setTimeline(const TimelineOptions& value)
        {
            _p->timeline->setIfChanged(value);
        }

        const WindowOptions& SettingsModel::getWindow() const
        {
            return _p->window->get();
        }

        std::shared_ptr<dtk::IObservableValue<WindowOptions> > SettingsModel::observeWindow() const
        {
            return _p->window;
        }

        void SettingsModel::setWindow(const WindowOptions& value)
        {
            _p->window->setIfChanged(value);
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

        void to_json(nlohmann::json& json, const CacheOptions& value)
        {
            json["sizeGB"] = value.sizeGB;
            json["readAhead"] = value.readAhead;
            json["readBehind"] = value.readBehind;
        }

        void to_json(nlohmann::json& json, const FileBrowserOptions& value)
        {
            json["nativeFileDialog"] = value.nativeFileDialog;
            json["path"] = value.path;
            json["options"] = value.options;
        }

        void to_json(nlohmann::json& json, const FileSequenceOptions& value)
        {
            json["audio"] = timeline::to_string(value.audio);
            json["audioFileName"] = value.audioFileName;
            json["audioDirectory"] = value.audioDirectory;
            json["maxDigits"] = value.maxDigits;
            json["io"] = value.io;
        }

        void to_json(nlohmann::json& json, const MiscOptions& value)
        {
            json["tooltipsEnabled"] = value.tooltipsEnabled;
        }

        void to_json(nlohmann::json& json, const PerformanceOptions& value)
        {
            json["audioBufferFrameCount"] = value.audioBufferFrameCount;
            json["videoRequestCount"] = value.videoRequestCount;
            json["audioRequestCount"] = value.audioRequestCount;
        }

        void to_json(nlohmann::json& json, const StyleOptions& value)
        {
            json["colorStyle"] = value.colorStyle;
            json["displayScale"] = value.displayScale;
        }

        void to_json(nlohmann::json& json, const TimelineOptions& value)
        {
            json["editable"] = value.editable;
            json["frameView"] = value.frameView;
            json["scroll"] = value.scroll;
            json["stopOnScrub"] = value.stopOnScrub;
            json["item"] = value.item;
            json["display"] = value.display;
            json["firstTrack"] = value.firstTrack;
        }

        void to_json(nlohmann::json& json, const WindowOptions& in)
        {
            json = nlohmann::json
            {
                { "size", in.size },
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

        void from_json(const nlohmann::json& json, CacheOptions& value)
        {
            json.at("sizeGB").get_to(value.sizeGB);
            json.at("readAhead").get_to(value.readAhead);
            json.at("readBehind").get_to(value.readBehind);
        }

        void from_json(const nlohmann::json& json, FileBrowserOptions& value)
        {
            json.at("nativeFileDialog").get_to(value.nativeFileDialog);
            json.at("path").get_to(value.path);
            json.at("options").get_to(value.options);
        }

        void from_json(const nlohmann::json& json, FileSequenceOptions& value)
        {
            timeline::from_string(json.at("audio").get<std::string>(), value.audio);
            json.at("audioFileName").get_to(value.audioFileName);
            json.at("audioDirectory").get_to(value.audioDirectory);
            json.at("maxDigits").get_to(value.maxDigits);
            json.at("io").get_to(value.io);
        }

        void from_json(const nlohmann::json& json, MiscOptions& value)
        {
            json.at("tooltipsEnabled").get_to(value.tooltipsEnabled);
        }

        void from_json(const nlohmann::json& json, PerformanceOptions& value)
        {
            json.at("audioBufferFrameCount").get_to(value.audioBufferFrameCount);
            json.at("videoRequestCount").get_to(value.videoRequestCount);
            json.at("audioRequestCount").get_to(value.audioRequestCount);
        }

        void from_json(const nlohmann::json& json, StyleOptions& value)
        {
            json.at("colorStyle").get_to(value.colorStyle);
            json.at("displayScale").get_to(value.displayScale);
        }

        void from_json(const nlohmann::json& json, TimelineOptions& value)
        {
            json.at("editable").get_to(value.editable);
            json.at("frameView").get_to(value.frameView);
            json.at("scroll").get_to(value.scroll);
            json.at("stopOnScrub").get_to(value.stopOnScrub);
            json.at("item").get_to(value.item);
            json.at("display").get_to(value.display);
            json.at("firstTrack").get_to(value.firstTrack);
        }

        void from_json(const nlohmann::json& json, WindowOptions& out)
        {
            json.at("size").get_to(out.size);
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
    }
}
