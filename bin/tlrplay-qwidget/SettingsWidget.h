// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "SettingsObject.h"

#include <tlrQt/TimeObject.h>

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QMap>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolBox>
#include <QWidget>

namespace tlr
{
    //! Frame cache settings widget.
    class FrameCacheSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        FrameCacheSettingsWidget(SettingsObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _readAheadCallback(int);
        void _readBehindCallback(int);

    private:
        QSpinBox* _readAheadSpinBox = nullptr;
        QSpinBox* _readBehindSpinBox = nullptr;
    };

    //! Time settings widget.
    class TimeSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        TimeSettingsWidget(qt::TimeObject*, QWidget* parent = nullptr);

    private Q_SLOTS:
        void _unitsCallback(QAbstractButton*);
        void _unitsCallback(qt::TimeUnits);

    private:
        QButtonGroup* _unitsButtonGroup = nullptr;
        QMap<QAbstractButton*, qt::TimeUnits> _buttonToUnits;
        QMap<qt::TimeUnits, QAbstractButton*> _unitsToButtons;
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

    //! Settings widget.
    class SettingsWidget : public QToolBox
    {
        Q_OBJECT

    public:
        SettingsWidget(
            SettingsObject*,
            qt::TimeObject*,
            QWidget* parent = nullptr);

    private Q_SLOTS:
        void _currentItemCallback(int);
    };
}
