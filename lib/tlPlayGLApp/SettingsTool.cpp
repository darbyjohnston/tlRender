// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SettingsTool.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>
#include <tlPlayGLApp/Style.h>

#include <tlUI/Bellows.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/DoubleEdit.h>
#include <tlUI/GridLayout.h>
#include <tlUI/IntEdit.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
#include <tlUI/MessageDialog.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<ui::IntEdit> cacheSize;
            std::shared_ptr<ui::DoubleEdit> readAhead;
            std::shared_ptr<ui::DoubleEdit> readBehind;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void CacheSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::CacheSettingsWidget", context, parent);
            TLRENDER_P();

            p.cacheSize = ui::IntEdit::create(context);
            p.cacheSize->setRange(math::IntRange(0, 1024));

            p.readAhead = ui::DoubleEdit::create(context);
            p.readAhead->setRange(math::DoubleRange(0.0, 60.0));
            p.readAhead->setStep(1.0);
            p.readAhead->setLargeStep(10.0);

            p.readBehind = ui::DoubleEdit::create(context);
            p.readBehind->setRange(math::DoubleRange(0.0, 60.0));
            p.readBehind->setStep(1.0);
            p.readBehind->setLargeStep(10.0);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Cache size (GB):", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.cacheSize->setParent(p.layout);
            p.layout->setGridPos(p.cacheSize, 0, 1);
            label = ui::Label::create("Read ahead (seconds):", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.readAhead->setParent(p.layout);
            p.layout->setGridPos(p.readAhead, 1, 1);
            label = ui::Label::create("Read behind (seconds):", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.readBehind->setParent(p.layout);
            p.layout->setGridPos(p.readBehind, 2, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.cacheSize->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/Size", value);
                    }
                });

            p.readAhead->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/ReadAhead", value);
                    }
                });

            p.readBehind->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/ReadBehind", value);
                    }
                });

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            int value = 0;
                            settings->getValue<int>("Cache/Size", value);
                            p.cacheSize->setValue(value);
                        }
                        {
                            double value = 0.0;
                            settings->getValue<double>("Cache/ReadAhead", value);
                            p.readAhead->setValue(value);
                        }
                        {
                            double value = 0.0;
                            settings->getValue<double>("Cache/ReadBehind", value);
                            p.readBehind->setValue(value);
                        }
                    }
                });
        }

        CacheSettingsWidget::CacheSettingsWidget() :
            _p(new Private)
        {}

        CacheSettingsWidget::~CacheSettingsWidget()
        {}

        std::shared_ptr<CacheSettingsWidget> CacheSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CacheSettingsWidget>(new CacheSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void CacheSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CacheSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<ui::ComboBox> audioComboBox;
            std::shared_ptr<ui::LineEdit> audioFileNameEdit;
            std::shared_ptr<ui::LineEdit> audioDirectoryEdit;
            std::shared_ptr<ui::IntEdit> maxDigitsEdit;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void FileSequenceSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::FileSequenceSettingsWidget", context, parent);
            TLRENDER_P();

            p.audioComboBox = ui::ComboBox::create(
                timeline::getFileSequenceAudioLabels(),
                context);

            p.audioFileNameEdit = ui::LineEdit::create(context);

            p.audioDirectoryEdit = ui::LineEdit::create(context);

            p.maxDigitsEdit = ui::IntEdit::create(context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Audio:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.audioComboBox->setParent(p.layout);
            p.layout->setGridPos(p.audioComboBox, 0, 1);
            label = ui::Label::create("Audio file name:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.audioFileNameEdit->setParent(p.layout);
            p.layout->setGridPos(p.audioFileNameEdit, 1, 1);
            label = ui::Label::create("Audio directory:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.audioDirectoryEdit->setParent(p.layout);
            p.layout->setGridPos(p.audioDirectoryEdit, 2, 1);
            label = ui::Label::create("Maximum digits:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.maxDigitsEdit->setParent(p.layout);
            p.layout->setGridPos(p.maxDigitsEdit, 3, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            timeline::FileSequenceAudio value = timeline::FileSequenceAudio::First;
                            settings->getValue("FileSequence/Audio", value);
                            p.audioComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                        {
                            std::string value;
                            settings->getValue("FileSequence/AudioFileName", value);
                            p.audioFileNameEdit->setText(value);
                        }
                        {
                            std::string value;
                            settings->getValue("FileSequence/AudioDirectory", value);
                            p.audioDirectoryEdit->setText(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("FileSequence/MaxDigits", value);
                            p.maxDigitsEdit->setValue(value);
                        }
                    }
                });

            p.audioComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "FileSequence/Audio",
                            static_cast<timeline::FileSequenceAudio>(value));
                    }
                });
        }

        FileSequenceSettingsWidget::FileSequenceSettingsWidget() :
            _p(new Private)
        {}

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget()
        {}

        std::shared_ptr<FileSequenceSettingsWidget> FileSequenceSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileSequenceSettingsWidget>(new FileSequenceSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void FileSequenceSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileSequenceSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct FileBrowserSettingsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> nativeFileDialogCheckBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void FileBrowserSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::FileBrowserSettingsWidget", context, parent);
            TLRENDER_P();

            p.nativeFileDialogCheckBox = ui::CheckBox::create(context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Native file dialog:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.nativeFileDialogCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.nativeFileDialogCheckBox, 0, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            bool value = false;
                            settings->getValue("FileBrowser/NativeFileDialog", value);
                            p.nativeFileDialogCheckBox->setChecked(value);
                        }
                    }
                });

            p.nativeFileDialogCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("FileBrowser/NativeFileDialog", value);
                    }
                });
        }

        FileBrowserSettingsWidget::FileBrowserSettingsWidget() :
            _p(new Private)
        {}

        FileBrowserSettingsWidget::~FileBrowserSettingsWidget()
        {}

        std::shared_ptr<FileBrowserSettingsWidget> FileBrowserSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowserSettingsWidget>(new FileBrowserSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void FileBrowserSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileBrowserSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<ui::ComboBox> timerComboBox;
            std::shared_ptr<ui::IntEdit> audioBufferFramesEdit;
            std::shared_ptr<ui::IntEdit> videoRequestsEdit;
            std::shared_ptr<ui::IntEdit> audioRequestsEdit;
            std::shared_ptr<ui::IntEdit> sequenceThreadsEdit;
            std::shared_ptr<ui::CheckBox> ffmpegYUVtoRGBCheckBox;
            std::shared_ptr<ui::IntEdit> ffmpegThreadsEdit;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void PerformanceSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::PerformanceSettingsWidget", context, parent);
            TLRENDER_P();

            p.timerComboBox = ui::ComboBox::create(
                timeline::getTimerModeLabels(), context);

            p.audioBufferFramesEdit = ui::IntEdit::create(context);
            p.audioBufferFramesEdit->setRange(math::IntRange(1024, 4096));
            p.audioBufferFramesEdit->setStep(256);
            p.audioBufferFramesEdit->setLargeStep(1024);

            p.videoRequestsEdit = ui::IntEdit::create(context);
            p.videoRequestsEdit->setRange(math::IntRange(1, 64));

            p.audioRequestsEdit = ui::IntEdit::create(context);
            p.audioRequestsEdit->setRange(math::IntRange(1, 64));

            p.sequenceThreadsEdit = ui::IntEdit::create(context);
            p.sequenceThreadsEdit->setRange(math::IntRange(1, 64));

            p.ffmpegYUVtoRGBCheckBox = ui::CheckBox::create(context);

            p.ffmpegThreadsEdit = ui::IntEdit::create(context);
            p.ffmpegThreadsEdit->setRange(math::IntRange(0, 64));

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Changes are applied to new files.", context, p.layout);
            auto griLayout = ui::GridLayout::create(context, p.layout);
            griLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            label = ui::Label::create("Timer mode:", context, griLayout);
            griLayout->setGridPos(label, 0, 0);
            p.timerComboBox->setParent(griLayout);
            griLayout->setGridPos(p.timerComboBox, 0, 1);
            label = ui::Label::create("Audio buffer frames:", context, griLayout);
            griLayout->setGridPos(label, 1, 0);
            p.audioBufferFramesEdit->setParent(griLayout);
            griLayout->setGridPos(p.audioBufferFramesEdit, 1, 1);
            label = ui::Label::create("Video requests:", context, griLayout);
            griLayout->setGridPos(label, 2, 0);
            p.videoRequestsEdit->setParent(griLayout);
            griLayout->setGridPos(p.videoRequestsEdit, 2, 1);
            label = ui::Label::create("Audio requests:", context, griLayout);
            griLayout->setGridPos(label, 3, 0);
            p.audioRequestsEdit->setParent(griLayout);
            griLayout->setGridPos(p.audioRequestsEdit, 3, 1);
            label = ui::Label::create("Sequence I/O threads:", context, griLayout);
            griLayout->setGridPos(label, 4, 0);
            p.sequenceThreadsEdit->setParent(griLayout);
            griLayout->setGridPos(p.sequenceThreadsEdit, 4, 1);
            label = ui::Label::create("FFmpeg YUV to RGB conversion:", context, griLayout);
            griLayout->setGridPos(label, 5, 0);
            p.ffmpegYUVtoRGBCheckBox->setParent(griLayout);
            griLayout->setGridPos(p.ffmpegYUVtoRGBCheckBox, 5, 1);
            label = ui::Label::create("FFmpeg I/O threads:", context, griLayout);
            griLayout->setGridPos(label, 6, 0);
            p.ffmpegThreadsEdit->setParent(griLayout);
            griLayout->setGridPos(p.ffmpegThreadsEdit, 6, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.timerComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/TimerMode",
                            static_cast<timeline::TimerMode>(value));
                    }
                });

            p.audioBufferFramesEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/AudioBufferFrameCount",
                            value);
                    }
                });

            p.videoRequestsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/VideoRequestCount",
                            value);
                    }
                });

            p.audioRequestsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/AudioRequestCount",
                            value);
                    }
                });

            p.sequenceThreadsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/SequenceThreadCount",
                            value);
                    }
                });

            p.ffmpegYUVtoRGBCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/FFmpegYUVToRGBConversion",
                            value);
                    }
                });

            p.ffmpegThreadsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/FFmpegThreadCount",
                            value);
                    }
                });

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            timeline::TimerMode value = timeline::TimerMode::First;
                            settings->getValue("Performance/TimerMode", value);
                            p.timerComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/AudioBufferFrameCount", value);
                            p.audioBufferFramesEdit->setValue(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/VideoRequestCount", value);
                            p.videoRequestsEdit->setValue(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/AudioRequestCount", value);
                            p.audioRequestsEdit->setValue(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/SequenceThreadCount", value);
                            p.sequenceThreadsEdit->setValue(value);
                        }
                        {
                            bool value = false;
                            settings->getValue("Performance/FFmpegYUVToRGBConversion", value);
                            p.ffmpegYUVtoRGBCheckBox->setChecked(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/FFmpegThreadCount", value);
                            p.ffmpegThreadsEdit->setValue(value);
                        }
                    }
                });
        }

        PerformanceSettingsWidget::PerformanceSettingsWidget() :
            _p(new Private)
        {}

        PerformanceSettingsWidget::~PerformanceSettingsWidget()
        {}

        std::shared_ptr<PerformanceSettingsWidget> PerformanceSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PerformanceSettingsWidget>(new PerformanceSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void PerformanceSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PerformanceSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct StyleSettingsWidget::Private
        {
            std::shared_ptr<ui::ComboBox> paletteComboBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void StyleSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::StyleSettingsWidget", context, parent);
            TLRENDER_P();

            p.paletteComboBox = ui::ComboBox::create(getStylePaletteLabels(), context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Palette:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.paletteComboBox->setParent(p.layout);
            p.layout->setGridPos(p.paletteComboBox, 0, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            StylePalette value = StylePalette::First;
                            settings->getValue("Style/Palette", value);
                            p.paletteComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                    }
                });

            p.paletteComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        StylePalette stylePalette = static_cast<StylePalette>(value);
                        app->getStyle()->setColorRoles(getStylePalette(stylePalette));
                        app->getSettings()->setValue("Style/Palette", stylePalette);
                    }
                });
        }

        StyleSettingsWidget::StyleSettingsWidget() :
            _p(new Private)
        {}

        StyleSettingsWidget::~StyleSettingsWidget()
        {}

        std::shared_ptr<StyleSettingsWidget> StyleSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StyleSettingsWidget>(new StyleSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void StyleSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void StyleSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> toolTipsEnabledCheckBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void MiscSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::MiscSettingsWidget", context, parent);
            TLRENDER_P();

            p.toolTipsEnabledCheckBox = ui::CheckBox::create(context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Enable tool tips:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.toolTipsEnabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.toolTipsEnabledCheckBox, 1, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
            if (auto app = appWeak.lock())
            {
                auto settings = app->getSettings();
                {
                    bool value = false;
                    settings->getValue("Misc/ToolTipsEnabled", value);
                    p.toolTipsEnabledCheckBox->setChecked(value);
                }
            }
                });

            p.toolTipsEnabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Misc/ToolTipsEnabled", value);
                    }
                });
        }

        MiscSettingsWidget::MiscSettingsWidget() :
            _p(new Private)
        {}

        MiscSettingsWidget::~MiscSettingsWidget()
        {}

        std::shared_ptr<MiscSettingsWidget> MiscSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MiscSettingsWidget>(new MiscSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void MiscSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void MiscSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct SettingsTool::Private
        {
            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<ui::ToolButton> resetButton;
            std::shared_ptr<ui::VerticalLayout> layout;
        };

        void SettingsTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Settings,
                "tl::play_gl::SettingsTool",
                app,
                context,
                parent);
            TLRENDER_P();

            auto cacheWidget = CacheSettingsWidget::create(app, context);
            auto fileSequenceWidget = FileSequenceSettingsWidget::create(app, context);
            auto fileBrowserWidget = FileBrowserSettingsWidget::create(app, context);
            auto performanceWidget = PerformanceSettingsWidget::create(app, context);
            auto styleWidget = StyleSettingsWidget::create(app, context);
            auto miscWidget = MiscSettingsWidget::create(app, context);
            auto vLayout = ui::VerticalLayout::create(context);
            vLayout->setSpacingRole(ui::SizeRole::None);
            auto bellows = ui::Bellows::create("Cache", context, vLayout);
            bellows->setWidget(cacheWidget);
            bellows = ui::Bellows::create("File Sequences", context, vLayout);
            bellows->setWidget(fileSequenceWidget);
            bellows = ui::Bellows::create("File Browser", context, vLayout);
            bellows->setWidget(fileBrowserWidget);
            bellows = ui::Bellows::create("Performance", context, vLayout);
            bellows->setWidget(performanceWidget);
            bellows = ui::Bellows::create("Style", context, vLayout);
            bellows->setWidget(styleWidget);
            bellows = ui::Bellows::create("Miscellaneous", context, vLayout);
            bellows->setWidget(miscWidget);
            p.scrollWidget = ui::ScrollWidget::create(context);
            p.scrollWidget->setWidget(vLayout);
            p.scrollWidget->setVStretch(ui::Stretch::Expanding);

            p.resetButton = ui::ToolButton::create("Default Settings", context);

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.scrollWidget->setParent(p.layout);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.resetButton->setParent(hLayout);
            _setWidget(p.layout);

            std::weak_ptr<App> appWeak(app);
            p.resetButton->setClickedCallback(
                [this, appWeak]
                {
                    if (auto context = _context.lock())
                    {
                        if (auto eventLoop = getEventLoop().lock())
                        {
                            if (auto messageDialogSystem = context->getSystem<ui::MessageDialogSystem>())
                            {
                                messageDialogSystem->open(
                                    "Reset preferences to default values?",
                                    eventLoop,
                                    [appWeak](bool value)
                                    {
                                        if (value)
                                        {
                                            if (auto app = appWeak.lock())
                                            {
                                                app->getSettings()->reset();
                                            }
                                        }
                                    });
                            }
                        }
                    }
                });
        }

        SettingsTool::SettingsTool() :
            _p(new Private)
        {}

        SettingsTool::~SettingsTool()
        {}

        std::shared_ptr<SettingsTool> SettingsTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<SettingsTool>(new SettingsTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
