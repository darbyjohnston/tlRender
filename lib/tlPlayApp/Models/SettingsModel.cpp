// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Models/SettingsModel.h>

#include <dtk/ui/Settings.h>
#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <sstream>

namespace tl
{
    namespace play
    {
        bool CacheSettings::operator == (const CacheSettings& other) const
        {
            return
                sizeGB == other.sizeGB &&
                readAhead == other.readAhead &&
                readBehind == other.readBehind;
        }

        bool CacheSettings::operator != (const CacheSettings& other) const
        {
            return !(*this == other);
        }

        DTK_ENUM_IMPL(
            ExportRenderSize,
            "Default",
            "1920x1080",
            "3840x2160",
            "4096x2160",
            "Custom");

        DTK_ENUM_IMPL(
            ExportFileType,
            "Images",
            "Movie");

        bool ExportSettings::operator == (const ExportSettings& other) const
        {
            return
                directory == other.directory &&
                renderSize == other.renderSize &&
                customRenderSize == other.customRenderSize &&
                fileType == other.fileType &&
                imageBaseName == other.imageBaseName &&
                imagePad == other.imagePad &&
                imageExtension == other.imageExtension &&
                movieBaseName == other.movieBaseName &&
                movieExtension == other.movieExtension &&
                movieCodec == other.movieCodec;
        }

        bool ExportSettings::operator != (const ExportSettings& other) const
        {
            return !(*this == other);
        }

        bool FileBrowserSettings::operator == (const FileBrowserSettings& other) const
        {
            return
                nativeFileDialog == other.nativeFileDialog &&
                path == other.path &&
                options == other.options;
        }

        bool FileBrowserSettings::operator != (const FileBrowserSettings& other) const
        {
            return !(*this == other);
        }

        bool FileSequenceSettings::operator == (const FileSequenceSettings& other) const
        {
            return
                audio == other.audio &&
                audioFileName == other.audioFileName &&
                audioDirectory == other.audioDirectory &&
                maxDigits == other.maxDigits &&
                io == other.io;
        }

        bool FileSequenceSettings::operator != (const FileSequenceSettings& other) const
        {
            return !(*this == other);
        }

        bool MiscSettings::operator == (const MiscSettings& other) const
        {
            return
                tooltipsEnabled == other.tooltipsEnabled;
        }

        bool MiscSettings::operator != (const MiscSettings& other) const
        {
            return !(*this == other);
        }

        bool PerformanceSettings::operator == (const PerformanceSettings& other) const
        {
            return
                audioBufferFrameCount == other.audioBufferFrameCount &&
                videoRequestCount == other.videoRequestCount &&
                audioRequestCount == other.audioRequestCount;
        }

        bool PerformanceSettings::operator != (const PerformanceSettings& other) const
        {
            return !(*this == other);
        }

        bool StyleSettings::operator == (const StyleSettings& other) const
        {
            return
                colorStyle == other.colorStyle &&
                displayScale == other.displayScale;
        }

        bool StyleSettings::operator != (const StyleSettings& other) const
        {
            return !(*this == other);
        }

        bool TimelineSettings::operator == (const TimelineSettings& other) const
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

        bool TimelineSettings::operator != (const TimelineSettings& other) const
        {
            return !(*this == other);
        }

        bool WindowSettings::operator == (const WindowSettings& other) const
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

        bool WindowSettings::operator != (const WindowSettings& other) const
        {
            return !(*this == other);
        }

        struct SettingsModel::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::Settings> settings;

            std::shared_ptr<dtk::ObservableValue<CacheSettings> > cache;
            std::shared_ptr<dtk::ObservableValue<ExportSettings> > exportSettings;
            std::shared_ptr<dtk::ObservableValue<FileBrowserSettings> > fileBrowser;
            std::shared_ptr<dtk::ObservableValue<FileSequenceSettings> > fileSequence;
            std::shared_ptr<dtk::ObservableValue<MiscSettings> > misc;
            std::shared_ptr<dtk::ObservableValue<PerformanceSettings> > performance;
            std::shared_ptr<dtk::ObservableValue<StyleSettings> > style;
            std::shared_ptr<dtk::ObservableValue<TimelineSettings> > timeline;
            std::shared_ptr<dtk::ObservableValue<WindowSettings> > window;
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

            CacheSettings cache;
            settings->getT("/Cache", cache);
            p.cache = dtk::ObservableValue<CacheSettings>::create(cache);

            ExportSettings exportSettings;
            settings->getT("/Export", exportSettings);
            p.exportSettings = dtk::ObservableValue<ExportSettings>::create(exportSettings);

            FileBrowserSettings fileBrowser;
            settings->getT("/FileBrowser", fileBrowser);
            p.fileBrowser = dtk::ObservableValue<FileBrowserSettings>::create(fileBrowser);
            auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
            fileBrowserSystem->setPath(fileBrowser.path);
            fileBrowserSystem->setOptions(fileBrowser.options);
            fileBrowserSystem->setNativeFileDialog(fileBrowser.nativeFileDialog);

            FileSequenceSettings fileSequence;
            settings->getT("/FileSequence", fileSequence);
            p.fileSequence = dtk::ObservableValue<FileSequenceSettings>::create(fileSequence);

            MiscSettings misc;
            settings->getT("/Misc", misc);
            p.misc = dtk::ObservableValue<MiscSettings>::create(misc);

            PerformanceSettings performance;
            settings->getT("/Performance", performance);
            p.performance = dtk::ObservableValue<PerformanceSettings>::create(performance);

            StyleSettings style;
            settings->getT("/Style", style);
            p.style = dtk::ObservableValue<StyleSettings>::create(style);

            TimelineSettings timeline;
            settings->getT("/Timeline", timeline);
            p.timeline = dtk::ObservableValue<TimelineSettings>::create(timeline);

            WindowSettings window;
            settings->getT("/Window", window);
            p.window = dtk::ObservableValue<WindowSettings>::create(window);

#if defined(TLRENDER_FFMPEG)
            ffmpeg::Options ffmpeg;
            settings->getT("/FFmpeg", ffmpeg);
            p.ffmpeg = dtk::ObservableValue<ffmpeg::Options>::create(ffmpeg);
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            usd::Options usd;
            settings->getT("/USD", usd);
            p.usd = dtk::ObservableValue<usd::Options>::create(usd);
#endif // TLRENDER_USD
        }

        SettingsModel::SettingsModel() :
            _p(new Private)
        {}

        SettingsModel::~SettingsModel()
        {
            DTK_P();
            p.settings->setT("/Cache", p.cache->get());

            p.settings->setT("/Export", p.exportSettings->get());

            FileBrowserSettings fileBrowser = p.fileBrowser->get();
            if (auto context = p.context.lock())
            {
                auto fileBrowserSystem = context->getSystem<dtk::FileBrowserSystem>();
                fileBrowser.path = fileBrowserSystem->getPath().u8string();
                fileBrowser.options = fileBrowserSystem->getOptions();
            }
            p.settings->setT("/FileBrowser", fileBrowser);

            p.settings->setT("/FileSequence", p.fileSequence->get());

            p.settings->setT("/Misc", p.misc->get());

            p.settings->setT("/Performance", p.performance->get());

            p.settings->setT("/Style", p.style->get());

            p.settings->setT("/Timeline", p.timeline->get());

            p.settings->setT("/Window", p.window->get());

#if defined(TLRENDER_FFMPEG)
            p.settings->setT("/FFmpeg", p.ffmpeg->get());
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            p.settings->setT("/USD", p.usd->get());
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
            setCache(CacheSettings());
            setExport(ExportSettings());
            setFileBrowser(FileBrowserSettings());
            setFileSequence(FileSequenceSettings());
            setMisc(MiscSettings());
            setPerformance(PerformanceSettings());
            setStyle(StyleSettings());
            setTimeline(TimelineSettings());
            setWindow(WindowSettings());
#if defined(TLRENDER_FFMPEG)
            setFFmpeg(ffmpeg::Options());
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            setUSD(usd::Options());
#endif // TLRENDER_USD
        }

        const CacheSettings& SettingsModel::getCache() const
        {
            return _p->cache->get();
        }

        std::shared_ptr<dtk::IObservableValue<CacheSettings> > SettingsModel::observeCache() const
        {
            return _p->cache;
        }

        void SettingsModel::setCache(const CacheSettings& value)
        {
            _p->cache->setIfChanged(value);
        }

        const ExportSettings& SettingsModel::getExport() const
        {
            return _p->exportSettings->get();
        }

        std::shared_ptr<dtk::IObservableValue<ExportSettings> > SettingsModel::observeExport() const
        {
            return _p->exportSettings;
        }

        void SettingsModel::setExport(const ExportSettings& value)
        {
            _p->exportSettings->setIfChanged(value);
        }

        const FileBrowserSettings& SettingsModel::getFileBrowser() const
        {
            return _p->fileBrowser->get();
        }

        std::shared_ptr<dtk::IObservableValue<FileBrowserSettings> > SettingsModel::observeFileBrowser() const
        {
            return _p->fileBrowser;
        }

        void SettingsModel::setFileBrowser(const FileBrowserSettings& value)
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

        const FileSequenceSettings& SettingsModel::getFileSequence() const
        {
            return _p->fileSequence->get();
        }

        std::shared_ptr<dtk::IObservableValue<FileSequenceSettings> > SettingsModel::observeFileSequence() const
        {
            return _p->fileSequence;
        }

        void SettingsModel::setFileSequence(const FileSequenceSettings& value)
        {
            _p->fileSequence->setIfChanged(value);
        }

        const MiscSettings& SettingsModel::getMisc() const
        {
            return _p->misc->get();
        }

        std::shared_ptr<dtk::IObservableValue<MiscSettings> > SettingsModel::observeMisc() const
        {
            return _p->misc;
        }

        void SettingsModel::setMisc(const MiscSettings& value)
        {
            _p->misc->setIfChanged(value);
        }

        const PerformanceSettings& SettingsModel::getPerformance() const
        {
            return _p->performance->get();
        }

        std::shared_ptr<dtk::IObservableValue<PerformanceSettings> > SettingsModel::observePerformance() const
        {
            return _p->performance;
        }

        void SettingsModel::setPerformance(const PerformanceSettings& value)
        {
            _p->performance->setIfChanged(value);
        }

        const StyleSettings& SettingsModel::getStyle() const
        {
            return _p->style->get();
        }

        std::shared_ptr<dtk::IObservableValue<StyleSettings> > SettingsModel::observeStyle() const
        {
            return _p->style;
        }

        void SettingsModel::setStyle(const StyleSettings& value)
        {
            _p->style->setIfChanged(value);
        }

        const TimelineSettings& SettingsModel::getTimeline() const
        {
            return _p->timeline->get();
        }

        std::shared_ptr<dtk::IObservableValue<TimelineSettings> > SettingsModel::observeTimeline() const
        {
            return _p->timeline;
        }

        void SettingsModel::setTimeline(const TimelineSettings& value)
        {
            _p->timeline->setIfChanged(value);
        }

        const WindowSettings& SettingsModel::getWindow() const
        {
            return _p->window->get();
        }

        std::shared_ptr<dtk::IObservableValue<WindowSettings> > SettingsModel::observeWindow() const
        {
            return _p->window;
        }

        void SettingsModel::setWindow(const WindowSettings& value)
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

        void to_json(nlohmann::json& json, const CacheSettings& value)
        {
            json["SizeGB"] = value.sizeGB;
            json["ReadAhead"] = value.readAhead;
            json["ReadBehind"] = value.readBehind;
        }

        void to_json(nlohmann::json& json, const ExportSettings& value)
        {
            json["Directory"] = value.directory;
            json["RenderSize"] = to_string(value.renderSize);
            json["CustomRenderSize"] = value.customRenderSize;
            json["FileType"] = to_string(value.fileType);
            json["ImageBaseName"] = value.imageBaseName;
            json["ImagePad"] = value.imagePad;
            json["ImageExtension"] = value.imageExtension;
            json["MovieBaseName"] = value.movieBaseName;
            json["MovieExtension"] = value.movieExtension;
            json["MovieCodec"] = value.movieCodec;
        }

        void to_json(nlohmann::json& json, const FileBrowserSettings& value)
        {
            json["NativeFileDialog"] = value.nativeFileDialog;
            json["Path"] = value.path;
            json["Options"] = value.options;
        }

        void to_json(nlohmann::json& json, const FileSequenceSettings& value)
        {
            json["Audio"] = timeline::to_string(value.audio);
            json["AudioFileName"] = value.audioFileName;
            json["AudioDirectory"] = value.audioDirectory;
            json["MaxDigits"] = value.maxDigits;
            json["IO"] = value.io;
        }

        void to_json(nlohmann::json& json, const MiscSettings& value)
        {
            json["TooltipsEnabled"] = value.tooltipsEnabled;
        }

        void to_json(nlohmann::json& json, const PerformanceSettings& value)
        {
            json["AudioBufferFrameCount"] = value.audioBufferFrameCount;
            json["VideoRequestCount"] = value.videoRequestCount;
            json["AudioRequestCount"] = value.audioRequestCount;
        }

        void to_json(nlohmann::json& json, const StyleSettings& value)
        {
            json["ColorStyle"] = value.colorStyle;
            json["DisplayScale"] = value.displayScale;
        }

        void to_json(nlohmann::json& json, const TimelineSettings& value)
        {
            json["Editable"] = value.editable;
            json["FrameView"] = value.frameView;
            json["Scroll"] = value.scroll;
            json["StopOnScrub"] = value.stopOnScrub;
            json["Item"] = value.item;
            json["Display"] = value.display;
            json["FirstTrack"] = value.firstTrack;
        }

        void to_json(nlohmann::json& json, const WindowSettings& in)
        {
            json = nlohmann::json
            {
                { "Size", in.size },
                { "FileToolBar", in.fileToolBar },
                { "CompareToolBar", in.compareToolBar },
                { "WindowToolBar", in.windowToolBar },
                { "ViewToolBar", in.viewToolBar },
                { "ToolsToolBar", in.toolsToolBar },
                { "Timeline", in.timeline },
                { "BottomToolBar", in.bottomToolBar },
                { "StatusToolBar", in.statusToolBar },
                { "Splitter", in.splitter },
                { "Splitter2", in.splitter2 }
            };
        }

        void from_json(const nlohmann::json& json, CacheSettings& value)
        {
            json.at("SizeGB").get_to(value.sizeGB);
            json.at("ReadAhead").get_to(value.readAhead);
            json.at("ReadBehind").get_to(value.readBehind);
        }

        void from_json(const nlohmann::json& json, ExportSettings& value)
        {
            json.at("Directory").get_to(value.directory);
            from_string(json.at("RenderSize").get<std::string>(), value.renderSize);
            json.at("CustomRenderSize").get_to(value.customRenderSize);
            from_string(json.at("FileType").get<std::string>(), value.fileType);
            json.at("ImageBaseName").get_to(value.imageBaseName);
            json.at("ImagePad").get_to(value.imagePad);
            json.at("ImageExtension").get_to(value.imageExtension);
            json.at("MovieBaseName").get_to(value.movieBaseName);
            json.at("MovieExtension").get_to(value.movieExtension);
            json.at("MovieCodec").get_to(value.movieCodec);
        }

        void from_json(const nlohmann::json& json, FileBrowserSettings& value)
        {
            json.at("NativeFileDialog").get_to(value.nativeFileDialog);
            json.at("Path").get_to(value.path);
            json.at("Options").get_to(value.options);
        }

        void from_json(const nlohmann::json& json, FileSequenceSettings& value)
        {
            timeline::from_string(json.at("Audio").get<std::string>(), value.audio);
            json.at("AudioFileName").get_to(value.audioFileName);
            json.at("AudioDirectory").get_to(value.audioDirectory);
            json.at("MaxDigits").get_to(value.maxDigits);
            json.at("IO").get_to(value.io);
        }

        void from_json(const nlohmann::json& json, MiscSettings& value)
        {
            json.at("TooltipsEnabled").get_to(value.tooltipsEnabled);
        }

        void from_json(const nlohmann::json& json, PerformanceSettings& value)
        {
            json.at("AudioBufferFrameCount").get_to(value.audioBufferFrameCount);
            json.at("VideoRequestCount").get_to(value.videoRequestCount);
            json.at("AudioRequestCount").get_to(value.audioRequestCount);
        }

        void from_json(const nlohmann::json& json, StyleSettings& value)
        {
            json.at("ColorStyle").get_to(value.colorStyle);
            json.at("DisplayScale").get_to(value.displayScale);
        }

        void from_json(const nlohmann::json& json, TimelineSettings& value)
        {
            json.at("Editable").get_to(value.editable);
            json.at("FrameView").get_to(value.frameView);
            json.at("Scroll").get_to(value.scroll);
            json.at("StopOnScrub").get_to(value.stopOnScrub);
            json.at("Item").get_to(value.item);
            json.at("Display").get_to(value.display);
            json.at("FirstTrack").get_to(value.firstTrack);
        }

        void from_json(const nlohmann::json& json, WindowSettings& out)
        {
            json.at("Size").get_to(out.size);
            json.at("FileToolBar").get_to(out.fileToolBar);
            json.at("CompareToolBar").get_to(out.compareToolBar);
            json.at("WindowToolBar").get_to(out.windowToolBar);
            json.at("ViewToolBar").get_to(out.viewToolBar);
            json.at("ToolsToolBar").get_to(out.toolsToolBar);
            json.at("Timeline").get_to(out.timeline);
            json.at("BottomToolBar").get_to(out.bottomToolBar);
            json.at("StatusToolBar").get_to(out.statusToolBar);
            json.at("Splitter").get_to(out.splitter);
            json.at("Splitter2").get_to(out.splitter2);
        }
    }
}
