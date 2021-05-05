// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimeLabel.h>

#include <QFontDatabase>
#include <QHBoxLayout>

namespace tlr
{
    namespace qt
    {
        TimeLabel::TimeLabel(QWidget* parent) :
            QWidget(parent)
        {
            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            setFont(fixedFont);

            _label = new QLabel;

            auto layout = new QHBoxLayout;
            layout->addWidget(_label);
            setLayout(layout);

            _textUpdate();
        }

        void TimeLabel::setTimeObject(TimeObject* timeObject)
        {
            if (timeObject == _timeObject)
                return;
            if (_timeObject)
            {
                disconnect(
                    _timeObject,
                    SIGNAL(unitsChanged(qt::Time::Units)),
                    this,
                    SLOT(setUnits(qt::Time::Units)));
            }
            _timeObject = timeObject;
            if (_timeObject)
            {
                _units = _timeObject->units();
                connect(
                    _timeObject,
                    SIGNAL(unitsChanged(qt::TimeObject::Units)),
                    SLOT(setUnits(qt::TimeObject::Units)));
            }
            _textUpdate();
        }

        void TimeLabel::setValue(const otime::RationalTime& value)
        {
            if (_value == value)
                return;
            _value = value;
            _textUpdate();
        }

        void TimeLabel::setUnits(TimeObject::Units units)
        {
            if (_units == units)
                return;
            _units = units;
            _textUpdate();
        }

        void TimeLabel::_textUpdate()
        {
            _label->setText(TimeObject::timeToText(_value, _units));
        }
    }
}
