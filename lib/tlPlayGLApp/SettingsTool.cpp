// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SettingsTool.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>

#include <tlUI/Bellows.h>
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

            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeData(),
                [this](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    auto i = value.find("Cache/ReadAhead");
                    if (i != value.end())
                    {
                        p.readAhead->getModel()->setValue(std::atof(i->second.c_str()));
                    }
                    i = value.find("Cache/ReadBehind");
                    if (i != value.end())
                    {
                        p.readBehind->getModel()->setValue(std::atof(i->second.c_str()));
                    }
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.readAheadObserver = observer::ValueObserver<double>::create(
                p.readAhead->getModel()->observeValue(),
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setData("Cache/ReadAhead", value);
                    }
                });

            p.readBehindObserver = observer::ValueObserver<double>::create(
                p.readBehind->getModel()->observeValue(),
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setData("Cache/ReadBehind", value);
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

            p.audioComboBox = ui::ComboBox::create(context);
            p.audioComboBox->setItems(timeline::getFileSequenceAudioLabels());

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

            p.settingsObserver = observer::MapObserver<std::string, std::string>::create(
                app->getSettings()->observeData(),
                [this](const std::map<std::string, std::string>& value)
                {
                    TLRENDER_P();
                    auto i = value.find("FileSequence/Audio");
                    if (i != value.end())
                    {
                        timeline::FileSequenceAudio value = timeline::FileSequenceAudio::None;
                        std::stringstream ss(i->second);
                        ss >> value;
                        p.audioComboBox->setCurrentIndex(static_cast<int>(value));
                    }
                    i = value.find("FileSequence/AudioFileName");
                    if (i != value.end())
                    {
                        p.audioFileNameEdit->setText(i->second);
                    }
                    i = value.find("FileSequence/AudioDirectory");
                    if (i != value.end())
                    {
                        p.audioDirectoryEdit->setText(i->second);
                    }
                    i = value.find("FileSequence/MaxDigits");
                    if (i != value.end())
                    {
                        p.maxDigitsEdit->getModel()->setValue(std::atoi(i->second.c_str()));
                    }
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.audioComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setData("FileSequence/Audio", value);
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
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
        };

        void PerformanceSettingsWidget::_init(
            const std::shared_ptr<App>&,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::PerformanceSettingsWidget", context, parent);
            TLRENDER_P();

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
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
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::MapObserver<std::string, std::string> > settingsObserver;
        };

        void MiscSettingsWidget::_init(
            const std::shared_ptr<App>&,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::MiscSettingsWidget", context, parent);
            TLRENDER_P();

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
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
