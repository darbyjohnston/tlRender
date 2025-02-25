// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SettingsToolPrivate.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlPlay/SettingsModel.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <tlQt/MetaTypes.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            QSpinBox* cacheSizeSpinBox = nullptr;
            QDoubleSpinBox* readAheadSpinBox = nullptr;
            QDoubleSpinBox* readBehindSpinBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<play::CacheOptions> > cacheObserver;
        };

        CacheSettingsWidget::CacheSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.model = app->settingsModel();

            p.cacheSizeSpinBox = new QSpinBox;
            p.cacheSizeSpinBox->setRange(0, 1024);

            p.readAheadSpinBox = new QDoubleSpinBox;
            p.readAheadSpinBox->setRange(0.0, 60.0);

            p.readBehindSpinBox = new QDoubleSpinBox;
            p.readBehindSpinBox->setRange(0, 60.0);

            auto layout = new QFormLayout;
            layout->addRow(tr("Cache size (GB):"), p.cacheSizeSpinBox);
            layout->addRow(tr("Read ahead (seconds):"), p.readAheadSpinBox);
            layout->addRow(tr("Read behind (seconds):"), p.readBehindSpinBox);
            setLayout(layout);

            p.cacheObserver = dtk::ValueObserver<play::CacheOptions>::create(
                p.model->observeCache(),
                [this](const play::CacheOptions& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.cacheSizeSpinBox);
                        p.cacheSizeSpinBox->setValue(value.sizeGB);
                    }
                    {
                        QSignalBlocker signalBlocker(p.readAheadSpinBox);
                        p.readAheadSpinBox->setValue(value.readAhead);

                    }
                    {
                        QSignalBlocker signalBlocker(p.readBehindSpinBox);
                        p.readBehindSpinBox->setValue(value.readBehind);
                    }
                });

            connect(
                p.cacheSizeSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    play::CacheOptions cache = p.model->getCache();
                    cache.sizeGB = value;
                    p.model->setCache(cache);
                });

            connect(
                p.readAheadSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    DTK_P();
                    play::CacheOptions cache = p.model->getCache();
                    cache.readAhead = value;
                    p.model->setCache(cache);
                });

            connect(
                p.readBehindSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    DTK_P();
                    play::CacheOptions cache = p.model->getCache();
                    cache.readBehind = value;
                    p.model->setCache(cache);
                });
        }

        CacheSettingsWidget::~CacheSettingsWidget()
        {}

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            QComboBox* audioComboBox = nullptr;
            QLineEdit* audioFileName = nullptr;
            QLineEdit* audioDirectory = nullptr;
            QSpinBox* maxDigitsSpinBox = nullptr;
            QDoubleSpinBox* defaultSpeedSpinBox = nullptr;
            QSpinBox* threadCountSpinBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<play::FileSequenceOptions> > fileSequenceObserver;
            std::shared_ptr<dtk::ValueObserver<io::SequenceOptions> > sequenceIOObserver;
        };

        FileSequenceSettingsWidget::FileSequenceSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.model = app->settingsModel();

            p.audioComboBox = new QComboBox;
            for (const auto& i : timeline::getFileSequenceAudioLabels())
            {
                p.audioComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioFileName = new QLineEdit;

            p.audioDirectory = new QLineEdit;

            p.maxDigitsSpinBox = new QSpinBox;
            p.maxDigitsSpinBox->setRange(0, 255);

            p.defaultSpeedSpinBox = new QDoubleSpinBox;
            p.defaultSpeedSpinBox->setRange(1.0, 120.0);

            p.threadCountSpinBox = new QSpinBox;
            p.threadCountSpinBox->setRange(1, 64);

            auto layout = new QFormLayout;
            layout->addRow(tr("Audio:"), p.audioComboBox);
            layout->addRow(tr("Audio file name:"), p.audioFileName);
            layout->addRow(tr("Audio directory:"), p.audioDirectory);
            layout->addRow(tr("Maximum digits:"), p.maxDigitsSpinBox);
            layout->addRow(tr("Default speed (FPS):"), p.defaultSpeedSpinBox);
            layout->addRow(tr("I/O threads:"), p.threadCountSpinBox);
            setLayout(layout);

            p.fileSequenceObserver = dtk::ValueObserver<play::FileSequenceOptions>::create(
                p.model->observeFileSequence(),
                [this](const play::FileSequenceOptions& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.audioComboBox);
                        p.audioComboBox->setCurrentIndex(static_cast<int>(value.audio));
                    }
                    {
                        QSignalBlocker signalBlocker(p.audioFileName);
                        p.audioFileName->setText(QString::fromUtf8(value.audioFileName.c_str()));
                    }
                    {
                        QSignalBlocker signalBlocker(p.audioDirectory);
                        p.audioDirectory->setText(QString::fromUtf8(value.audioDirectory.c_str()));
                    }
                    {
                        QSignalBlocker signalBlocker(p.maxDigitsSpinBox);
                        p.maxDigitsSpinBox->setValue(value.maxDigits);
                    }
                });

            p.sequenceIOObserver = dtk::ValueObserver<io::SequenceOptions>::create(
                p.model->observeSequenceIO(),
                [this](const io::SequenceOptions& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.defaultSpeedSpinBox);
                        p.defaultSpeedSpinBox->setValue(value.defaultSpeed);
                    }
                    {
                        QSignalBlocker signalBlocker(p.threadCountSpinBox);
                        p.threadCountSpinBox->setValue(value.threadCount);
                    }
                });

            connect(
                p.audioComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.audio = static_cast<timeline::FileSequenceAudio>(value);
                    p.model->setFileSequence(fileSequence);
                });

            connect(
                p.audioFileName,
                &QLineEdit::textChanged,
                [this](const QString& value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.audioFileName = value.toUtf8().data();
                    p.model->setFileSequence(fileSequence);
                });

            connect(
                p.audioDirectory,
                &QLineEdit::textChanged,
                [this](const QString& value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.audioDirectory = value.toUtf8().data();
                    p.model->setFileSequence(fileSequence);
                });

            connect(
                p.maxDigitsSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    play::FileSequenceOptions fileSequence = p.model->getFileSequence();
                    fileSequence.maxDigits = value;
                    p.model->setFileSequence(fileSequence);
                });

            connect(
                p.defaultSpeedSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    DTK_P();
                    io::SequenceOptions sequenceIO = p.model->getSequenceIO();
                    sequenceIO.defaultSpeed = value;
                    p.model->setSequenceIO(sequenceIO);
                });

            connect(
                p.threadCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    io::SequenceOptions sequenceIO = p.model->getSequenceIO();
                    sequenceIO.threadCount = value;
                    p.model->setSequenceIO(sequenceIO);
                });
        }

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget()
        {}

#if defined(TLRENDER_FFMPEG)
        struct FFmpegSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            QCheckBox* yuvToRGBCheckBox = nullptr;
            QSpinBox* threadCountSpinBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<ffmpeg::Options> > ffmpegObserver;
        };

        FFmpegSettingsWidget::FFmpegSettingsWidget(App * app, QWidget * parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.model = app->settingsModel();

            p.yuvToRGBCheckBox = new QCheckBox;

            p.threadCountSpinBox = new QSpinBox;
            p.threadCountSpinBox->setRange(0, 64);

            auto layout = new QFormLayout;
            auto label = new QLabel(tr("Changes are applied to new files."));
            label->setWordWrap(true);
            layout->addRow(label);
            layout->addRow(tr("YUV to RGB conversion:"), p.yuvToRGBCheckBox);
            layout->addRow(tr("I/O threads:"), p.threadCountSpinBox);
            setLayout(layout);

            p.ffmpegObserver = dtk::ValueObserver<ffmpeg::Options>::create(
                p.model->observeFFmpeg(),
                [this](const ffmpeg::Options& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.yuvToRGBCheckBox);
                        p.yuvToRGBCheckBox->setChecked(value.yuvToRgb);
                    }
                    {
                        QSignalBlocker signalBlocker(p.threadCountSpinBox);
                        p.threadCountSpinBox->setValue(value.threadCount);
                    }
                });

            connect(
                p.yuvToRGBCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    DTK_P();
                    ffmpeg::Options ffmpeg = p.model->getFFmpeg();
                    ffmpeg.yuvToRgb = value;
                    p.model->setFFmpeg(ffmpeg);
                });

            connect(
                p.threadCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    ffmpeg::Options ffmpeg = p.model->getFFmpeg();
                    ffmpeg.threadCount = value;
                    p.model->setFFmpeg(ffmpeg);
                });
        }

        FFmpegSettingsWidget::~FFmpegSettingsWidget()
        {}
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        struct USDSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            QSpinBox* renderWidthSpinBox = nullptr;
            qtwidget::FloatEditSlider* complexitySlider = nullptr;
            QComboBox* drawModeComboBox = nullptr;
            QCheckBox* lightingCheckBox = nullptr;
            QCheckBox* sRGBCheckBox = nullptr;
            QSpinBox* stageCacheSpinBox = nullptr;
            QSpinBox* diskCacheSpinBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<usd::Options> > usdObserver;
        };

        USDSettingsWidget::USDSettingsWidget(App * app, QWidget * parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.model = app->settingsModel();

            p.renderWidthSpinBox = new QSpinBox;
            p.renderWidthSpinBox->setRange(1, 8192);

            p.complexitySlider = new qtwidget::FloatEditSlider;

            p.drawModeComboBox = new QComboBox;
            for (const auto& i : usd::getDrawModeLabels())
            {
                p.drawModeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.lightingCheckBox = new QCheckBox;

            p.sRGBCheckBox = new QCheckBox;

            p.stageCacheSpinBox = new QSpinBox;
            p.stageCacheSpinBox->setRange(0, 10);

            p.diskCacheSpinBox = new QSpinBox;
            p.diskCacheSpinBox->setRange(0, 1024);

            auto layout = new QFormLayout;
            layout->addRow(tr("Render width:"), p.renderWidthSpinBox);
            layout->addRow(tr("Render complexity:"), p.complexitySlider);
            layout->addRow(tr("Draw mode:"), p.drawModeComboBox);
            layout->addRow(tr("Enable lighting:"), p.lightingCheckBox);
            layout->addRow(tr("Enable sRGB color space:"), p.sRGBCheckBox);
            layout->addRow(tr("Stage cache size:"), p.stageCacheSpinBox);
            layout->addRow(tr("Disk cache size (GB):"), p.diskCacheSpinBox);
            setLayout(layout);

            p.usdObserver = dtk::ValueObserver<usd::Options>::create(
                p.model->observeUSD(),
                [this](const usd::Options& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.renderWidthSpinBox);
                        p.renderWidthSpinBox->setValue(value.renderWidth);
                    }
                    {
                        QSignalBlocker signalBlocker(p.complexitySlider);
                        p.complexitySlider->setValue(value.complexity);
                    }
                    {
                        QSignalBlocker signalBlocker(p.drawModeComboBox);
                        p.drawModeComboBox->setCurrentIndex(static_cast<int>(value.drawMode));
                    }
                    {
                        QSignalBlocker signalBlocker(p.lightingCheckBox);
                        p.lightingCheckBox->setChecked(value.enableLighting);
                    }
                    {
                        QSignalBlocker signalBlocker(p.sRGBCheckBox);
                        p.sRGBCheckBox->setChecked(value.sRGB);
                    }
                    {
                        QSignalBlocker signalBlocker(p.stageCacheSpinBox);
                        p.stageCacheSpinBox->setValue(value.stageCache);
                    }
                    {
                        QSignalBlocker signalBlocker(p.diskCacheSpinBox);
                        p.diskCacheSpinBox->setValue(value.diskCache);
                    }
                });

            connect(
                p.renderWidthSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.renderWidth = value;
                    p.model->setUSD(usd);
                });

            connect(
                p.complexitySlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.complexity = value;
                    p.model->setUSD(usd);
                });

            connect(
                p.drawModeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.drawMode = static_cast<usd::DrawMode>(value);
                    p.model->setUSD(usd);
                });

            connect(
                p.lightingCheckBox,
                &QCheckBox::stateChanged,
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.enableLighting = value;
                    p.model->setUSD(usd);
                });

            connect(
                p.sRGBCheckBox,
                &QCheckBox::stateChanged,
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.sRGB = value;
                    p.model->setUSD(usd);
                });

            connect(
                p.stageCacheSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.stageCache = value;
                    p.model->setUSD(usd);
                });

            connect(
                p.diskCacheSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    usd::Options usd = p.model->getUSD();
                    usd.diskCache = value;
                    p.model->setUSD(usd);
                });
        }

        USDSettingsWidget::~USDSettingsWidget()
        {}
#endif // TLRENDER_USD

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            QSpinBox* audioBufferFrameCountSpinBox = nullptr;
            QSpinBox* videoRequestCountSpinBox = nullptr;
            QSpinBox* audioRequestCountSpinBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<play::PerformanceOptions> > performanceObserver;
        };

        PerformanceSettingsWidget::PerformanceSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.model = app->settingsModel();

            p.audioBufferFrameCountSpinBox = new QSpinBox;
            p.audioBufferFrameCountSpinBox->setRange(1024, 4096);

            p.videoRequestCountSpinBox = new QSpinBox;
            p.videoRequestCountSpinBox->setRange(1, 64);

            p.audioRequestCountSpinBox = new QSpinBox;
            p.audioRequestCountSpinBox->setRange(1, 64);

            auto layout = new QFormLayout;
            auto label = new QLabel(tr("Changes are applied to new files."));
            label->setWordWrap(true);
            layout->addRow(label);
            layout->addRow(tr("Audio buffer frames:"), p.audioBufferFrameCountSpinBox);
            layout->addRow(tr("Video requests:"), p.videoRequestCountSpinBox);
            layout->addRow(tr("Audio requests:"), p.audioRequestCountSpinBox);
            setLayout(layout);

            p.performanceObserver = dtk::ValueObserver<play::PerformanceOptions>::create(
                p.model->observePerformance(),
                [this](const play::PerformanceOptions& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.audioBufferFrameCountSpinBox);
                        p.audioBufferFrameCountSpinBox->setValue(value.audioBufferFrameCount);
                    }
                    {
                        QSignalBlocker signalBlocker(p.videoRequestCountSpinBox);
                        p.videoRequestCountSpinBox->setValue(value.videoRequestCount);
                    }
                    {
                        QSignalBlocker signalBlocker(p.audioRequestCountSpinBox);
                        p.audioRequestCountSpinBox->setValue(value.audioRequestCount);
                    }
                });

            connect(
                p.audioBufferFrameCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    auto performance = p.model->getPerformance();
                    performance.audioBufferFrameCount = value;
                    p.model->setPerformance(performance);
                });

            connect(
                p.videoRequestCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    auto performance = p.model->getPerformance();
                    performance.videoRequestCount = value;
                    p.model->setPerformance(performance);
                });

            connect(
                p.audioRequestCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [this](int value)
                {
                    DTK_P();
                    auto performance = p.model->getPerformance();
                    performance.audioRequestCount = value;
                    p.model->setPerformance(performance);
                });
        }

        PerformanceSettingsWidget::~PerformanceSettingsWidget()
        {}

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<play::SettingsModel> model;

            QCheckBox* toolTipsCheckBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<bool> > tooltipsEnabledObserver;
        };

        MiscSettingsWidget::MiscSettingsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.model = app->settingsModel();

            p.toolTipsCheckBox = new QCheckBox;
            p.toolTipsCheckBox->setText(tr("Enable tool tips"));

            auto layout = new QFormLayout;
            layout->addRow(p.toolTipsCheckBox);
            setLayout(layout);

            p.tooltipsEnabledObserver = dtk::ValueObserver<bool>::create(
                p.model->observeTooltipsEnabled(),
                [this](bool value)
                {
                    DTK_P();
                    {
                        QSignalBlocker signalBlocker(p.toolTipsCheckBox);
                        _p->toolTipsCheckBox->setChecked(value);
                    }
                });

            connect(
                p.toolTipsCheckBox,
                &QCheckBox::stateChanged,
                [this](int value)
                {
                    _p->model->setTooltipsEnabled(value);
                });
        }

        MiscSettingsWidget::~MiscSettingsWidget()
        {}

        SettingsTool::SettingsTool(App* app, QWidget* parent) :
            IToolWidget(app, parent)
        {
            addBellows(tr("Cache"), new CacheSettingsWidget(app));
            addBellows(tr("File Sequences"), new FileSequenceSettingsWidget(app));
#if defined(TLRENDER_FFMPEG)
            addBellows(tr("FFmpeg"), new FFmpegSettingsWidget(app));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            addBellows(tr("USD"), new USDSettingsWidget(app));
#endif // TLRENDER_USD
            addBellows(tr("Performance"), new PerformanceSettingsWidget(app));
            addBellows(tr("Miscellaneous"), new MiscSettingsWidget(app));
            auto resetButton = new QToolButton;
            resetButton->setText(tr("Default Settings"));
            resetButton->setAutoRaise(true);
            auto layout = new QHBoxLayout;
            layout->setContentsMargins(5, 5, 5, 5);
            layout->setSpacing(5);
            layout->addWidget(resetButton);
            layout->addStretch();
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);
            addStretch();

            connect(
                resetButton,
                &QToolButton::clicked,
                [app]
                {
                    QMessageBox messageBox;
                    messageBox.setText("Reset preferences to default values?");
                    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                    messageBox.setDefaultButton(QMessageBox::Ok);
                    if (messageBox.exec() == QMessageBox::Ok)
                    {
                        app->settingsModel()->reset();
                    }
                });
        }

        SettingsDockWidget::SettingsDockWidget(
            SettingsTool* settingsTool,
            QWidget* parent)
        {
            setObjectName("SettingsTool");
            setWindowTitle(tr("Settings"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Settings"));
            dockTitleBar->setIcon(QIcon(":/Icons/Settings.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(settingsTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Settings.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F7));
            toggleViewAction()->setToolTip(tr("Show settings"));
        }
    }
}
