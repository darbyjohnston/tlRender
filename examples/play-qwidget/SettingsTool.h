// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "ToolWidget.h"
#include "SettingsObject.h"

#include <tlrQWidget/RadioButtonGroup.h>

#include <tlrQt/TimeObject.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMap>
#include <QSpinBox>

namespace tlr
{
    //! Cache settings widget.
    class CacheSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        CacheSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _readAheadCallback(double);
        void _readBehindCallback(double);

    private:
        QDoubleSpinBox* _readAheadSpinBox = nullptr;
        QDoubleSpinBox* _readBehindSpinBox = nullptr;
    };

    //! File sequence settings widget.
    class FileSequenceSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        FileSequenceSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _audioCallback(int);
        void _audioCallback(tlr::timeline::FileSequenceAudio);
        void _audioFileNameCallback(const QString&);
        void _audioDirectoryCallback(const QString&);
        void _maxDigitsCallback(int);

    private:
        SettingsObject* _settingsObject = nullptr;
        QComboBox* _audioComboBox = nullptr;
        QLineEdit* _audioFileName = nullptr;
        QLineEdit* _audioDirectory = nullptr;
        QSpinBox* _maxDigitsSpinBox = nullptr;
    };

    //! Performance settings widget.
    class PerformanceSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        PerformanceSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _timerModeCallback(int);
        void _timerModeCallback(tlr::timeline::TimerMode);
        void _audioBufferFrameCountCallback(int);
        void _audioBufferFrameCountCallback(tlr::timeline::AudioBufferFrameCount);
        void _videoRequestCountCallback(int);
        void _audioRequestCountCallback(int);
        void _sequenceThreadCountCallback(int);
        void _ffmpegThreadCountCallback(int);

    private:
        SettingsObject* _settingsObject = nullptr;
        QComboBox* _timerModeComboBox = nullptr;
        QComboBox * _audioBufferFrameCountComboBox = nullptr;
        QSpinBox* _videoRequestCountSpinBox = nullptr;
        QSpinBox* _audioRequestCountSpinBox = nullptr;
        QSpinBox* _sequenceThreadCountSpinBox = nullptr;
        QSpinBox* _ffmpegThreadCountSpinBox = nullptr;
    };

    //! Time settings widget.
    class TimeSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        TimeSettingsWidget(qt::TimeObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _unitsCallback(const QVariant&);
        void _unitsCallback(tlr::qt::TimeUnits);

    private:
        qwidget::RadioButtonGroup* _unitsButtonGroup = nullptr;
        qt::TimeObject* _timeObject = nullptr;
    };

    //! Miscellaneous settings widget.
    class MiscSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        MiscSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _toolTipsCallback(int);
        void _toolTipsCallback(bool);

    private:
        QCheckBox* _toolTipsCheckBox = nullptr;
        SettingsObject* _settingsObject = nullptr;
    };

    //! Settings tool.
    class SettingsTool : public ToolWidget
    {
        Q_OBJECT

    public:
        SettingsTool(
            SettingsObject*,
            qt::TimeObject*,
            QWidget* parent = nullptr);
    };
}
