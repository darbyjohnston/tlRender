// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/SettingsTool.h>

#include <tlPlay/SettingsObject.h>

#include <tlQWidget/RadioButtonGroup.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QSpinBox>

namespace tl
{
    namespace play
    {
        struct CacheSettingsWidget::Private
        {
            QDoubleSpinBox* readAheadSpinBox = nullptr;
            QDoubleSpinBox* readBehindSpinBox = nullptr;
        };

        CacheSettingsWidget::CacheSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.readAheadSpinBox = new QDoubleSpinBox;
            p.readAheadSpinBox->setRange(0.0, 60.0);

            p.readBehindSpinBox = new QDoubleSpinBox;
            p.readBehindSpinBox->setRange(0, 60.0);

            auto layout = new QVBoxLayout;
            layout->addWidget(new QLabel(tr("Read Ahead")));
            layout->addWidget(p.readAheadSpinBox);
            layout->addWidget(new QLabel(tr("Read Behind")));
            layout->addWidget(p.readBehindSpinBox);
            setLayout(layout);

            p.readAheadSpinBox->setValue(settingsObject->cacheReadAhead());
            p.readBehindSpinBox->setValue(settingsObject->cacheReadBehind());

            connect(
                p.readAheadSpinBox,
                SIGNAL(valueChanged(double)),
                settingsObject,
                SLOT(setCacheReadAhead(double)));

            connect(
                p.readBehindSpinBox,
                SIGNAL(valueChanged(double)),
                settingsObject,
                SLOT(setCacheReadBehind(double)));

            connect(
                settingsObject,
                SIGNAL(cacheReadAheadChanged(double)),
                SLOT(_readAheadCallback(double)));
            connect(
                settingsObject,
                SIGNAL(cacheReadBehindChanged(double)),
                SLOT(_readBehindCallback(double)));
        }

        CacheSettingsWidget::~CacheSettingsWidget()
        {}

        void CacheSettingsWidget::_readAheadCallback(double value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.readAheadSpinBox);
            p.readAheadSpinBox->setValue(value);
        }

        void CacheSettingsWidget::_readBehindCallback(double value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.readBehindSpinBox);
            p.readBehindSpinBox->setValue(value);
        }

        struct FileSequenceSettingsWidget::Private
        {
            SettingsObject* settingsObject = nullptr;
            QComboBox* audioComboBox = nullptr;
            QLineEdit* audioFileName = nullptr;
            QLineEdit* audioDirectory = nullptr;
            QSpinBox* maxDigitsSpinBox = nullptr;
        };

        FileSequenceSettingsWidget::FileSequenceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settingsObject = settingsObject;

            p.audioComboBox = new QComboBox;
            for (const auto& i : timeline::getFileSequenceAudioLabels())
            {
                p.audioComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioFileName = new QLineEdit;

            p.audioDirectory = new QLineEdit;

            p.maxDigitsSpinBox = new QSpinBox;
            p.maxDigitsSpinBox->setRange(0, 255);

            auto layout = new QVBoxLayout;
            layout->addWidget(new QLabel(tr("Audio")));
            layout->addWidget(p.audioComboBox);
            layout->addWidget(new QLabel(tr("Audio File Name")));
            layout->addWidget(p.audioFileName);
            layout->addWidget(new QLabel(tr("Audio Directory")));
            layout->addWidget(p.audioDirectory);
            layout->addWidget(new QLabel(tr("Maximum Digits")));
            layout->addWidget(p.maxDigitsSpinBox);
            setLayout(layout);

            p.audioComboBox->setCurrentIndex(static_cast<int>(settingsObject->fileSequenceAudio()));
            p.audioFileName->setText(settingsObject->fileSequenceAudioFileName());
            p.audioDirectory->setText(settingsObject->fileSequenceAudioDirectory());
            p.maxDigitsSpinBox->setValue(static_cast<int>(settingsObject->maxFileSequenceDigits()));

            connect(
                p.audioComboBox,
                SIGNAL(activated(int)),
                SLOT(_audioCallback(int)));

            connect(
                p.audioFileName,
                SIGNAL(textChanged(const QString&)),
                settingsObject,
                SLOT(setFileSequenceAudioFileName(const QString&)));

            connect(
                p.audioDirectory,
                SIGNAL(textChanged(const QString&)),
                settingsObject,
                SLOT(setFileSequenceAudioDirectory(const QString&)));

            connect(
                p.maxDigitsSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setMaxFileSequenceDigits(int)));

            connect(
                settingsObject,
                SIGNAL(fileSequenceAudioChanged(tl::timeline::FileSequenceAudio)),
                SLOT(_audioCallback(tl::timeline::FileSequenceAudio)));
            connect(
                settingsObject,
                SIGNAL(fileSequenceAudioFileNameChanged(const QString&)),
                SLOT(_audioFileNameCallback(const QString&)));
            connect(
                settingsObject,
                SIGNAL(fileSequenceAudioDirectoryChanged(const QString&)),
                SLOT(_audioDirectoryCallback(const QString&)));
            connect(
                settingsObject,
                SIGNAL(maxFileSequenceDigitsChanged(int)),
                SLOT(_maxDigitsCallback(int)));
        }

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget()
        {}

        void FileSequenceSettingsWidget::_audioCallback(int value)
        {
            TLRENDER_P();
            p.settingsObject->setFileSequenceAudio(static_cast<timeline::FileSequenceAudio>(value));
        }

        void FileSequenceSettingsWidget::_audioCallback(timeline::FileSequenceAudio value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.audioComboBox);
            p.audioComboBox->setCurrentIndex(static_cast<int>(value));
        }

        void FileSequenceSettingsWidget::_audioFileNameCallback(const QString& value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.audioFileName);
            p.audioFileName->setText(value);
        }

        void FileSequenceSettingsWidget::_audioDirectoryCallback(const QString& value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.audioDirectory);
            p.audioDirectory->setText(value);
        }

        void FileSequenceSettingsWidget::_maxDigitsCallback(int value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.maxDigitsSpinBox);
            p.maxDigitsSpinBox->setValue(value);
        }

        struct PerformanceSettingsWidget::Private
        {
            SettingsObject* settingsObject = nullptr;
            QComboBox* timerModeComboBox = nullptr;
            QComboBox* audioBufferFrameCountComboBox = nullptr;
            QSpinBox* videoRequestCountSpinBox = nullptr;
            QSpinBox* audioRequestCountSpinBox = nullptr;
            QSpinBox* sequenceThreadCountSpinBox = nullptr;
            QSpinBox* ffmpegThreadCountSpinBox = nullptr;
        };

        PerformanceSettingsWidget::PerformanceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settingsObject = settingsObject;

            p.timerModeComboBox = new QComboBox;
            for (const auto& i : timeline::getTimerModeLabels())
            {
                p.timerModeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioBufferFrameCountComboBox = new QComboBox;
            for (const auto& i : timeline::getAudioBufferFrameCountLabels())
            {
                p.audioBufferFrameCountComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.videoRequestCountSpinBox = new QSpinBox;
            p.videoRequestCountSpinBox->setRange(1, 64);

            p.audioRequestCountSpinBox = new QSpinBox;
            p.audioRequestCountSpinBox->setRange(1, 64);

            p.sequenceThreadCountSpinBox = new QSpinBox;
            p.sequenceThreadCountSpinBox->setRange(1, 64);

            p.ffmpegThreadCountSpinBox = new QSpinBox;
            p.ffmpegThreadCountSpinBox->setRange(1, 64);

            auto layout = new QVBoxLayout;
            auto label = new QLabel(tr("Changes are applied to newly opened files."));
            label->setWordWrap(true);
            layout->addWidget(label);
            layout->addWidget(new QLabel(tr("Timer Mode")));
            layout->addWidget(p.timerModeComboBox);
            layout->addWidget(new QLabel(tr("Audio Buffer Frame Count")));
            layout->addWidget(p.audioBufferFrameCountComboBox);
            layout->addWidget(new QLabel(tr("Timeline Video Requests")));
            layout->addWidget(p.videoRequestCountSpinBox);
            layout->addWidget(new QLabel(tr("Timeline Audio Requests")));
            layout->addWidget(p.audioRequestCountSpinBox);
            layout->addWidget(new QLabel(tr("Sequence I/O Threads")));
            layout->addWidget(p.sequenceThreadCountSpinBox);
            layout->addWidget(new QLabel(tr("FFmpeg I/O threads")));
            layout->addWidget(p.ffmpegThreadCountSpinBox);
            setLayout(layout);

            p.timerModeComboBox->setCurrentIndex(static_cast<int>(settingsObject->timerMode()));
            p.audioBufferFrameCountComboBox->setCurrentIndex(static_cast<int>(settingsObject->audioBufferFrameCount()));
            p.videoRequestCountSpinBox->setValue(settingsObject->videoRequestCount());
            p.audioRequestCountSpinBox->setValue(settingsObject->audioRequestCount());
            p.sequenceThreadCountSpinBox->setValue(settingsObject->sequenceThreadCount());
            p.ffmpegThreadCountSpinBox->setValue(settingsObject->ffmpegThreadCount());

            connect(
                p.timerModeComboBox,
                SIGNAL(activated(int)),
                SLOT(_timerModeCallback(int)));

            connect(
                p.audioBufferFrameCountComboBox,
                SIGNAL(activated(int)),
                SLOT(_audioBufferFrameCountCallback(int)));

            connect(
                p.videoRequestCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setVideoRequestCount(int)));

            connect(
                p.audioRequestCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setAudioRequestCount(int)));

            connect(
                p.sequenceThreadCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setSequenceThreadCount(int)));

            connect(
                p.ffmpegThreadCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setFFmpegThreadCount(int)));

            connect(
                settingsObject,
                SIGNAL(timerModeChanged(tl::timeline::TimerMode)),
                SLOT(_timerModeCallback(tl::timeline::TimerMode)));
            connect(
                settingsObject,
                SIGNAL(audioBufferFrameCountChanged(tl::timeline::AudioBufferFrameCount)),
                SLOT(_audioBufferFrameCountCallback(tl::timeline::AudioBufferFrameCount)));
            connect(
                settingsObject,
                SIGNAL(videoRequestCountChanged(int)),
                SLOT(_videoRequestCountCallback(int)));
            connect(
                settingsObject,
                SIGNAL(audioRequestCountChanged(int)),
                SLOT(_audioRequestCountCallback(int)));
            connect(
                settingsObject,
                SIGNAL(sequenceThreadCountChanged(int)),
                SLOT(_sequenceThreadCountCallback(int)));
            connect(
                settingsObject,
                SIGNAL(ffmpegThreadCountChanged(int)),
                SLOT(_ffmpegThreadCountCallback(int)));
        }

        PerformanceSettingsWidget::~PerformanceSettingsWidget()
        {}

        void PerformanceSettingsWidget::_timerModeCallback(int value)
        {
            TLRENDER_P();
            p.settingsObject->setTimerMode(static_cast<timeline::TimerMode>(value));
        }

        void PerformanceSettingsWidget::_timerModeCallback(timeline::TimerMode value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.timerModeComboBox);
            p.timerModeComboBox->setCurrentIndex(static_cast<int>(value));
        }

        void PerformanceSettingsWidget::_audioBufferFrameCountCallback(int value)
        {
            TLRENDER_P();
            p.settingsObject->setAudioBufferFrameCount(static_cast<timeline::AudioBufferFrameCount>(value));
        }

        void PerformanceSettingsWidget::_audioBufferFrameCountCallback(timeline::AudioBufferFrameCount value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.audioRequestCountSpinBox);
            p.audioBufferFrameCountComboBox->setCurrentIndex(static_cast<int>(value));
        }

        void PerformanceSettingsWidget::_videoRequestCountCallback(int value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.videoRequestCountSpinBox);
            p.videoRequestCountSpinBox->setValue(value);
        }

        void PerformanceSettingsWidget::_audioRequestCountCallback(int value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.audioRequestCountSpinBox);
            p.audioRequestCountSpinBox->setValue(value);
        }

        void PerformanceSettingsWidget::_sequenceThreadCountCallback(int value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.sequenceThreadCountSpinBox);
            p.sequenceThreadCountSpinBox->setValue(value);
        }

        void PerformanceSettingsWidget::_ffmpegThreadCountCallback(int value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.ffmpegThreadCountSpinBox);
            p.ffmpegThreadCountSpinBox->setValue(value);
        }

        struct MiscSettingsWidget::Private
        {
            QCheckBox* toolTipsCheckBox = nullptr;
            SettingsObject* settingsObject = nullptr;
        };

        MiscSettingsWidget::MiscSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.settingsObject = settingsObject;

            p.toolTipsCheckBox = new QCheckBox;
            p.toolTipsCheckBox->setText(tr("Enable tool tips"));

            auto layout = new QVBoxLayout;
            layout->addWidget(p.toolTipsCheckBox);
            setLayout(layout);

            p.toolTipsCheckBox->setChecked(settingsObject->hasToolTipsEnabled());

            connect(
                p.toolTipsCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(_toolTipsCallback(int)));

            connect(
                settingsObject,
                SIGNAL(toolTipsEnabledChanged(bool)),
                SLOT(_toolTipsCallback(bool)));
        }

        MiscSettingsWidget::~MiscSettingsWidget()
        {}

        void MiscSettingsWidget::_toolTipsCallback(int value)
        {
            TLRENDER_P();
            p.settingsObject->setToolTipsEnabled(Qt::Checked == value);
        }

        void MiscSettingsWidget::_toolTipsCallback(bool value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.toolTipsCheckBox);
            p.toolTipsCheckBox->setChecked(value);
        }

        SettingsTool::SettingsTool(
            SettingsObject* settingsObject,
            qt::TimeObject* timeObject,
            QWidget* parent) :
            ToolWidget(parent)
        {
            addBellows(tr("Cache"), new CacheSettingsWidget(settingsObject));
            addBellows(tr("File Sequences"), new FileSequenceSettingsWidget(settingsObject));
            addBellows(tr("Performance"), new PerformanceSettingsWidget(settingsObject));
            addBellows(tr("Miscellaneous"), new MiscSettingsWidget(settingsObject));
            addStretch();
        }
    }
}
