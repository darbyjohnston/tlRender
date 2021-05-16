// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "SettingsWidget.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QSettings>

namespace tlr
{
    FrameCacheSettingsWidget::FrameCacheSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
        QWidget(parent)
    {
        _readAheadSpinBox = new QSpinBox;
        _readAheadSpinBox->setRange(0, 1000);

        _readBehindSpinBox = new QSpinBox;
        _readBehindSpinBox->setRange(0, 1000);

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

        _readAheadSpinBox->setValue(settingsObject->frameCacheReadAhead());
        _readBehindSpinBox->setValue(settingsObject->frameCacheReadBehind());

        connect(
            _readAheadSpinBox,
            SIGNAL(valueChanged(int)),
            settingsObject,
            SLOT(setFrameCacheReadAhead(int)));

        connect(
            _readBehindSpinBox,
            SIGNAL(valueChanged(int)),
            settingsObject,
            SLOT(setFrameCacheReadBehind(int)));

        connect(
            settingsObject,
            SIGNAL(frameCacheReadAheadChanged(int)),
            SLOT(_readAheadCallback(int)));

        connect(
            settingsObject,
            SIGNAL(frameCacheReadBehindChanged(int)),
            SLOT(_readBehindCallback(int)));
    }

    void FrameCacheSettingsWidget::_readAheadCallback(int value)
    {
        QSignalBlocker signalBlocker(_readAheadSpinBox);
        _readAheadSpinBox->setValue(value);
    }

    void FrameCacheSettingsWidget::_readBehindCallback(int value)
    {
        QSignalBlocker signalBlocker(_readBehindSpinBox);
        _readBehindSpinBox->setValue(value);
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
        _buttonToUnits[framesButton] = qt::TimeObject::Units::Frames;
        _buttonToUnits[secondsButton] = qt::TimeObject::Units::Seconds;
        _buttonToUnits[timecodeButton] = qt::TimeObject::Units::Timecode;
        _unitsToButtons[qt::TimeObject::Units::Frames] = framesButton;
        _unitsToButtons[qt::TimeObject::Units::Seconds] = secondsButton;
        _unitsToButtons[qt::TimeObject::Units::Timecode] = timecodeButton;

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
            SIGNAL(unitsChanged(qt::TimeObject::Units)),
            SLOT(_unitsCallback(qt::TimeObject::Units)));
    }

    void TimeSettingsWidget::_unitsCallback(QAbstractButton* button)
    {
        const auto i = _buttonToUnits.find(button);
        if (i != _buttonToUnits.end())
        {
            _timeObject->setUnits(i.value());
        }
    }

    void TimeSettingsWidget::_unitsCallback(qt::TimeObject::Units units)
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
        addItem(new FrameCacheSettingsWidget(settingsObject), tr("Frame Cache"));
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
