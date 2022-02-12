// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/SettingsTool.h>

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>

namespace tl
{
    namespace play
    {
        CacheSettingsWidget::CacheSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent)
        {
            _readAheadSpinBox = new QDoubleSpinBox;
            _readAheadSpinBox->setRange(0.0, 60.0);

            _readBehindSpinBox = new QDoubleSpinBox;
            _readBehindSpinBox->setRange(0, 60.0);

            auto layout = new QVBoxLayout;
            layout->addWidget(new QLabel(tr("Read Ahead")));
            layout->addWidget(_readAheadSpinBox);
            layout->addWidget(new QLabel(tr("Read Behind")));
            layout->addWidget(_readBehindSpinBox);
            setLayout(layout);

            _readAheadSpinBox->setValue(settingsObject->cacheReadAhead());
            _readBehindSpinBox->setValue(settingsObject->cacheReadBehind());

            connect(
                _readAheadSpinBox,
                SIGNAL(valueChanged(double)),
                settingsObject,
                SLOT(setCacheReadAhead(double)));

            connect(
                _readBehindSpinBox,
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

        void CacheSettingsWidget::_readAheadCallback(double value)
        {
            QSignalBlocker signalBlocker(_readAheadSpinBox);
            _readAheadSpinBox->setValue(value);
        }

        void CacheSettingsWidget::_readBehindCallback(double value)
        {
            QSignalBlocker signalBlocker(_readBehindSpinBox);
            _readBehindSpinBox->setValue(value);
        }

        FileSequenceSettingsWidget::FileSequenceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _settingsObject(settingsObject)
        {
            _audioComboBox = new QComboBox;
            for (const auto& i : timeline::getFileSequenceAudioLabels())
            {
                _audioComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            _audioFileName = new QLineEdit;

            _audioDirectory = new QLineEdit;

            _maxDigitsSpinBox = new QSpinBox;
            _maxDigitsSpinBox->setRange(0, 255);

            auto layout = new QVBoxLayout;
            layout->addWidget(new QLabel(tr("Audio")));
            layout->addWidget(_audioComboBox);
            layout->addWidget(new QLabel(tr("Audio File Name")));
            layout->addWidget(_audioFileName);
            layout->addWidget(new QLabel(tr("Audio Directory")));
            layout->addWidget(_audioDirectory);
            layout->addWidget(new QLabel(tr("Maximum Digits")));
            layout->addWidget(_maxDigitsSpinBox);
            setLayout(layout);

            _audioComboBox->setCurrentIndex(static_cast<int>(settingsObject->fileSequenceAudio()));
            _audioFileName->setText(settingsObject->fileSequenceAudioFileName());
            _audioDirectory->setText(settingsObject->fileSequenceAudioDirectory());
            _maxDigitsSpinBox->setValue(static_cast<int>(settingsObject->maxFileSequenceDigits()));

            connect(
                _audioComboBox,
                SIGNAL(activated(int)),
                SLOT(_audioCallback(int)));

            connect(
                _audioFileName,
                SIGNAL(textChanged(const QString&)),
                settingsObject,
                SLOT(setFileSequenceAudioFileName(const QString&)));

            connect(
                _audioDirectory,
                SIGNAL(textChanged(const QString&)),
                settingsObject,
                SLOT(setFileSequenceAudioDirectory(const QString&)));

            connect(
                _maxDigitsSpinBox,
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

        void FileSequenceSettingsWidget::_audioCallback(int value)
        {
            _settingsObject->setFileSequenceAudio(static_cast<timeline::FileSequenceAudio>(value));
        }

        void FileSequenceSettingsWidget::_audioCallback(timeline::FileSequenceAudio value)
        {
            QSignalBlocker signalBlocker(_audioComboBox);
            _audioComboBox->setCurrentIndex(static_cast<int>(value));
        }

        void FileSequenceSettingsWidget::_audioFileNameCallback(const QString& value)
        {
            QSignalBlocker signalBlocker(_audioFileName);
            _audioFileName->setText(value);
        }

        void FileSequenceSettingsWidget::_audioDirectoryCallback(const QString& value)
        {
            QSignalBlocker signalBlocker(_audioDirectory);
            _audioDirectory->setText(value);
        }

        void FileSequenceSettingsWidget::_maxDigitsCallback(int value)
        {
            QSignalBlocker signalBlocker(_maxDigitsSpinBox);
            _maxDigitsSpinBox->setValue(value);
        }

        PerformanceSettingsWidget::PerformanceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent)
        {
            _settingsObject = settingsObject;

            _timerModeComboBox = new QComboBox;
            for (const auto& i : timeline::getTimerModeLabels())
            {
                _timerModeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            _audioBufferFrameCountComboBox = new QComboBox;
            for (const auto& i : timeline::getAudioBufferFrameCountLabels())
            {
                _audioBufferFrameCountComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            _videoRequestCountSpinBox = new QSpinBox;
            _videoRequestCountSpinBox->setRange(1, 64);

            _audioRequestCountSpinBox = new QSpinBox;
            _audioRequestCountSpinBox->setRange(1, 64);

            _sequenceThreadCountSpinBox = new QSpinBox;
            _sequenceThreadCountSpinBox->setRange(1, 64);

            _ffmpegThreadCountSpinBox = new QSpinBox;
            _ffmpegThreadCountSpinBox->setRange(1, 64);

            auto layout = new QVBoxLayout;
            auto label = new QLabel(tr("Changes are applied to newly opened files."));
            label->setWordWrap(true);
            layout->addWidget(label);
            layout->addWidget(new QLabel(tr("Timer Mode")));
            layout->addWidget(_timerModeComboBox);
            layout->addWidget(new QLabel(tr("Audio Buffer Frame Count")));
            layout->addWidget(_audioBufferFrameCountComboBox);
            layout->addWidget(new QLabel(tr("Timeline Video Requests")));
            layout->addWidget(_videoRequestCountSpinBox);
            layout->addWidget(new QLabel(tr("Timeline Audio Requests")));
            layout->addWidget(_audioRequestCountSpinBox);
            layout->addWidget(new QLabel(tr("Sequence I/O Threads")));
            layout->addWidget(_sequenceThreadCountSpinBox);
            layout->addWidget(new QLabel(tr("FFmpeg I/O threads")));
            layout->addWidget(_ffmpegThreadCountSpinBox);
            setLayout(layout);

            _timerModeComboBox->setCurrentIndex(static_cast<int>(settingsObject->timerMode()));
            _audioBufferFrameCountComboBox->setCurrentIndex(static_cast<int>(settingsObject->audioBufferFrameCount()));
            _videoRequestCountSpinBox->setValue(settingsObject->videoRequestCount());
            _audioRequestCountSpinBox->setValue(settingsObject->audioRequestCount());
            _sequenceThreadCountSpinBox->setValue(settingsObject->sequenceThreadCount());
            _ffmpegThreadCountSpinBox->setValue(settingsObject->ffmpegThreadCount());

            connect(
                _timerModeComboBox,
                SIGNAL(activated(int)),
                SLOT(_timerModeCallback(int)));

            connect(
                _audioBufferFrameCountComboBox,
                SIGNAL(activated(int)),
                SLOT(_audioBufferFrameCountCallback(int)));

            connect(
                _videoRequestCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setVideoRequestCount(int)));

            connect(
                _audioRequestCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setAudioRequestCount(int)));

            connect(
                _sequenceThreadCountSpinBox,
                SIGNAL(valueChanged(int)),
                settingsObject,
                SLOT(setSequenceThreadCount(int)));

            connect(
                _ffmpegThreadCountSpinBox,
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

        void PerformanceSettingsWidget::_timerModeCallback(int value)
        {
            _settingsObject->setTimerMode(static_cast<timeline::TimerMode>(value));
        }

        void PerformanceSettingsWidget::_timerModeCallback(timeline::TimerMode value)
        {
            QSignalBlocker signalBlocker(_timerModeComboBox);
            _timerModeComboBox->setCurrentIndex(static_cast<int>(value));
        }

        void PerformanceSettingsWidget::_audioBufferFrameCountCallback(int value)
        {
            _settingsObject->setAudioBufferFrameCount(static_cast<timeline::AudioBufferFrameCount>(value));
        }

        void PerformanceSettingsWidget::_audioBufferFrameCountCallback(timeline::AudioBufferFrameCount value)
        {
            QSignalBlocker signalBlocker(_audioRequestCountSpinBox);
            _audioBufferFrameCountComboBox->setCurrentIndex(static_cast<int>(value));
        }

        void PerformanceSettingsWidget::_videoRequestCountCallback(int value)
        {
            QSignalBlocker signalBlocker(_videoRequestCountSpinBox);
            _videoRequestCountSpinBox->setValue(value);
        }

        void PerformanceSettingsWidget::_audioRequestCountCallback(int value)
        {
            QSignalBlocker signalBlocker(_audioRequestCountSpinBox);
            _audioRequestCountSpinBox->setValue(value);
        }

        void PerformanceSettingsWidget::_sequenceThreadCountCallback(int value)
        {
            QSignalBlocker signalBlocker(_sequenceThreadCountSpinBox);
            _sequenceThreadCountSpinBox->setValue(value);
        }

        void PerformanceSettingsWidget::_ffmpegThreadCountCallback(int value)
        {
            QSignalBlocker signalBlocker(_ffmpegThreadCountSpinBox);
            _ffmpegThreadCountSpinBox->setValue(value);
        }

        TimeSettingsWidget::TimeSettingsWidget(qt::TimeObject* timeObject, QWidget* parent) :
            QWidget(parent),
            _timeObject(timeObject)
        {
            _unitsButtonGroup = new qwidget::RadioButtonGroup;
            for (const auto i : qt::getTimeUnitsEnums())
            {
                _unitsButtonGroup->addButton(
                    QString::fromUtf8(qt::getLabel(i).c_str()),
                    QVariant::fromValue<qt::TimeUnits>(i));
            }

            auto layout = new QVBoxLayout;
            layout->addWidget(new QLabel(tr("Units")));
            layout->addWidget(_unitsButtonGroup);
            setLayout(layout);

            _unitsButtonGroup->setChecked(QVariant::fromValue<qt::TimeUnits>(_timeObject->units()));

            connect(
                _unitsButtonGroup,
                SIGNAL(checked(const QVariant&)),
                SLOT(_unitsCallback(const QVariant&)));

            connect(
                _timeObject,
                SIGNAL(unitsChanged(tl::qt::TimeUnits)),
                SLOT(_unitsCallback(tl::qt::TimeUnits)));
        }

        void TimeSettingsWidget::_unitsCallback(const QVariant& value)
        {
            _timeObject->setUnits(value.value<qt::TimeUnits>());
        }

        void TimeSettingsWidget::_unitsCallback(qt::TimeUnits value)
        {
            const QSignalBlocker blocker(_unitsButtonGroup);
            _unitsButtonGroup->setChecked(QVariant::fromValue<qt::TimeUnits>(value));
        }

        MiscSettingsWidget::MiscSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _settingsObject(settingsObject)
        {
            _toolTipsCheckBox = new QCheckBox;
            _toolTipsCheckBox->setText(tr("Enable tool tips"));

            auto layout = new QVBoxLayout;
            layout->addWidget(_toolTipsCheckBox);
            setLayout(layout);

            _toolTipsCheckBox->setChecked(settingsObject->hasToolTipsEnabled());

            connect(
                _toolTipsCheckBox,
                SIGNAL(stateChanged(int)),
                SLOT(_toolTipsCallback(int)));

            connect(
                settingsObject,
                SIGNAL(toolTipsEnabledChanged(bool)),
                SLOT(_toolTipsCallback(bool)));
        }

        void MiscSettingsWidget::_toolTipsCallback(int value)
        {
            _settingsObject->setToolTipsEnabled(Qt::Checked == value);
        }

        void MiscSettingsWidget::_toolTipsCallback(bool value)
        {
            QSignalBlocker signalBlocker(_toolTipsCheckBox);
            _toolTipsCheckBox->setChecked(value);
        }

        SettingsTool::SettingsTool(
            SettingsObject* settingsObject,
            qt::TimeObject* timeObject,
            QWidget* parent) :
            ToolWidget(parent)
        {
            auto cacheSettingsWidget = new CacheSettingsWidget(settingsObject);
            auto fileSequenceSettingsWidget = new FileSequenceSettingsWidget(settingsObject);
            auto performanceSettingsWidget = new PerformanceSettingsWidget(settingsObject);
            auto timeSettingsWidget = new TimeSettingsWidget(timeObject);
            auto miscSettingsWidget = new MiscSettingsWidget(settingsObject);

            addBellows(tr("Cache"), cacheSettingsWidget);
            addBellows(tr("File Sequences"), fileSequenceSettingsWidget);
            addBellows(tr("Performance"), performanceSettingsWidget);
            addBellows(tr("Time"), timeSettingsWidget);
            addBellows(tr("Miscellaneous"), miscSettingsWidget);
            addStretch();
        }
    }
}
