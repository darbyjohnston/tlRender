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

        dtk::Size2I getSize(ExportRenderSize value)
        {
            const std::array<dtk::Size2I, static_cast<size_t>(ExportRenderSize::Count)> data =
            {
                dtk::Size2I(),
                dtk::Size2I(1920, 1080),
                dtk::Size2I(3840, 2160),
                dtk::Size2I(4096, 2160),
                dtk::Size2I()
            };
            return data[static_cast<size_t>(value)];
        }

        DTK_ENUM_IMPL(
            ExportFileType,
            "Images",
            "Movie");

        bool ExportSettings::operator == (const ExportSettings& other) const
        {
            return
                directory == other.directory &&
                renderSize == other.renderSize &&
                customSize == other.customSize &&
                fileType == other.fileType &&
                imageBaseName == other.imageBaseName &&
                imageZeroPad == other.imageZeroPad &&
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

        ShortcutsSettings::ShortcutsSettings()
        {
            shortcuts =
            {
                Shortcut("Audio/VolumeUp", "Volume up", dtk::Key::Period),
                Shortcut("Audio/VolumeDown", "Volume down", dtk::Key::Comma),
                Shortcut("Audio/Mute", "Mute", dtk::Key::M),

                Shortcut("Compare/Next", "Next", dtk::Key::PageDown, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Compare/Prev", "Previous", dtk::Key::PageUp, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Compare/A", "A", dtk::Key::A, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Compare/B", "B", dtk::Key::B, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Compare/Wipe", "Wipe", dtk::Key::W, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Compare/Overlay", "Overlay"),
                Shortcut("Compare/Difference", "Difference"),
                Shortcut("Compare/Horizontal", "Horizontal"),
                Shortcut("Compare/Vertical", "Vertical"),
                Shortcut("Compare/Tile", "Tile", dtk::Key::T, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Compare/Relative", "Relative"),
                Shortcut("Compare/Absolute", "Absolute"),

                Shortcut("File/Open", "Open", dtk::Key::O, static_cast<int>(dtk::commandKeyModifier)),
                Shortcut(
                    "File/OpenSeparateAudio",
                    "Open separate audio",
                    dtk::Key::O,
                    static_cast<int>(dtk::KeyModifier::Shift) | static_cast<int>(dtk::commandKeyModifier)),
                Shortcut("File/Close", "Close", dtk::Key::E, static_cast<int>(dtk::commandKeyModifier)),
                Shortcut(
                    "File/CloseAll",
                    "Close all",
                    dtk::Key::E,
                    static_cast<int>(dtk::KeyModifier::Shift) | static_cast<int>(dtk::commandKeyModifier)),
                Shortcut(
                    "File/Reload",
                    "Reload",
                    dtk::Key::R,
                    static_cast<int>(dtk::KeyModifier::Shift) | static_cast<int>(dtk::commandKeyModifier)),
                Shortcut("File/Next", "Next", dtk::Key::PageDown, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("File/Prev", "Previous", dtk::Key::PageUp, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("File/NextLayer", "Next layer", dtk::Key::Equal, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("File/PrevLayer", "Previous layer", dtk::Key::Minus, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("File/Exit", "Exit", dtk::Key::Q, static_cast<int>(dtk::commandKeyModifier)),

                Shortcut("Frame/Start", "Start", dtk::Key::Home),
                Shortcut("Frame/End", "End", dtk::Key::End),
                Shortcut("Frame/Prev", "Previous", dtk::Key::Left),
                Shortcut("Frame/PrevX10", "Previous X10", dtk::Key::Left, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Frame/PrevX100", "Previous X100", dtk::Key::Left, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Frame/Next", "Next", dtk::Key::Right),
                Shortcut("Frame/NextX10", "Next X10", dtk::Key::Right, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Frame/NextX100", "Next X100", dtk::Key::Right, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Frame/FocusCurrent", "Focus current", dtk::Key::F, static_cast<int>(dtk::KeyModifier::Control)),

                Shortcut("Playback/Stop", "Stop", dtk::Key::K),
                Shortcut("Playback/Forward", "Forward", dtk::Key::L),
                Shortcut("Playback/Reverse", "Reverse", dtk::Key::J),
                Shortcut("Playback/Toggle", "Toggle", dtk::Key::Space),
                Shortcut("Playback/JumpBack1s", "Jump back 1s", dtk::Key::J, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Playback/JumpBack10s", "Jump back 10s", dtk::Key::J, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Playback/JumpForward1s", "Jump forward 1s", dtk::Key::L, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Playback/JumpForward10s", "Jump forward 10s", dtk::Key::L, static_cast<int>(dtk::KeyModifier::Control)),
                Shortcut("Playback/Loop", "Loop"),
                Shortcut("Playback/Once", "Once"),
                Shortcut("Playback/PingPong", "Ping pong"),
                Shortcut("Playback/SetInPoint", "Set in point", dtk::Key::I),
                Shortcut("Playback/ResetInPoint", "Reset in point", dtk::Key::I, static_cast<int>(dtk::KeyModifier::Shift)),
                Shortcut("Playback/SetOutPoint", "Set out point", dtk::Key::O),
                Shortcut("Playback/ResetOutPoint", "Reset out point", dtk::Key::O, static_cast<int>(dtk::KeyModifier::Shift)),

                Shortcut("Timeline/FrameView", "Frame view"),
                Shortcut("Timeline/Scroll", "Scroll"),
                Shortcut("Timeline/StopOnScrub", "Stop on scrub"),
                Shortcut("Timeline/Thumbnails", "Thumbnails"),
                Shortcut("Timeline/ThumbnailsSmall", "Thumbnails small"),
                Shortcut("Timeline/ThumbnailsMedium", "Thumbnails medium"),
                Shortcut("Timeline/ThumbnailsLarge", "Thumbnails large"),

                Shortcut("Tools/Files", "Files", dtk::Key::F1),
                Shortcut("Tools/Export", "Export", dtk::Key::F2),
                Shortcut("Tools/View", "View", dtk::Key::F3),
                Shortcut("Tools/ColorPicker", "Color picker", dtk::Key::F4),
                Shortcut("Tools/ColorControls", "Color controls", dtk::Key::F5),
                Shortcut("Tools/Info", "Information", dtk::Key::F6),
                Shortcut("Tools/Audio", "Audio", dtk::Key::F7),
                Shortcut("Tools/Devices", "Devices", dtk::Key::F8),
                Shortcut("Tools/Settings", "Settings", dtk::Key::F9),
                Shortcut("Tools/Messages", "Messages", dtk::Key::F10),
                Shortcut("Tools/SystemLog", "System log", dtk::Key::F11),

                Shortcut("View/Frame", "Frame", dtk::Key::Backspace),
                Shortcut("View/ZoomReset", "Zoom reset", dtk::Key::_0),
                Shortcut("View/ZoomIn", "Zoom in", dtk::Key::Equal),
                Shortcut("View/ZoomOut", "Zoom out", dtk::Key::Minus),
                Shortcut("View/Red", "Red", dtk::Key::R),
                Shortcut("View/Green", "Green", dtk::Key::G),
                Shortcut("View/Blue", "Blue", dtk::Key::B),
                Shortcut("View/Alpha", "Alpha", dtk::Key::A),
                Shortcut("View/MirrorHorizontal", "Mirror horizontal", dtk::Key::H),
                Shortcut("View/MirrorVertical", "Mirror vertical", dtk::Key::V),
                Shortcut("View/MinifyNearest", "Minify nearest"),
                Shortcut("View/MinifyLinear", "Minify linear"),
                Shortcut("View/MagnifyNearest", "Magnify nearest"),
                Shortcut("View/MagnifyLinear", "Magnify linear"),
                Shortcut("View/FromFile", "From file"),
                Shortcut("View/FullRange", "Full range"),
                Shortcut("View/LegalRange", "Legal range"),
                Shortcut("View/AlphaBlendNone", "Alpha blend none"),
                Shortcut("View/AlphaBlendStraight", "Alpha blend straight"),
                Shortcut("View/AlphaBlendPremultiplied", "Alpha blend premultiplied"),
                Shortcut("View/HUD", "HUD", dtk::Key::H, static_cast<int>(dtk::KeyModifier::Control)),

                Shortcut("Window/FullScreen", "Full screen", dtk::Key::U),
                Shortcut("Window/FloatOnTop", "Float on top"),
                Shortcut("Window/Secondary", "Secondary", dtk::Key::Y),
                Shortcut("Window/FileToolBar", "File tool bar"),
                Shortcut("Window/CompareToolBar", "Compare tool bar"),
                Shortcut("Window/WindowToolBar", "Window tool bar"),
                Shortcut("Window/ViewToolBar", "View tool bar"),
                Shortcut("Window/ToolsToolBar", "Tools tool bar"),
                Shortcut("Window/Timeline", "Timeline"),
                Shortcut("Window/BottomToolBar", "Bottom tool bar"),
                Shortcut("Window/StatusToolBar", "Status tool bar")
            };
        }

        bool ShortcutsSettings::operator == (const ShortcutsSettings& other) const
        {
            return shortcuts == other.shortcuts;
        }

        bool ShortcutsSettings::operator != (const ShortcutsSettings& other) const
        {
            return !(*this == other);
        }

        bool MiscSettings::operator == (const MiscSettings& other) const
        {
            return tooltipsEnabled == other.tooltipsEnabled;
        }

        bool MiscSettings::operator != (const MiscSettings& other) const
        {
            return !(*this == other);
        }

        DTK_ENUM_IMPL(
            MouseAction,
            "PanView",
            "CompareWipe",
            "ColorPicker",
            "FrameShuttle");

        bool MouseSettings::operator == (const MouseSettings& other) const
        {
            return actions == other.actions;
        }

        bool MouseSettings::operator != (const MouseSettings& other) const
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
                displayScale == other.displayScale &&
                colorControls == other.colorControls &&
                colorStyle == other.colorStyle &&
                customColorRoles == other.customColorRoles;
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
            std::shared_ptr<dtk::ObservableValue<ShortcutsSettings> > Shortcuts;
            std::shared_ptr<dtk::ObservableValue<MiscSettings> > misc;
            std::shared_ptr<dtk::ObservableValue<MouseSettings> > mouse;
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

            ShortcutsSettings Shortcuts;
            settings->getT("/Shortcuts", Shortcuts);
            p.Shortcuts = dtk::ObservableValue<ShortcutsSettings>::create(Shortcuts);

            MiscSettings misc;
            settings->getT("/Misc", misc);
            p.misc = dtk::ObservableValue<MiscSettings>::create(misc);

            MouseSettings mouse;
            settings->getT("/Mouse", mouse);
            p.mouse = dtk::ObservableValue<MouseSettings>::create(mouse);

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

            p.settings->setT("/Shortcuts", p.Shortcuts->get());

            p.settings->setT("/Misc", p.misc->get());

            p.settings->setT("/Mouse", p.mouse->get());

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
            setShortcuts(ShortcutsSettings());
            setMisc(MiscSettings());
            setMouse(MouseSettings());
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

        const ShortcutsSettings& SettingsModel::getShortcuts() const
        {
            return _p->Shortcuts->get();
        }

        std::shared_ptr<dtk::IObservableValue<ShortcutsSettings> > SettingsModel::observeShortcuts() const
        {
            return _p->Shortcuts;
        }

        void SettingsModel::setShortcuts(const ShortcutsSettings& value)
        {
            _p->Shortcuts->setIfChanged(value);
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

        const MouseSettings& SettingsModel::getMouse() const
        {
            return _p->mouse->get();
        }

        std::shared_ptr<dtk::IObservableValue<MouseSettings> > SettingsModel::observeMouse() const
        {
            return _p->mouse;
        }

        void SettingsModel::setMouse(const MouseSettings& value)
        {
            _p->mouse->setIfChanged(value);
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
            json["CustomSize"] = value.customSize;
            json["FileType"] = to_string(value.fileType);
            json["ImageBaseName"] = value.imageBaseName;
            json["ImageZeroPad"] = value.imageZeroPad;
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

        void to_json(nlohmann::json& json, const ShortcutsSettings& value)
        {
            for (const auto shortcut : value.shortcuts)
            {
                json["Shortcuts"].push_back(shortcut);
            }
        }

        void to_json(nlohmann::json& json, const MiscSettings& value)
        {
            json["TooltipsEnabled"] = value.tooltipsEnabled;
        }

        void to_json(nlohmann::json& json, const MouseSettings& value)
        {
            for (const auto& i : value.actions)
            {
                json["Actions"][to_string(i.first)] = to_string(i.second);
            }
        }

        void to_json(nlohmann::json& json, const PerformanceSettings& value)
        {
            json["AudioBufferFrameCount"] = value.audioBufferFrameCount;
            json["VideoRequestCount"] = value.videoRequestCount;
            json["AudioRequestCount"] = value.audioRequestCount;
        }

        void to_json(nlohmann::json& json, const StyleSettings& value)
        {
            json["DisplayScale"] = value.displayScale;
            json["ColorControls"] = value.colorControls;
            json["ColorStyle"] = to_string(value.colorStyle);
            for (auto i : value.customColorRoles)
            {
                json["CustomColorRoles"][dtk::getLabel(i.first)] = i.second;
            }
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
            json.at("CustomSize").get_to(value.customSize);
            from_string(json.at("FileType").get<std::string>(), value.fileType);
            json.at("ImageBaseName").get_to(value.imageBaseName);
            json.at("ImageZeroPad").get_to(value.imageZeroPad);
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

        void from_json(const nlohmann::json& json, MouseSettings& value)
        {
            for (auto i = json.at("Actions").begin(); i != json.at("Actions").end(); ++i)
            {
                MouseAction mouseAction = MouseAction::First;
                from_string(i.key(), mouseAction);
                from_string(i.value().get<std::string>(), value.actions[mouseAction]);
            }
        }

        void from_json(const nlohmann::json& json, ShortcutsSettings& value)
        {
            for (auto i = json.at("Shortcuts").begin(); i != json.at("Shortcuts").end(); ++i)
            {
                const Shortcut shortcut = i->get<Shortcut>();
                const auto j = std::find_if(
                    value.shortcuts.begin(),
                    value.shortcuts.end(),
                    [shortcut](const Shortcut& value)
                    {
                        return shortcut.name == value.name;
                    });
                if (j != value.shortcuts.end())
                {
                    *j = shortcut;
                }
                else
                {
                    value.shortcuts.push_back(shortcut);
                }
            }
        }

        void from_json(const nlohmann::json& json, PerformanceSettings& value)
        {
            json.at("AudioBufferFrameCount").get_to(value.audioBufferFrameCount);
            json.at("VideoRequestCount").get_to(value.videoRequestCount);
            json.at("AudioRequestCount").get_to(value.audioRequestCount);
        }

        void from_json(const nlohmann::json& json, StyleSettings& value)
        {
            json.at("DisplayScale").get_to(value.displayScale);
            from_string(json.at("ColorStyle").get<std::string>(), value.colorStyle);
            json.at("ColorControls").get_to(value.colorControls);
            for (auto i = json.at("CustomColorRoles").begin(); i != json.at("CustomColorRoles").end(); ++i)
            {
                dtk::ColorRole colorRole = dtk::ColorRole::None;
                from_string(i.key(), colorRole);
                i.value().get_to(value.customColorRoles[colorRole]);
            }
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
