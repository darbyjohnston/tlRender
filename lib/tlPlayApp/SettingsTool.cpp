// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SettingsToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlPlay/Settings.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <dtk/ui/Bellows.h>
#include <dtk/ui/CheckBox.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/DialogSystem.h>
#include <dtk/ui/DoubleEdit.h>
#include <dtk/ui/FloatEditSlider.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/IntEdit.h>
#include <dtk/ui/IntEdit.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/LineEdit.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::IntEdit> cacheSize;
            std::shared_ptr<dtk::DoubleEdit> readAhead;
            std::shared_ptr<dtk::DoubleEdit> readBehind;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void CacheSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::CacheSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.cacheSize = dtk::IntEdit::create(context);
            p.cacheSize->setRange(dtk::RangeI(0, 1024));

            p.readAhead = dtk::DoubleEdit::create(context);
            p.readAhead->setRange(dtk::RangeD(0.0, 60.0));
            p.readAhead->setStep(1.0);
            p.readAhead->setLargeStep(10.0);

            p.readBehind = dtk::DoubleEdit::create(context);
            p.readBehind->setRange(dtk::RangeD(0.0, 60.0));
            p.readBehind->setStep(1.0);
            p.readBehind->setLargeStep(10.0);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Cache size (GB):", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.cacheSize->setParent(p.layout);
            p.layout->setGridPos(p.cacheSize, 0, 1);
            label = dtk::Label::create(context, "Read ahead (seconds):", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.readAhead->setParent(p.layout);
            p.layout->setGridPos(p.readAhead, 1, 1);
            label = dtk::Label::create(context, "Read behind (seconds):", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.readBehind->setParent(p.layout);
            p.layout->setGridPos(p.readBehind, 2, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.cacheSize->setCallback(
                [this](double value)
                {
                    _p->settings->setValue("Cache/Size", value);
                });

            p.readAhead->setCallback(
                [this](double value)
                {
                    _p->settings->setValue("Cache/ReadAhead", value);
                });

            p.readBehind->setCallback(
                [this](double value)
                {
                    _p->settings->setValue("Cache/ReadBehind", value);
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

        void CacheSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("Cache/Size" == name || name.empty())
            {
                p.cacheSize->setValue(
                    p.settings->getValue<int>("Cache/Size"));
            }
            if ("Cache/ReadAhead" == name || name.empty())
            {
                p.readAhead->setValue(
                    p.settings->getValue<double>("Cache/ReadAhead"));
            }
            if ("Cache/ReadBehind" == name || name.empty())
            {
                p.readBehind->setValue(
                    p.settings->getValue<double>("Cache/ReadBehind"));
            }
        }

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::ComboBox> audioComboBox;
            std::shared_ptr<dtk::LineEdit> audioFileNameEdit;
            std::shared_ptr<dtk::LineEdit> audioDirectoryEdit;
            std::shared_ptr<dtk::IntEdit> maxDigitsEdit;
            std::shared_ptr<dtk::DoubleEdit> defaultSpeedEdit;
            std::shared_ptr<dtk::IntEdit> threadsEdit;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void FileSequenceSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::FileSequenceSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

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

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Audio:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.audioComboBox->setParent(p.layout);
            p.layout->setGridPos(p.audioComboBox, 0, 1);
            label = dtk::Label::create(context, "Audio file name:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.audioFileNameEdit->setParent(p.layout);
            p.layout->setGridPos(p.audioFileNameEdit, 1, 1);
            label = dtk::Label::create(context, "Audio directory:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.audioDirectoryEdit->setParent(p.layout);
            p.layout->setGridPos(p.audioDirectoryEdit, 2, 1);
            label = dtk::Label::create(context, "Maximum digits:", p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.maxDigitsEdit->setParent(p.layout);
            p.layout->setGridPos(p.maxDigitsEdit, 3, 1);
            label = dtk::Label::create(context, "Default FPS:", p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.defaultSpeedEdit->setParent(p.layout);
            p.layout->setGridPos(p.defaultSpeedEdit, 4, 1);
            label = dtk::Label::create(context, "I/O threads:", p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.threadsEdit->setParent(p.layout);
            p.layout->setGridPos(p.threadsEdit, 5, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.audioComboBox->setIndexCallback(
                [this](int value)
                {
                    _p->settings->setValue(
                        "FileSequence/Audio",
                        static_cast<timeline::FileSequenceAudio>(value));
                });

            p.audioFileNameEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->settings->setValue("FileSequence/AudioFileName", value);
                });

            p.audioDirectoryEdit->setTextCallback(
                [this](const std::string& value)
                {
                    _p->settings->setValue("FileSequence/AudioDirectory", value);
                });

            p.maxDigitsEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("FileSequence/MaxDigits", value);
                });

            p.defaultSpeedEdit->setCallback(
                [this](double value)
                {
                    _p->settings->setValue("SequenceIO/DefaultSpeed", value);
                });

            p.threadsEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("SequenceIO/ThreadCount", value);
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

        void FileSequenceSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("FileSequence/Audio" == name || name.empty())
            {
                p.audioComboBox->setCurrentIndex(static_cast<int>(
                    p.settings->getValue<timeline::FileSequenceAudio>("FileSequence/Audio")));
            }
            if ("FileSequence/AudioFileName" == name || name.empty())
            {
                p.audioFileNameEdit->setText(
                    p.settings->getValue<std::string>("FileSequence/AudioFileName"));
            }
            if ("FileSequence/AudioDirectory" == name || name.empty())
            {
                p.audioDirectoryEdit->setText(
                    p.settings->getValue<std::string>("FileSequence/AudioDirectory"));
            }
            if ("FileSequence/MaxDigits" == name || name.empty())
            {
                p.maxDigitsEdit->setValue(
                    p.settings->getValue<size_t>("FileSequence/MaxDigits"));
            }
            if ("SequenceIO/DefaultSpeed" == name || name.empty())
            {
                p.defaultSpeedEdit->setValue(
                    p.settings->getValue<double>("SequenceIO/DefaultSpeed"));
            }
            if ("SequenceIO/ThreadCount" == name || name.empty())
            {
                p.threadsEdit->setValue(
                    p.settings->getValue<size_t>("SequenceIO/ThreadCount"));
            }
        }

#if defined(TLRENDER_FFMPEG)
        struct FFmpegSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::CheckBox> yuvToRGBCheckBox;
            std::shared_ptr<dtk::IntEdit> threadsEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void FFmpegSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::FFmpegSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.yuvToRGBCheckBox = dtk::CheckBox::create(context);

            p.threadsEdit = dtk::IntEdit::create(context);
            p.threadsEdit->setRange(dtk::RangeI(0, 64));

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Changes are applied to new files.", p.layout);
            auto gridLayout = dtk::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            label = dtk::Label::create(context, "YUV to RGB conversion:", gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.yuvToRGBCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p.yuvToRGBCheckBox, 0, 1);
            label = dtk::Label::create(context, "I/O threads:", gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.threadsEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.threadsEdit, 1, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.yuvToRGBCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->settings->setValue("FFmpeg/YUVToRGBConversion", value);
                });

            p.threadsEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("FFmpeg/ThreadCount", value);
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

        void FFmpegSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("FFmpeg/YUVToRGBConversion" == name || name.empty())
            {
                p.yuvToRGBCheckBox->setChecked(
                    p.settings->getValue<bool>("FFmpeg/YUVToRGBConversion"));
            }
            if ("FFmpeg/ThreadCount" == name || name.empty())
            {
                p.threadsEdit->setValue(
                    p.settings->getValue<size_t>("FFmpeg/ThreadCount"));
            }
        }

#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        struct USDSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::IntEdit> renderWidthEdit;
            std::shared_ptr<dtk::FloatEditSlider> complexitySlider;
            std::shared_ptr<dtk::ComboBox> drawModeComboBox;
            std::shared_ptr<dtk::CheckBox> lightingCheckBox;
            std::shared_ptr<dtk::CheckBox> sRGBCheckBox;
            std::shared_ptr<dtk::IntEdit> stageCacheEdit;
            std::shared_ptr<dtk::IntEdit> diskCacheEdit;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void USDSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::USDSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.renderWidthEdit = dtk::IntEdit::create(context);
            p.renderWidthEdit->setRange(dtk::RangeI(1, 8192));

            p.complexitySlider = dtk::FloatEditSlider::create(context);

            p.drawModeComboBox = dtk::ComboBox::create(context, usd::getDrawModeLabels());
            p.drawModeComboBox->setHStretch(dtk::Stretch::Expanding);

            p.lightingCheckBox = dtk::CheckBox::create(context);

            p.sRGBCheckBox = dtk::CheckBox::create(context);

            p.stageCacheEdit = dtk::IntEdit::create(context);
            p.stageCacheEdit->setRange(dtk::RangeI(0, 10));

            p.diskCacheEdit = dtk::IntEdit::create(context);
            p.diskCacheEdit->setRange(dtk::RangeI(0, 1024));

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Render width:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.renderWidthEdit->setParent(p.layout);
            p.layout->setGridPos(p.renderWidthEdit, 0, 1);
            label = dtk::Label::create(context, "Render complexity:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.complexitySlider->setParent(p.layout);
            p.layout->setGridPos(p.complexitySlider, 1, 1);
            label = dtk::Label::create(context, "Draw mode:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.drawModeComboBox->setParent(p.layout);
            p.layout->setGridPos(p.drawModeComboBox, 2, 1);
            label = dtk::Label::create(context, "Enable lighting:", p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.lightingCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.lightingCheckBox, 3, 1);
            label = dtk::Label::create(context, "Enable sRGB color space:", p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sRGBCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.sRGBCheckBox, 4, 1);
            label = dtk::Label::create(context, "Stage cache size:", p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.stageCacheEdit->setParent(p.layout);
            p.layout->setGridPos(p.stageCacheEdit, 5, 1);
            label = dtk::Label::create(context, "Disk cache size (GB):", p.layout);
            p.layout->setGridPos(label, 6, 0);
            p.diskCacheEdit->setParent(p.layout);
            p.layout->setGridPos(p.diskCacheEdit, 6, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.renderWidthEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("USD/renderWidth", value);
                });

            p.complexitySlider->setCallback(
                [this](float value)
                {
                    _p->settings->setValue("USD/complexity", value);
                });

            p.drawModeComboBox->setIndexCallback(
                [this](int value)
                {
                    const usd::DrawMode drawMode = static_cast<usd::DrawMode>(value);
                    _p->settings->setValue("USD/drawMode", drawMode);
                });

            p.lightingCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->settings->setValue("USD/enableLighting", value);
                });

            p.sRGBCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->settings->setValue("USD/sRGB", value);
                });

            p.stageCacheEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("USD/stageCacheCount", value);
                });

            p.diskCacheEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("USD/diskCacheByteCount", value * dtk::gigabyte);
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

        void USDSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("USD/renderWidth" == name || name.empty())
            {
                p.renderWidthEdit->setValue(
                    p.settings->getValue<int>("USD/renderWidth"));
            }
            if ("USD/complexity" == name || name.empty())
            {
                p.complexitySlider->setValue(
                    p.settings->getValue<float>("USD/complexity"));
            }
            if ("USD/drawMode" == name || name.empty())
            {
                p.drawModeComboBox->setCurrentIndex(static_cast<int>(
                    p.settings->getValue<usd::DrawMode>("USD/drawMode")));
            }
            if ("USD/enableLighting" == name || name.empty())
            {
                p.lightingCheckBox->setChecked(
                    p.settings->getValue<bool>("USD/enableLighting"));
            }
            if ("USD/sRGB" == name || name.empty())
            {
                p.sRGBCheckBox->setChecked(
                    p.settings->getValue<bool>("USD/sRGB"));
            }
            if ("USD/stageCacheCount" == name || name.empty())
            {
                p.stageCacheEdit->setValue(
                    p.settings->getValue<size_t>("USD/stageCacheCount"));
            }
            if ("USD/diskCacheByteCount" == name || name.empty())
            {
                p.diskCacheEdit->setValue(
                    p.settings->getValue<size_t>("USD/diskCacheByteCount") / dtk::gigabyte);
            }
        }
#endif // TLRENDER_USD

        struct FileBrowserSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::CheckBox> nativeFileDialogCheckBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void FileBrowserSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::FileBrowserSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.nativeFileDialogCheckBox = dtk::CheckBox::create(context);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Native file dialog:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.nativeFileDialogCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.nativeFileDialogCheckBox, 0, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.nativeFileDialogCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->settings->setValue("FileBrowser/NativeFileDialog", value);
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

        void FileBrowserSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("FileBrowser/NativeFileDialog" == name || name.empty())
            {
                p.nativeFileDialogCheckBox->setChecked(
                    p.settings->getValue<bool>("FileBrowser/NativeFileDialog"));
            }
        }

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::IntEdit> audioBufferFramesEdit;
            std::shared_ptr<dtk::IntEdit> videoRequestsEdit;
            std::shared_ptr<dtk::IntEdit> audioRequestsEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void PerformanceSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::PerformanceSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

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
            auto gridLayout = dtk::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            label = dtk::Label::create(context, "Audio buffer frames:", gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.audioBufferFramesEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.audioBufferFramesEdit, 0, 1);
            label = dtk::Label::create(context, "Video requests:", gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.videoRequestsEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.videoRequestsEdit, 1, 1);
            label = dtk::Label::create(context, "Audio requests:", gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.audioRequestsEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.audioRequestsEdit, 2, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.audioBufferFramesEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("Performance/AudioBufferFrameCount", value);
                });

            p.videoRequestsEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("Performance/VideoRequestCount", value);
                });

            p.audioRequestsEdit->setCallback(
                [this](int value)
                {
                    _p->settings->setValue("Performance/AudioRequestCount", value);
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

        void PerformanceSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("Performance/AudioBufferFrameCount" == name || name.empty())
            {
                p.audioBufferFramesEdit->setValue(
                    p.settings->getValue<size_t>("Performance/AudioBufferFrameCount"));
            }
            if ("Performance/VideoRequestCount" == name || name.empty())
            {
                p.videoRequestsEdit->setValue(
                    p.settings->getValue<size_t>("Performance/VideoRequestCount"));
            }
            if ("Performance/AudioRequestCount" == name || name.empty())
            {
                p.audioRequestsEdit->setValue(
                    p.settings->getValue<size_t>("Performance/AudioRequestCount"));
            }
        }

        struct OpenGLSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::CheckBox> shareContextsCheckBox;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void OpenGLSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::OpenGLSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.shareContextsCheckBox = dtk::CheckBox::create(context);

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Changes are applied to new windows.", p.layout);
            auto gridLayout = dtk::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            label = dtk::Label::create(context, "Share contexts:", gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.shareContextsCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p.shareContextsCheckBox, 0, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.shareContextsCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->settings->setValue("OpenGL/ShareContexts", value);
                });
        }

        OpenGLSettingsWidget::OpenGLSettingsWidget() :
            _p(new Private)
        {}

        OpenGLSettingsWidget::~OpenGLSettingsWidget()
        {}

        std::shared_ptr<OpenGLSettingsWidget> OpenGLSettingsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<OpenGLSettingsWidget>(new OpenGLSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void OpenGLSettingsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void OpenGLSettingsWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void OpenGLSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("OpenGL/ShareContexts" == name || name.empty())
            {
                p.shareContextsCheckBox->setChecked(
                    p.settings->getValue<bool>("OpenGL/ShareContexts"));
            }
        }

        struct StyleSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::ComboBox> colorStyleComboBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void StyleSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::StyleSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.colorStyleComboBox = dtk::ComboBox::create(context, dtk::getColorStyleLabels());
            p.colorStyleComboBox->setHStretch(dtk::Stretch::Expanding);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Color style:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.colorStyleComboBox->setParent(p.layout);
            p.layout->setGridPos(p.colorStyleComboBox, 0, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.colorStyleComboBox->setIndexCallback(
                [this](int value)
                {
                    const dtk::ColorStyle colorStyle = static_cast<dtk::ColorStyle>(value);
                    _p->settings->setValue("Style/Palette", colorStyle);
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

        void StyleSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("Style/Palette" == name || name.empty())
            {
                p.colorStyleComboBox->setCurrentIndex(static_cast<int>(
                    p.settings->getValue<dtk::ColorStyle>("Style/Palette")));
            }
        }

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<play::Settings> settings;

            std::shared_ptr<dtk::CheckBox> toolTipsEnabledCheckBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<std::string> > settingsObserver;
        };

        void MiscSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::MiscSettingsWidget", parent);
            DTK_P();

            p.settings = app->getSettings();

            p.toolTipsEnabledCheckBox = dtk::CheckBox::create(context);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Enable tool tips:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.toolTipsEnabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.toolTipsEnabledCheckBox, 1, 1);

            _settingsUpdate(std::string());

            p.settingsObserver = dtk::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.toolTipsEnabledCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->settings->setValue("Misc/ToolTipsEnabled", value);
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

        void MiscSettingsWidget::_settingsUpdate(const std::string& name)
        {
            DTK_P();
            if ("Misc/ToolTipsEnabled" == name || name.empty())
            {
                p.toolTipsEnabledCheckBox->setChecked(
                    p.settings->getValue<bool>("Misc/ToolTipsEnabled"));
            }
        }

        struct SettingsTool::Private
        {
            std::shared_ptr<dtk::ScrollWidget> scrollWidget;
            std::shared_ptr<dtk::ToolButton> resetButton;
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
            auto fileSequenceWidget = FileSequenceSettingsWidget::create(context, app);
#if defined(TLRENDER_FFMPEG)
            auto ffmpegWidget = FFmpegSettingsWidget::create(context, app);
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            auto usdWidget = USDSettingsWidget::create(context, app);
#endif // TLRENDER_USD
            auto fileBrowserWidget = FileBrowserSettingsWidget::create(context, app);
            auto performanceWidget = PerformanceSettingsWidget::create(context, app);
            auto openGLWidget = OpenGLSettingsWidget::create(context, app);
            auto styleWidget = StyleSettingsWidget::create(context, app);
            auto miscWidget = MiscSettingsWidget::create(context, app);
            auto vLayout = dtk::VerticalLayout::create(context);
            vLayout->setSpacingRole(dtk::SizeRole::None);
            auto bellows = dtk::Bellows::create(context, "Cache", vLayout);
            bellows->setWidget(cacheWidget);
            bellows = dtk::Bellows::create(context, "File Sequences", vLayout);
            bellows->setWidget(fileSequenceWidget);
#if defined(TLRENDER_FFMPEG)
            bellows = dtk::Bellows::create(context, "FFmpeg", vLayout);
            bellows->setWidget(ffmpegWidget);
#endif // TLRENDER_USD
#if defined(TLRENDER_USD)
            bellows = dtk::Bellows::create(context, "USD", vLayout);
            bellows->setWidget(usdWidget);
#endif // TLRENDER_USD
            bellows = dtk::Bellows::create(context, "File Browser", vLayout);
            bellows->setWidget(fileBrowserWidget);
            bellows = dtk::Bellows::create(context, "Performance", vLayout);
            bellows->setWidget(performanceWidget);
            bellows = dtk::Bellows::create(context, "OpenGL", vLayout);
            bellows->setWidget(openGLWidget);
            bellows = dtk::Bellows::create(context, "Style", vLayout);
            bellows->setWidget(styleWidget);
            bellows = dtk::Bellows::create(context, "Miscellaneous", vLayout);
            bellows->setWidget(miscWidget);
            p.scrollWidget = dtk::ScrollWidget::create(context);
            p.scrollWidget->setWidget(vLayout);
            p.scrollWidget->setVStretch(dtk::Stretch::Expanding);

            p.resetButton = dtk::ToolButton::create(context, "Default Settings");

            p.layout = dtk::VerticalLayout::create(context);
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.scrollWidget->setParent(p.layout);
            auto hLayout = dtk::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(dtk::SizeRole::MarginInside);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.resetButton->setParent(hLayout);
            _setWidget(p.layout);

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
                                            app->getSettings()->reset();
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
        {}

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
