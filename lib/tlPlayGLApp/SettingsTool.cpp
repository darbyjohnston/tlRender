// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SettingsTool.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>

#include <tlUI/Bellows.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/DoubleEdit.h>
#include <tlUI/GridLayout.h>
#include <tlUI/IntEdit.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
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
            std::shared_ptr<ui::DoubleEdit> readAhead;
            std::shared_ptr<ui::DoubleEdit> readBehind;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
            std::shared_ptr<observer::ValueObserver<double> > readAheadObserver;
            std::shared_ptr<observer::ValueObserver<double> > readBehindObserver;
        };

        void CacheSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::CacheSettingsWidget", context, parent);
            TLRENDER_P();

            p.readAhead = ui::DoubleEdit::create(context);
            p.readAhead->getModel()->setRange(math::DoubleRange(0.0, 60.0));
            p.readAhead->getModel()->setStep(1.0);
            p.readAhead->getModel()->setLargeStep(10.0);

            p.readBehind = ui::DoubleEdit::create(context);
            p.readBehind->getModel()->setRange(math::DoubleRange(0.0, 60.0));
            p.readBehind->getModel()->setStep(1.0);
            p.readBehind->getModel()->setLargeStep(10.0);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Read ahead (seconds):", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.readAhead->setParent(p.layout);
            p.layout->setGridPos(p.readAhead, 0, 1);
            label = ui::Label::create("Read behind (seconds):", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.readBehind->setParent(p.layout);
            p.layout->setGridPos(p.readBehind, 1, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        p.readAhead->getModel()->setValue(
                            settings->getValue<double>("Cache/ReadAhead"));
                        p.readBehind->getModel()->setValue(
                            settings->getValue<double>("Cache/ReadBehind"));
                    }
                });

            p.readAheadObserver = observer::ValueObserver<double>::create(
                p.readAhead->getModel()->observeValue(),
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/ReadAhead", value);
                    }
                });

            p.readBehindObserver = observer::ValueObserver<double>::create(
                p.readBehind->getModel()->observeValue(),
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/ReadBehind", value);
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

        void CacheSettingsWidget::setGeometry(const math::BBox2i& value)
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

            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
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
            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        p.audioComboBox->setCurrentIndex(static_cast<int>(
                            settings->getValue<timeline::FileSequenceAudio>("FileSequence/Audio")));
                        p.audioFileNameEdit->setText(
                            settings->getValue("FileSequence/AudioFileName"));
                        p.audioDirectoryEdit->setText(
                            settings->getValue("FileSequence/AudioDirectory"));
                        p.maxDigitsEdit->getModel()->setValue(
                            settings->getValue<int>("FileSequence/MaxDigits"));
                    }
                });

            p.audioComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("FileSequence/Audio", value);
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

        void FileSequenceSettingsWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileSequenceSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
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

            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
            std::shared_ptr<observer::ValueObserver<int> > audioBufferFramesObserver;
            std::shared_ptr<observer::ValueObserver<int> > videoRequestObserver;
            std::shared_ptr<observer::ValueObserver<int> > audioRequestObserver;
            std::shared_ptr<observer::ValueObserver<int> > sequenceThreadsObserver;
            std::shared_ptr<observer::ValueObserver<int> > ffmpegThreadsObserver;
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
            p.audioBufferFramesEdit->setDigits(4);
            p.audioBufferFramesEdit->getModel()->setRange(math::IntRange(1024, 4096));
            p.audioBufferFramesEdit->getModel()->setStep(256);
            p.audioBufferFramesEdit->getModel()->setLargeStep(1024);

            p.videoRequestsEdit = ui::IntEdit::create(context);
            p.videoRequestsEdit->getModel()->setRange(math::IntRange(1, 64));

            p.audioRequestsEdit = ui::IntEdit::create(context);
            p.audioRequestsEdit->getModel()->setRange(math::IntRange(1, 64));

            p.sequenceThreadsEdit = ui::IntEdit::create(context);
            p.sequenceThreadsEdit->getModel()->setRange(math::IntRange(1, 64));

            p.ffmpegYUVtoRGBCheckBox = ui::CheckBox::create(context);

            p.ffmpegThreadsEdit = ui::IntEdit::create(context);
            p.ffmpegThreadsEdit->getModel()->setRange(math::IntRange(0, 64));

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
            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        p.timerComboBox->setCurrentIndex(static_cast<int>(
                            settings->getValue<timeline::TimerMode>("Performance/TimerMode")));
                        p.audioBufferFramesEdit->getModel()->setValue(
                            settings->getValue<int>("Performance/AudioBufferFrameCount"));
                        p.videoRequestsEdit->getModel()->setValue(
                            settings->getValue<int>("Performance/VideoRequestCount"));
                        p.audioRequestsEdit->getModel()->setValue(
                            settings->getValue<int>("Performance/AudioRequestCount"));
                        p.sequenceThreadsEdit->getModel()->setValue(
                            settings->getValue<int>("Performance/SequenceThreadCount"));
                        p.ffmpegYUVtoRGBCheckBox->setChecked(
                            settings->getValue<bool>("Performance/FFmpegYUVToRGBConversion"));
                        p.ffmpegThreadsEdit->getModel()->setValue(
                            settings->getValue<int>("Performance/FFmpegThreadCount"));
                    }
                });

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

            p.audioBufferFramesObserver = observer::ValueObserver<int>::create(
                p.audioBufferFramesEdit->getModel()->observeValue(),
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/AudioBufferFrameCount",
                            value);
                    }
                });

            p.videoRequestObserver = observer::ValueObserver<int>::create(
                p.videoRequestsEdit->getModel()->observeValue(),
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/VideoRequestCount",
                            value);
                    }
                });

            p.audioRequestObserver = observer::ValueObserver<int>::create(
                p.audioRequestsEdit->getModel()->observeValue(),
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/AudioRequestCount",
                            value);
                    }
                });

            p.sequenceThreadsObserver = observer::ValueObserver<int>::create(
                p.sequenceThreadsEdit->getModel()->observeValue(),
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

            p.ffmpegThreadsObserver = observer::ValueObserver<int>::create(
                p.ffmpegThreadsEdit->getModel()->observeValue(),
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/FFmpegThreadCount",
                            value);
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

        void PerformanceSettingsWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PerformanceSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> toolTipsEnabledCheckBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
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
            p.layout->setGridPos(label, 0, 0);
            p.toolTipsEnabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.toolTipsEnabledCheckBox, 0, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        p.toolTipsEnabledCheckBox->setChecked(
                            settings->getValue<bool>("Misc/ToolTipsEnabled"));
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

        void MiscSettingsWidget::setGeometry(const math::BBox2i& value)
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
            auto performanceWidget = PerformanceSettingsWidget::create(app, context);
            auto miscWidget = MiscSettingsWidget::create(app, context);
            auto vLayout = ui::VerticalLayout::create(context);
            vLayout->setSpacingRole(ui::SizeRole::None);
            auto bellows = ui::Bellows::create("Cache", context, vLayout);
            bellows->setWidget(cacheWidget);
            bellows = ui::Bellows::create("File Sequences", context, vLayout);
            bellows->setWidget(fileSequenceWidget);
            bellows = ui::Bellows::create("Performance", context, vLayout);
            bellows->setWidget(performanceWidget);
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
