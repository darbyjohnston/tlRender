// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SettingsToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlPlay/SettingsModel.h>

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
#include <dtk/core/Format.h>

namespace tl
{
    namespace play_app
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::IntEdit> sizeGB;
            std::shared_ptr<dtk::DoubleEdit> readAhead;
            std::shared_ptr<dtk::DoubleEdit> readBehind;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<play::CacheOptions> > cacheObserver;
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

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Cache size (GB):", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.sizeGB->setParent(p.layout);
            p.layout->setGridPos(p.sizeGB, 0, 1);
            label = dtk::Label::create(context, "Read ahead (seconds):", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.readAhead->setParent(p.layout);
            p.layout->setGridPos(p.readAhead, 1, 1);
            label = dtk::Label::create(context, "Read behind (seconds):", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.readBehind->setParent(p.layout);
            p.layout->setGridPos(p.readBehind, 2, 1);

            p.cacheObserver = dtk::ValueObserver<play::CacheOptions>::create(
                p.model->observeCache(),
                [this](const play::CacheOptions& value)
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
                    play::CacheOptions cache = p.model->getCache();
                    cache.sizeGB = value;
                    p.model->setCache(cache);
                });

            p.readAhead->setCallback(
                [this](double value)
                {
                    DTK_P();
                    play::CacheOptions cache = p.model->getCache();
                    cache.readAhead = value;
                    p.model->setCache(cache);
                });

            p.readBehind->setCallback(
                [this](double value)
                {
                    DTK_P();
                    play::CacheOptions cache = p.model->getCache();
                    cache.readBehind = value;
                    p.model->setCache(cache);
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

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::ComboBox> audioComboBox;
            std::shared_ptr<dtk::LineEdit> audioFileNameEdit;
            std::shared_ptr<dtk::LineEdit> audioDirectoryEdit;
            std::shared_ptr<dtk::IntEdit> maxDigitsEdit;
            std::shared_ptr<dtk::DoubleEdit> defaultSpeedEdit;
            std::shared_ptr<dtk::IntEdit> threadsEdit;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<play::FileSequenceOptions> > fileSequenceObserver;
            std::shared_ptr<dtk::ValueObserver<io::SequenceOptions> > sequenceIOObserver;
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

            p.fileSequenceObserver = dtk::ValueObserver<play::FileSequenceOptions>::create(
                p.model->observeFileSequence(),
                [this](const play::FileSequenceOptions& value)
                {
                    DTK_P();
                    p.audioComboBox->setCurrentIndex(static_cast<int>(value.audio));
                    p.audioFileNameEdit->setText(value.audioFileName);
                    p.audioDirectoryEdit->setText(value.audioDirectory);
                    p.maxDigitsEdit->setValue(value.maxDigits);
                });

            p.sequenceIOObserver = dtk::ValueObserver<io::SequenceOptions>::create(
                p.model->observeSequenceIO(),
                [this](const io::SequenceOptions& value)
                {
                    DTK_P();
                    p.defaultSpeedEdit->setValue(value.defaultSpeed);
                    p.threadsEdit->setValue(value.threadCount);
                });

            p.audioComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.audio = static_cast<timeline::FileSequenceAudio>(value);
                    p.model->setFileSequence(fileSequence);
                });

            p.audioFileNameEdit->setTextCallback(
                [this](const std::string& value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.audioFileName = value;
                    p.model->setFileSequence(fileSequence);
                });

            p.audioDirectoryEdit->setTextCallback(
                [this](const std::string& value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.audioDirectory = value;
                    p.model->setFileSequence(fileSequence);
                });

            p.maxDigitsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.maxDigits = value;
                    p.model->setFileSequence(fileSequence);
                });

            p.defaultSpeedEdit->setCallback(
                [this](double value)
                {
                    DTK_P();
                    io::SequenceOptions sequenceIO = p.model->getSequenceIO();
                    sequenceIO.defaultSpeed = value;
                    p.model->setSequenceIO(sequenceIO);
                });

            p.threadsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    io::SequenceOptions sequenceIO = p.model->getSequenceIO();
                    sequenceIO.threadCount = value;
                    p.model->setSequenceIO(sequenceIO);
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

#if defined(TLRENDER_FFMPEG)
        struct FFmpegSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::CheckBox> yuvToRGBCheckBox;
            std::shared_ptr<dtk::IntEdit> threadsEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<ffmpeg::Options> > ffmpegObserver;
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

            p.ffmpegObserver = dtk::ValueObserver<ffmpeg::Options>::create(
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
                    ffmpeg::Options ffmpeg = p.model->getFFmpeg();
                    ffmpeg.yuvToRgb = value;
                    p.model->setFFmpeg(ffmpeg);
                });

            p.threadsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    ffmpeg::Options ffmpeg = p.model->getFFmpeg();
                    ffmpeg.threadCount = value;
                    p.model->setFFmpeg(ffmpeg);
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
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::IntEdit> renderWidthEdit;
            std::shared_ptr<dtk::FloatEditSlider> complexitySlider;
            std::shared_ptr<dtk::ComboBox> drawModeComboBox;
            std::shared_ptr<dtk::CheckBox> lightingCheckBox;
            std::shared_ptr<dtk::CheckBox> sRGBCheckBox;
            std::shared_ptr<dtk::IntEdit> stageCacheEdit;
            std::shared_ptr<dtk::IntEdit> diskCacheEdit;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<usd::Options> > usdObserver;
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

            p.usdObserver = dtk::ValueObserver<usd::Options>::create(
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
                    usd::Options usd = p.model->getUSD();
                    usd.renderWidth = value;
                    p.model->setUSD(usd);
                });

            p.complexitySlider->setCallback(
                [this](float value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.complexity = value;
                    p.model->setUSD(usd);
                });

            p.drawModeComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.drawMode = static_cast<usd::DrawMode>(value);
                    p.model->setUSD(usd);
                });

            p.lightingCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.enableLighting = value;
                    p.model->setUSD(usd);
                });

            p.sRGBCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.sRGB = value;
                    p.model->setUSD(usd);
                });

            p.stageCacheEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.stageCache = value;
                    p.model->setUSD(usd);
                });

            p.diskCacheEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.diskCache = value;
                    p.model->setUSD(usd);
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

        struct FileBrowserSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::CheckBox> nfdCheckBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<bool> > nfdObserver;
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

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Native file dialog:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.nfdCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.nfdCheckBox, 0, 1);

            p.nfdObserver = dtk::ValueObserver<bool>::create(
                p.model->observeNativeFileDialog(),
                [this](bool value)
                {
                    _p->nfdCheckBox->setChecked(value);
                });

            p.nfdCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->model->setNativeFileDialog(value);
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

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::IntEdit> audioBufferFramesEdit;
            std::shared_ptr<dtk::IntEdit> videoRequestsEdit;
            std::shared_ptr<dtk::IntEdit> audioRequestsEdit;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<play::PerformanceOptions> > performanceObserver;
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

            p.performanceObserver = dtk::ValueObserver<play::PerformanceOptions>::create(
                p.model->observePerformance(),
                [this](const play::PerformanceOptions& value)
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
                    auto performance = p.model->getPerformance();
                    performance.audioBufferFrameCount = value;
                    p.model->setPerformance(performance);
                });

            p.videoRequestsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto performance = p.model->getPerformance();
                    performance.videoRequestCount = value;
                    p.model->setPerformance(performance);
                });

            p.audioRequestsEdit->setCallback(
                [this](int value)
                {
                    DTK_P();
                    auto performance = p.model->getPerformance();
                    performance.audioRequestCount = value;
                    p.model->setPerformance(performance);
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
            std::shared_ptr<play::SettingsModel> model;

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
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<play::StyleOptions> > styleObserver;
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

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Color style:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.colorStyleComboBox->setParent(p.layout);
            p.layout->setGridPos(p.colorStyleComboBox, 0, 1);
            label = dtk::Label::create(context, "Display scale:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.displayScaleComboBox->setParent(p.layout);
            p.layout->setGridPos(p.displayScaleComboBox, 1, 1);

            p.styleObserver = dtk::ValueObserver<play::StyleOptions>::create(
                app->getSettingsModel()->observeStyle(),
                [this](const play::StyleOptions& value)
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
                    auto style = p.model->getStyle();
                    style.colorStyle = static_cast<dtk::ColorStyle>(value);
                    p.model->setStyle(style);
                });

            p.displayScaleComboBox->setIndexCallback(
                [this](int value)
                {
                    DTK_P();
                    auto style = p.model->getStyle();
                    if (value >= 0 && value < p.displayScales.size())
                    {
                        style.displayScale = p.displayScales[value];
                    }
                    p.model->setStyle(style);
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

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            std::shared_ptr<dtk::CheckBox> toolTipsEnabledCheckBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<bool> > tooltipsEnabledObserver;
        };

        void MiscSettingsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play_app::MiscSettingsWidget", parent);
            DTK_P();

            p.model = app->getSettingsModel();

            p.toolTipsEnabledCheckBox = dtk::CheckBox::create(context);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Enable tool tips:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.toolTipsEnabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.toolTipsEnabledCheckBox, 1, 1);

            p.tooltipsEnabledObserver = dtk::ValueObserver<bool>::create(
                app->getSettingsModel()->observeTooltipsEnabled(),
                [this](bool value)
                {
                    _p->toolTipsEnabledCheckBox->setChecked(value);
                });

            p.toolTipsEnabledCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->model->setTooltipsEnabled(value);
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
