// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "SettingsWidget.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>

namespace tlr
{
    CacheSettingsWidget::CacheSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
        QWidget(parent)
    {
        _readAheadSpinBox = new QDoubleSpinBox;
        _readAheadSpinBox->setRange(0.0, 60.0);

        _readBehindSpinBox = new QDoubleSpinBox;
        _readBehindSpinBox->setRange(0, 60.0);

        auto layout = new QVBoxLayout;
        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(_readAheadSpinBox);
        auto groupBox = new QGroupBox(tr("Read Ahead"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);
        vLayout = new QVBoxLayout;
        vLayout->addWidget(_readBehindSpinBox);
        groupBox = new QGroupBox(tr("Read Behind"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);
        layout->addStretch();
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

    AudioSettingsWidget::AudioSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
        QWidget(parent),
        _settingsObject(settingsObject)
    {
        _separateAudioComboBox = new QComboBox;
        for (const auto& i : timeline::getSeparateAudioLabels())
        {
            _separateAudioComboBox->addItem(i.c_str());
        }

        _separateAudioFileName = new QLineEdit;

        _separateAudioDirectory = new QLineEdit;

        auto layout = new QVBoxLayout;
        auto vLayout = new QVBoxLayout;
        auto label = new QLabel(tr("Separate audio for file sequences."));
        label->setWordWrap(true);
        vLayout->addWidget(label);
        vLayout->addWidget(_separateAudioComboBox);
        auto formLayout = new QFormLayout;
        formLayout->addRow(tr("File name:"), _separateAudioFileName);
        formLayout->addRow(tr("Directory:"), _separateAudioDirectory);
        vLayout->addLayout(formLayout);
        auto groupBox = new QGroupBox(tr("Separate Audio"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);
        layout->addStretch();
        setLayout(layout);

        _separateAudioComboBox->setCurrentIndex(static_cast<int>(settingsObject->separateAudio()));
        _separateAudioFileName->setText(settingsObject->separateAudioFileName());
        _separateAudioDirectory->setText(settingsObject->separateAudioDirectory());

        connect(
            _separateAudioComboBox,
            SIGNAL(activated(int)),
            SLOT(_separateAudioCallback(int)));

        connect(
            _separateAudioFileName,
            SIGNAL(textChanged(const QString&)),
            settingsObject,
            SLOT(setSeparateAudioFileName(const QString&)));

        connect(
            _separateAudioDirectory,
            SIGNAL(textChanged(const QString&)),
            settingsObject,
            SLOT(setSeparateAudioDirectory(const QString&)));

        connect(
            settingsObject,
            SIGNAL(separateAudioChanged(tlr::timeline::SeparateAudio)),
            SLOT(_separateAudioCallback(tlr::timeline::SeparateAudio)));
        connect(
            settingsObject,
            SIGNAL(separateAudioFileNameChanged(const QString&)),
            SLOT(_separateAudioFileNameCallback(const QString&)));
        connect(
            settingsObject,
            SIGNAL(separateAudioDirectoryChanged(const QString&)),
            SLOT(_separateAudioDirectoryCallback(const QString&)));
    }

    void AudioSettingsWidget::_separateAudioCallback(int value)
    {
        _settingsObject->setSeparateAudio(static_cast<timeline::SeparateAudio>(value));
    }

    void AudioSettingsWidget::_separateAudioCallback(timeline::SeparateAudio value)
    {
        QSignalBlocker signalBlocker(_separateAudioComboBox);
        _separateAudioComboBox->setCurrentIndex(static_cast<int>(value));
    }

    void AudioSettingsWidget::_separateAudioFileNameCallback(const QString& value)
    {
        _separateAudioFileName->setText(value);
    }

    void AudioSettingsWidget::_separateAudioDirectoryCallback(const QString& value)
    {
        _separateAudioDirectory->setText(value);
    }

    PerformanceSettingsWidget::PerformanceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
        QWidget(parent)
    {
        _settingsObject = settingsObject;

        _timerModeComboBox = new QComboBox;
        for (const auto& i : timeline::getTimerModeLabels())
        {
            _timerModeComboBox->addItem(i.c_str());
        }

        _audioBufferFrameCountComboBox = new QComboBox;
        for (const auto& i : timeline::getAudioBufferFrameCountLabels())
        {
            _audioBufferFrameCountComboBox->addItem(i.c_str());
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
        layout->addSpacing(10);

        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(_timerModeComboBox);
        auto groupBox = new QGroupBox(tr("Timer Mode"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);

        vLayout = new QVBoxLayout;
        vLayout->addWidget(_audioBufferFrameCountComboBox);
        groupBox = new QGroupBox(tr("Audio Buffer Frame Count"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);

        vLayout = new QVBoxLayout;
        vLayout->addWidget(_videoRequestCountSpinBox);
        groupBox = new QGroupBox(tr("Timeline Video Requests"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);

        vLayout = new QVBoxLayout;
        vLayout->addWidget(_audioRequestCountSpinBox);
        groupBox = new QGroupBox(tr("Timeline Audio Requests"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);

        vLayout = new QVBoxLayout;
        vLayout->addWidget(_sequenceThreadCountSpinBox);
        groupBox = new QGroupBox(tr("Sequence I/O Threads"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);

        vLayout = new QVBoxLayout;
        vLayout->addWidget(_ffmpegThreadCountSpinBox);
        groupBox = new QGroupBox(tr("FFmpeg I/O threads"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);

        layout->addStretch();
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
            SIGNAL(timerModeChanged(tlr::timeline::TimerMode)),
            SLOT(_timerModeCallback(tlr::timeline::TimerMode)));
        connect(
            settingsObject,
            SIGNAL(audioBufferFrameCountChanged(tlr::timeline::AudioBufferFrameCount)),
            SLOT(_audioBufferFrameCountCallback(tlr::timeline::AudioBufferFrameCount)));
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
        auto framesButton = new QRadioButton;
        framesButton->setText(tr("Frames"));
        auto secondsButton = new QRadioButton;
        secondsButton->setText(tr("Seconds"));
        auto timecodeButton = new QRadioButton;
        timecodeButton->setText(tr("Timecode"));
        _unitsButtonGroup = new QButtonGroup(this);
        _unitsButtonGroup->setExclusive(true);
        _unitsButtonGroup->addButton(framesButton);
        _unitsButtonGroup->addButton(secondsButton);
        _unitsButtonGroup->addButton(timecodeButton);
        _buttonToUnits[framesButton] = qt::TimeUnits::Frames;
        _buttonToUnits[secondsButton] = qt::TimeUnits::Seconds;
        _buttonToUnits[timecodeButton] = qt::TimeUnits::Timecode;
        _unitsToButtons[qt::TimeUnits::Frames] = framesButton;
        _unitsToButtons[qt::TimeUnits::Seconds] = secondsButton;
        _unitsToButtons[qt::TimeUnits::Timecode] = timecodeButton;

        auto layout = new QVBoxLayout;
        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(framesButton);
        vLayout->addWidget(secondsButton);
        vLayout->addWidget(timecodeButton);
        auto groupBox = new QGroupBox(tr("Units"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);
        layout->addStretch();
        setLayout(layout);

        const auto unitsButton = _unitsToButtons.find(_timeObject->units());
        if (unitsButton != _unitsToButtons.end())
        {
            unitsButton.value()->setChecked(true);
        }

        connect(
            _unitsButtonGroup,
            SIGNAL(buttonClicked(QAbstractButton*)),
            SLOT(_unitsCallback(QAbstractButton*)));

        connect(
            _timeObject,
            SIGNAL(unitsChanged(tlr::qt::TimeUnits)),
            SLOT(_unitsCallback(tlr::qt::TimeUnits)));
    }

    void TimeSettingsWidget::_unitsCallback(QAbstractButton* button)
    {
        const auto i = _buttonToUnits.find(button);
        if (i != _buttonToUnits.end())
        {
            _timeObject->setUnits(i.value());
        }
    }

    void TimeSettingsWidget::_unitsCallback(qt::TimeUnits units)
    {
        const QSignalBlocker blocker(_unitsButtonGroup);
        const auto i = _unitsToButtons.find(units);
        if (i != _unitsToButtons.end())
        {
            i.value()->setChecked(true);
        }
    }

    MiscSettingsWidget::MiscSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
        QWidget(parent),
        _settingsObject(settingsObject)
    {
        _toolTipsCheckBox = new QCheckBox;
        _toolTipsCheckBox->setText(tr("Enable tool tips"));

        auto layout = new QVBoxLayout;
        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(_toolTipsCheckBox);
        auto groupBox = new QGroupBox(tr("Tool Tips"));
        groupBox->setLayout(vLayout);
        layout->addWidget(groupBox);
        layout->addStretch();
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

    SettingsWidget::SettingsWidget(
        SettingsObject* settingsObject,
        qt::TimeObject* timeObject,
        QWidget* parent) :
        QToolBox(parent)
    {
        addItem(new CacheSettingsWidget(settingsObject), tr("Cache"));
        addItem(new AudioSettingsWidget(settingsObject), tr("Audio"));
        addItem(new PerformanceSettingsWidget(settingsObject), tr("Performance"));
        addItem(new TimeSettingsWidget(timeObject), tr("Time"));
        addItem(new MiscSettingsWidget(settingsObject), tr("Miscellaneous"));

        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("Settings/CurrentItem").toInt());
    }

    void SettingsWidget::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("Settings/CurrentItem", value);
    }
}
