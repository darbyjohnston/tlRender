// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/SettingsToolPrivate.h>

#include <tlPlayApp/App.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <dtk/ui/Bellows.h>
#include <dtk/ui/CheckBox.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/DialogSystem.h>
#include <dtk/ui/Divider.h>
#include <dtk/ui/DoubleEdit.h>
#include <dtk/ui/FloatEditSlider.h>
#include <dtk/ui/FormLayout.h>
#include <dtk/ui/IntEdit.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/LineEdit.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>
#include <dtk/ui/ToolButton.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::IntEdit> sizeGB;
            std::shared_ptr<dtk::DoubleEdit> readAhead;
            std::shared_ptr<dtk::DoubleEdit> readBehind;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<CacheSettings> > settingsObserver;
        };

        void CacheSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::CacheSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.sizeGB = dtk::IntEdit::create(context);
            p.sizeGB->setRange(dtk::RangeI(0, 1024));

            p.readAhead = dtk::DoubleEdit::create(context);
            p.readAhead->setRange(dtk::RangeD(0.0, 60.0));
            p.readAhead->setStep(1.0);
            p.readAhead->setLargeStep(10.0);

            p.readBehind = dtk::DoubleEdit::create(context);
            p.readBehind->setRange(dtk::RangeD(0.0, 60.0));
            p.readBehind->setStep(1.0);
            p.readBehind->setLargeStep(10.0);

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Cache size (GB):", p.sizeGB);
            p.layout->addRow("Read ahead (seconds):", p.readAhead);
            p.layout->addRow("Read behind (seconds):", p.readBehind);

            p.settingsObserver = dtk::ValueObserver<CacheSettings>::create(
                p.model->observeCache(),
                [this](const CacheSettings& value)
                {
                    DTK_P();
                    p.sizeGB->setValue(value.sizeGB);
                    p.readAhead->setValue(value.readAhead);
                    p.readBehind->setValue(value.readBehind);
                });

            p.sizeGB->setCallback(
                [this](int value)
                {
                    DTK_P();
                    CacheSettings settings = p.model->getCache();
                    settings.sizeGB = value;
                    p.model->setCache(settings);
                });

            p.readAhead->setCallback(
                [this](double value)
                {
                    DTK_P();
                    CacheSettings settings = p.model->getCache();
                    settings.readAhead = value;
                    p.model->setCache(settings);
                });

            p.readBehind->setCallback(
                [this](double value)
                {
                    DTK_P();
                    CacheSettings settings = p.model->getCache();
                    settings.readBehind = value;
                    p.model->setCache(settings);
                });
        }

        CacheSettingsWidget::CacheSettingsWidget() :
            _p(new Private)
        {}

        CacheSettingsWidget::~CacheSettingsWidget()
        {}

        std::shared_ptr<CacheSettingsWidget> CacheSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CacheSettingsWidget>(new CacheSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void CacheSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CacheSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct FileBrowserSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::CheckBox> nfdCheckBox;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<FileBrowserSettings> > settingsObserver;
        };

        void FileBrowserSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::FileBrowserSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.nfdCheckBox = dtk::CheckBox::create(context);
            p.nfdCheckBox->setHStretch(dtk::Stretch::Expanding);

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Native file dialog:", p.nfdCheckBox);

            p.settingsObserver = dtk::ValueObserver<FileBrowserSettings>::create(
                p.model->observeFileBrowser(),
                [this](const FileBrowserSettings& value)
                {
                    DTK_P();
                    p.nfdCheckBox->setChecked(value.nativeFileDialog);
                });

            p.nfdCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    auto settings = p.model->getFileBrowser();
                    settings.nativeFileDialog = value;
                    p.model->setFileBrowser(settings);
                });
        }

        FileBrowserSettingsWidget::FileBrowserSettingsWidget() :
            _p(new Private)
        {}

        FileBrowserSettingsWidget::~FileBrowserSettingsWidget()
        {}

        std::shared_ptr<FileBrowserSettingsWidget> FileBrowserSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowserSettingsWidget>(new FileBrowserSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void FileBrowserSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileBrowserSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::ComboBox> audioComboBox;
            std::shared_ptr<dtk::LineEdit> audioFileNameEdit;
            std::shared_ptr<dtk::LineEdit> audioDirectoryEdit;
            std::shared_ptr<dtk::IntEdit> maxDigitsEdit;
            std::shared_ptr<dtk::DoubleEdit> defaultSpeedEdit;
            std::shared_ptr<dtk::IntEdit> threadsEdit;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<FileSequenceSettings> > settingsObserver;
        };

        void FileSequenceSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::FileSequenceSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.audioComboBox = dtk::ComboBox::create(context, timeline::getFileSequenceAudioLabels());
            p.audioComboBox->setHStretch(dtk::Stretch::Expanding);

            p.audioFileNameEdit = dtk::LineEdit::create(context);
            p.audioFileNameEdit->setHStretch(dtk::Stretch::Expanding);

            p.audioDirectoryEdit = dtk::LineEdit::create(context);
            p.audioDirectoryEdit->setHStretch(dtk::Stretch::Expanding);

            p.maxDigitsEdit = dtk::IntEdit::create(context);

            p.defaultSpeedEdit = dtk::DoubleEdit::create(context);
            p.defaultSpeedEdit->setRange(dtk::RangeD(1.0, 120.0));

            p.threadsEdit = dtk::IntEdit::create(context);
            p.threadsEdit->setRange(dtk::RangeI(1, 64));

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Audio:", p.audioComboBox);
            p.layout->addRow("Audio file name:", p.audioFileNameEdit);
            p.layout->addRow("Audio directory:", p.audioDirectoryEdit);
            p.layout->addRow("Maximum digits:", p.maxDigitsEdit);
            p.layout->addRow("Default speed (FPS):", p.defaultSpeedEdit);
            p.layout->addRow("I/O threads:", p.threadsEdit);

            p.settingsObserver = dtk::ValueObserver<FileSequenceSettings>::create(
                p.model->observeFileSequence(),
                [this](const FileSequenceSettings& value)
                {
                    DTK_P();
                    p.audioComboBox->setCurrentIndex(static_cast<int>(value.audio));
                    p.audioFileNameEdit->setText(value.audioFileName);
                    p.audioDirectoryEdit->setText(value.audioDirectory);
                    p.maxDigitsEdit->setValue(value.maxDigits);
                    p.defaultSpeedEdit->setValue(value.io.defaultSpeed);
                    p.threadsEdit->setValue(value.io.threadCount);
                });

            p.audioComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    FileSequenceSettings settings = p.model->getFileSequence();
                    settings.audio = static_cast<timeline::FileSequenceAudio>(value);
                    p.model->setFileSequence(settings);
                });

            p.audioFileNameEdit->setTextCallback(
                [this](const std::string& value)
                {
                    DTK_P();
                    FileSequenceSettings settings = p.model->getFileSequence();
                    settings.audioFileName = value;
                    p.model->setFileSequence(settings);
                });

            p.audioDirectoryEdit->setTextCallback(
                [this](const std::string& value)
                {
                    DTK_P();
                    FileSequenceSettings settings = p.model->getFileSequence();
                    settings.audioDirectory = value;
                    p.model->setFileSequence(settings);
                });

            p.maxDigitsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    FileSequenceSettings settings = p.model->getFileSequence();
                    settings.maxDigits = value;
                    p.model->setFileSequence(settings);
                });

            p.defaultSpeedEdit->setCallback(
                [this](double value)
                {
                    DTK_P();
                    FileSequenceSettings settings = p.model->getFileSequence();
                    settings.io.defaultSpeed = value;
                    p.model->setFileSequence(settings);
                });

            p.threadsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    FileSequenceSettings settings = p.model->getFileSequence();
                    settings.io.threadCount = value;
                    p.model->setFileSequence(settings);
                });
        }

        FileSequenceSettingsWidget::FileSequenceSettingsWidget() :
            _p(new Private)
        {}

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget()
        {}

        std::shared_ptr<FileSequenceSettingsWidget> FileSequenceSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileSequenceSettingsWidget>(new FileSequenceSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void FileSequenceSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileSequenceSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::CheckBox> tooltipsCheckBox;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<MiscSettings> > settingsObserver;
        };

        void MiscSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::MiscSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.tooltipsCheckBox = dtk::CheckBox::create(context);
            p.tooltipsCheckBox->setHStretch(dtk::Stretch::Expanding);

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Enable tooltips:", p.tooltipsCheckBox);

            p.settingsObserver = dtk::ValueObserver<MiscSettings>::create(
                p.model->observeMisc(),
                [this](const MiscSettings& value)
                {
                    DTK_P();
                    p.tooltipsCheckBox->setChecked(value.tooltipsEnabled);
                });

            p.tooltipsCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    auto settings = p.model->getMisc();
                    settings.tooltipsEnabled = value;
                    p.model->setMisc(settings);
                });
        }

        MiscSettingsWidget::MiscSettingsWidget() :
            _p(new Private)
        {}

        MiscSettingsWidget::~MiscSettingsWidget()
        {}

        std::shared_ptr<MiscSettingsWidget> MiscSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MiscSettingsWidget>(new MiscSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void MiscSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void MiscSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct MouseSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;
            std::vector<std::string> actionLabels;
            std::vector<dtk::KeyModifier> modifiers;
            std::vector<std::string> modifierLabels;

            std::map<MouseAction, std::shared_ptr<dtk::ComboBox> > modifierComboBoxes;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<MouseSettings> > settingsObserver;
        };

        void MouseSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::MouseSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.actionLabels.push_back("Pan view");
            p.actionLabels.push_back("Compare wipe");
            p.actionLabels.push_back("Color picker");
            p.actionLabels.push_back("Frame shuttle");

            p.modifiers.push_back(dtk::KeyModifier::None);
            p.modifiers.push_back(dtk::KeyModifier::Shift);
            p.modifiers.push_back(dtk::KeyModifier::Control);
            p.modifiers.push_back(dtk::KeyModifier::Alt);
            p.modifiers.push_back(dtk::KeyModifier::Super);
            p.modifierLabels.push_back("Click");
            p.modifierLabels.push_back(dtk::to_string(dtk::KeyModifier::Shift) + " + click");
            p.modifierLabels.push_back(dtk::to_string(dtk::KeyModifier::Control) + " + click");
            p.modifierLabels.push_back(dtk::to_string(dtk::KeyModifier::Alt) + " + click");
            p.modifierLabels.push_back(dtk::to_string(dtk::KeyModifier::Super) + " + click");

            for (const auto mouseAction : getMouseActionEnums())
            {
                p.modifierComboBoxes[mouseAction] = dtk::ComboBox::create(context, p.modifierLabels);
                p.modifierComboBoxes[mouseAction]->setHStretch(dtk::Stretch::Expanding);
            }

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            for (const auto mouseAction : getMouseActionEnums())
            {
                p.modifierComboBoxes[mouseAction]->setParent(p.layout);
                p.layout->addRow(dtk::Format("{0}:").arg(p.actionLabels[static_cast<size_t>(mouseAction)]), p.modifierComboBoxes[mouseAction]);
            }

            p.settingsObserver = dtk::ValueObserver<MouseSettings>::create(
                p.model->observeMouse(),
                [this](const MouseSettings& value)
                {
                    DTK_P();
                    for (const auto& i : value.actions)
                    {
                        const auto j = p.modifierComboBoxes.find(i.first);
                        if (j != p.modifierComboBoxes.end())
                        {
                            const auto k = std::find(p.modifiers.begin(), p.modifiers.end(), i.second);
                            if (k != p.modifiers.end())
                            {

                                j->second->setCurrentIndex(k - p.modifiers.begin());
                            }
                        }
                    }
                });

            for (const auto mouseAction : getMouseActionEnums())
            {
                p.modifierComboBoxes[mouseAction]->setIndexCallback(
                    [this, mouseAction](int index)
                    {
                        DTK_P();
                        if (index >= 0 && index < p.modifiers.size())
                        {
                            auto settings = p.model->getMouse();
                            settings.actions[mouseAction] = p.modifiers[index];
                            p.model->setMouse(settings);
                        }
                    });
            }
        }

        MouseSettingsWidget::MouseSettingsWidget() :
            _p(new Private)
        {}

        MouseSettingsWidget::~MouseSettingsWidget()
        {}

        std::shared_ptr<MouseSettingsWidget> MouseSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MouseSettingsWidget>(new MouseSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void MouseSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void MouseSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::IntEdit> audioBufferFramesEdit;
            std::shared_ptr<dtk::IntEdit> videoRequestsEdit;
            std::shared_ptr<dtk::IntEdit> audioRequestsEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<PerformanceSettings> > settingsObserver;
        };

        void PerformanceSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::PerformanceSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.audioBufferFramesEdit = dtk::IntEdit::create(context);
            p.audioBufferFramesEdit->setRange(dtk::RangeI(1, 1000000));
            p.audioBufferFramesEdit->setStep(256);
            p.audioBufferFramesEdit->setLargeStep(1024);

            p.videoRequestsEdit = dtk::IntEdit::create(context);
            p.videoRequestsEdit->setRange(dtk::RangeI(1, 64));

            p.audioRequestsEdit = dtk::IntEdit::create(context);
            p.audioRequestsEdit->setRange(dtk::RangeI(1, 64));

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Changes are applied to new files.", p.layout);
            auto formLayout = dtk::FormLayout::create(context, p.layout);
            formLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            formLayout->addRow("Audio buffer frames:", p.audioBufferFramesEdit);
            formLayout->addRow("Video requests:", p.videoRequestsEdit);
            formLayout->addRow("Audio requests:", p.audioRequestsEdit);

            p.settingsObserver = dtk::ValueObserver<PerformanceSettings>::create(
                p.model->observePerformance(),
                [this](const PerformanceSettings& value)
                {
                    DTK_P();
                    p.audioBufferFramesEdit->setValue(value.audioBufferFrameCount);
                    p.videoRequestsEdit->setValue(value.videoRequestCount);
                    p.audioRequestsEdit->setValue(value.audioRequestCount);
                });

            p.audioBufferFramesEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto settings = p.model->getPerformance();
                    settings.audioBufferFrameCount = value;
                    p.model->setPerformance(settings);
                });

            p.videoRequestsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto settings = p.model->getPerformance();
                    settings.videoRequestCount = value;
                    p.model->setPerformance(settings);
                });

            p.audioRequestsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto settings = p.model->getPerformance();
                    settings.audioRequestCount = value;
                    p.model->setPerformance(settings);
                });
        }

        PerformanceSettingsWidget::PerformanceSettingsWidget() :
            _p(new Private)
        {}

        PerformanceSettingsWidget::~PerformanceSettingsWidget()
        {}

        std::shared_ptr<PerformanceSettingsWidget> PerformanceSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PerformanceSettingsWidget>(new PerformanceSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void PerformanceSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PerformanceSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct StyleSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            const std::vector<float> displayScales =
            {
                0.F,
                1.F,
                1.5F,
                2.F,
                2.5F,
                3.F,
                3.5F,
                4.F
            };

            std::shared_ptr<dtk::ComboBox> colorStyleComboBox;
            std::shared_ptr<dtk::ComboBox> displayScaleComboBox;
            std::shared_ptr<dtk::FormLayout> layout;

            std::shared_ptr<dtk::ValueObserver<StyleSettings> > settingsObserver;
        };

        void StyleSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::StyleSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.colorStyleComboBox = dtk::ComboBox::create(context, dtk::getColorStyleLabels());
            p.colorStyleComboBox->setHStretch(dtk::Stretch::Expanding);

            std::vector<std::string> labels;
            for (auto d : p.displayScales)
            {
                labels.push_back(0.F == d ?
                    std::string("Automatic") :
                    dtk::Format("{0}").arg(d).operator std::string());
            }
            p.displayScaleComboBox = dtk::ComboBox::create(context, labels);            
            p.displayScaleComboBox->setHStretch(dtk::Stretch::Expanding);

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Color style:", p.colorStyleComboBox);
            p.layout->addRow("Display scale:", p.displayScaleComboBox);

            p.settingsObserver = dtk::ValueObserver<StyleSettings>::create(
                app->getSettingsModel()->observeStyle(),
                [this](const StyleSettings& value)
                {
                    DTK_P();
                    p.colorStyleComboBox->setCurrentIndex(static_cast<int>(value.colorStyle));
                    const auto i = std::find(
                        p.displayScales.begin(),
                        p.displayScales.end(),
                        value.displayScale);
                    p.displayScaleComboBox->setCurrentIndex(
                        i != p.displayScales.end() ?
                        (i - p.displayScales.begin()) :
                        -1);
                });

            p.colorStyleComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    auto settings = p.model->getStyle();
                    settings.colorStyle = static_cast<dtk::ColorStyle>(value);
                    p.model->setStyle(settings);
                });

            p.displayScaleComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    auto settings = p.model->getStyle();
                    if (value >= 0 && value < p.displayScales.size())
                    {
                        settings.displayScale = p.displayScales[value];
                    }
                    p.model->setStyle(settings);
                });
        }

        StyleSettingsWidget::StyleSettingsWidget() :
            _p(new Private)
        {}

        StyleSettingsWidget::~StyleSettingsWidget()
        {}

        std::shared_ptr<StyleSettingsWidget> StyleSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StyleSettingsWidget>(new StyleSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void StyleSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void StyleSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

#if defined(TLRENDER_FFMPEG)
        struct FFmpegSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::CheckBox> yuvToRGBCheckBox;
            std::shared_ptr<dtk::IntEdit> threadsEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<ffmpeg::Options> > optionsObserver;
        };

        void FFmpegSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::FFmpegSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.yuvToRGBCheckBox = dtk::CheckBox::create(context);
            p.yuvToRGBCheckBox->setHStretch(dtk::Stretch::Expanding);

            p.threadsEdit = dtk::IntEdit::create(context);
            p.threadsEdit->setRange(dtk::RangeI(0, 64));

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Changes are applied to new files.", p.layout);
            auto formLayout = dtk::FormLayout::create(context, p.layout);
            formLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            formLayout->addRow("YUV to RGB conversion:", p.yuvToRGBCheckBox);
            formLayout->addRow("I/O threads:", p.threadsEdit);

            p.optionsObserver = dtk::ValueObserver<ffmpeg::Options>::create(
                p.model->observeFFmpeg(),
                [this](const ffmpeg::Options& value)
                {
                    DTK_P();
                    p.yuvToRGBCheckBox->setChecked(value.yuvToRgb);
                    p.threadsEdit->setValue(value.threadCount);
                });

            p.yuvToRGBCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    ffmpeg::Options options = p.model->getFFmpeg();
                    options.yuvToRgb = value;
                    p.model->setFFmpeg(options);
                });

            p.threadsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    ffmpeg::Options options = p.model->getFFmpeg();
                    options.threadCount = value;
                    p.model->setFFmpeg(options);
                });
        }

        FFmpegSettingsWidget::FFmpegSettingsWidget() :
            _p(new Private)
        {}

        FFmpegSettingsWidget::~FFmpegSettingsWidget()
        {}

        std::shared_ptr<FFmpegSettingsWidget> FFmpegSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FFmpegSettingsWidget>(new FFmpegSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void FFmpegSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FFmpegSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        struct USDSettingsWidget::Private
        {
            std::shared_ptr<SettingsModel> model;

            std::shared_ptr<dtk::IntEdit> renderWidthEdit;
            std::shared_ptr<dtk::FloatEditSlider> complexitySlider;
            std::shared_ptr<dtk::ComboBox> drawModeComboBox;
            std::shared_ptr<dtk::CheckBox> lightingCheckBox;
            std::shared_ptr<dtk::CheckBox> sRGBCheckBox;
            std::shared_ptr<dtk::IntEdit> stageCacheEdit;
            std::shared_ptr<dtk::IntEdit> diskCacheEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<usd::Options> > optionsObserver;
        };

        void USDSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::USDSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.renderWidthEdit = dtk::IntEdit::create(context);
            p.renderWidthEdit->setRange(dtk::RangeI(1, 8192));

            p.complexitySlider = dtk::FloatEditSlider::create(context);

            p.drawModeComboBox = dtk::ComboBox::create(context, usd::getDrawModeLabels());
            p.drawModeComboBox->setHStretch(dtk::Stretch::Expanding);

            p.lightingCheckBox = dtk::CheckBox::create(context);
            p.lightingCheckBox->setHStretch(dtk::Stretch::Expanding);

            p.sRGBCheckBox = dtk::CheckBox::create(context);
            p.sRGBCheckBox->setHStretch(dtk::Stretch::Expanding);

            p.stageCacheEdit = dtk::IntEdit::create(context);
            p.stageCacheEdit->setRange(dtk::RangeI(0, 10));

            p.diskCacheEdit = dtk::IntEdit::create(context);
            p.diskCacheEdit->setRange(dtk::RangeI(0, 1024));

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Changes are applied to new files.", p.layout);
            auto formLayout = dtk::FormLayout::create(context, p.layout);
            formLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            formLayout->addRow("Render width:", p.renderWidthEdit);
            formLayout->addRow("Render complexity:", p.complexitySlider);
            formLayout->addRow("Draw mode:", p.drawModeComboBox);
            formLayout->addRow("Enable lighting:", p.lightingCheckBox);
            formLayout->addRow("Enable sRGB color space:", p.sRGBCheckBox);
            formLayout->addRow("Stage cache size:", p.stageCacheEdit);
            formLayout->addRow("Disk cache size (GB):", p.diskCacheEdit);

            p.optionsObserver = dtk::ValueObserver<usd::Options>::create(
                p.model->observeUSD(),
                [this](const usd::Options& value)
                {
                    DTK_P();
                    p.renderWidthEdit->setValue(value.renderWidth);
                    p.complexitySlider->setValue(value.complexity);
                    p.drawModeComboBox->setCurrentIndex(static_cast<int>(value.drawMode));
                    p.lightingCheckBox->setChecked(value.enableLighting);
                    p.sRGBCheckBox->setChecked(value.sRGB);
                    p.stageCacheEdit->setValue(value.stageCache);
                    p.diskCacheEdit->setValue(value.diskCache);
                });

            p.renderWidthEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.renderWidth = value;
                    p.model->setUSD(options);
                });

            p.complexitySlider->setCallback(
                [this](float value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.complexity = value;
                    p.model->setUSD(options);
                });

            p.drawModeComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.drawMode = static_cast<usd::DrawMode>(value);
                    p.model->setUSD(options);
                });

            p.lightingCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.enableLighting = value;
                    p.model->setUSD(options);
                });

            p.sRGBCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.sRGB = value;
                    p.model->setUSD(options);
                });

            p.stageCacheEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.stageCache = value;
                    p.model->setUSD(options);
                });

            p.diskCacheEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options options = p.model->getUSD();
                    options.diskCache = value;
                    p.model->setUSD(options);
                });
        }

        USDSettingsWidget::USDSettingsWidget() :
            _p(new Private)
        {}

        USDSettingsWidget::~USDSettingsWidget()
        {}

        std::shared_ptr<USDSettingsWidget> USDSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<USDSettingsWidget>(new USDSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void USDSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void USDSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }
#endif // TLRENDER_USD

        struct SettingsTool::Private
        {
            std::shared_ptr<dtk::ScrollWidget> scrollWidget;
            std::shared_ptr<dtk::ToolButton> resetButton;
            std::map<std::string, std::shared_ptr<dtk::Bellows> > bellows;
            std::shared_ptr<dtk::VerticalLayout> layout;
        };

        void SettingsTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Settings,
                "tl::play_app::SettingsTool",
                parent);
            DTK_P();

            auto cacheWidget = CacheSettingsWidget::create(context, app);
            auto fileBrowserWidget = FileBrowserSettingsWidget::create(context, app);
            auto fileSequenceWidget = FileSequenceSettingsWidget::create(context, app);
            auto keyShortcutsWidget = KeyShortcutsSettingsWidget::create(context, app);
            auto miscWidget = MiscSettingsWidget::create(context, app);
            auto mouseWidget = MouseSettingsWidget::create(context, app);
            auto performanceWidget = PerformanceSettingsWidget::create(context, app);
            auto styleWidget = StyleSettingsWidget::create(context, app);
#if defined(TLRENDER_FFMPEG)
            auto ffmpegWidget = FFmpegSettingsWidget::create(context, app);
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            auto usdWidget = USDSettingsWidget::create(context, app);
#endif // TLRENDER_USD

            auto vLayout = dtk::VerticalLayout::create(context);
            vLayout->setSpacingRole(dtk::SizeRole::None);
            p.bellows["Cache"] = dtk::Bellows::create(context, "Cache", vLayout);
            p.bellows["Cache"]->setWidget(cacheWidget);
            p.bellows["FileBrowser"] = dtk::Bellows::create(context, "File Browser", vLayout);
            p.bellows["FileBrowser"]->setWidget(fileBrowserWidget);
            p.bellows["FileSequences"] = dtk::Bellows::create(context, "File Sequences", vLayout);
            p.bellows["FileSequences"]->setWidget(fileSequenceWidget);
            p.bellows["KeyShortcuts"] = dtk::Bellows::create(context, "Keyboard Shortcuts", vLayout);
            p.bellows["KeyShortcuts"]->setWidget(keyShortcutsWidget);
            p.bellows["Misc"] = dtk::Bellows::create(context, "Miscellaneous", vLayout);
            p.bellows["Misc"]->setWidget(miscWidget);
            p.bellows["Mouse"] = dtk::Bellows::create(context, "Mouse", vLayout);
            p.bellows["Mouse"]->setWidget(mouseWidget);
            p.bellows["Performance"] = dtk::Bellows::create(context, "Performance", vLayout);
            p.bellows["Performance"]->setWidget(performanceWidget);
            p.bellows["Style"] = dtk::Bellows::create(context, "Style", vLayout);
            p.bellows["Style"]->setWidget(styleWidget);
#if defined(TLRENDER_FFMPEG)
            p.bellows["FFmpeg"] = dtk::Bellows::create(context, "FFmpeg", vLayout);
            p.bellows["FFmpeg"]->setWidget(ffmpegWidget);
#endif // TLRENDER_USD
#if defined(TLRENDER_USD)
            p.bellows["USD"] = dtk::Bellows::create(context, "USD", vLayout);
            p.bellows["USD"]->setWidget(usdWidget);
#endif // TLRENDER_USD

            p.scrollWidget = dtk::ScrollWidget::create(context);
            p.scrollWidget->setWidget(vLayout);
            p.scrollWidget->setBorder(false);
            p.scrollWidget->setVStretch(dtk::Stretch::Expanding);

            p.resetButton = dtk::ToolButton::create(context, "Default Settings");

            p.layout = dtk::VerticalLayout::create(context);
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.scrollWidget->setParent(p.layout);
            dtk::Divider::create(context, dtk::Orientation::Vertical, p.layout);
            auto hLayout = dtk::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(dtk::SizeRole::MarginInside);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.resetButton->setParent(hLayout);
            _setWidget(p.layout);

            _loadSettings(p.bellows);

            std::weak_ptr<App> appWeak(app);
            p.resetButton->setClickedCallback(
                [this, appWeak]
                {
                    if (auto context = getContext())
                    {
                        if (auto dialogSystem = context->getSystem<dtk::DialogSystem>())
                        {
                            dialogSystem->confirm(
                                "Reset Settings",
                                "Reset settings to default values?",
                                getWindow(),
                                [appWeak](bool value)
                                {
                                    if (value)
                                    {
                                        if (auto app = appWeak.lock())
                                        {
                                            app->getSettingsModel()->reset();
                                        }
                                    }
                                });
                        }
                    }
                });
        }

        SettingsTool::SettingsTool() :
            _p(new Private)
        {}

        SettingsTool::~SettingsTool()
        {
            _saveSettings(_p->bellows);
        }

        std::shared_ptr<SettingsTool> SettingsTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SettingsTool>(new SettingsTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
