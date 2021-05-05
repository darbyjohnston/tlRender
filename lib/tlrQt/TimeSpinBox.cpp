// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimeSpinBox.h>

#include <QApplication>
#include <QFontDatabase>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QStyleOptionSpinBox>

namespace tlr
{
    namespace qt
    {
        TimeSpinBox::TimeSpinBox(QWidget* parent) :
            QAbstractSpinBox(parent)
        {
            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            setFont(fixedFont);

            _textUpdate();

            connect(
                lineEdit(),
                SIGNAL(returnPressed()),
                SLOT(_lineEditCallback()));
            connect(
                lineEdit(),
                SIGNAL(editingFinished()),
                SLOT(_lineEditCallback()));
        }

        void TimeSpinBox::setTimeObject(TimeObject* timeObject)
        {
            if (timeObject == _timeObject)
                return;
            if (_timeObject)
            {
                disconnect(
                    _timeObject,
                    SIGNAL(unitsChanged(qt::TimeObject::Units)),
                    this,
                    SLOT(setUnits(qt::TimeObject::Units)));
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

        const otime::RationalTime& TimeSpinBox::value() const
        {
            return _value;
        }

        TimeObject::Units TimeSpinBox::units() const
        {
            return _units;
        }

        void TimeSpinBox::stepBy(int steps)
        {
            _value += otime::RationalTime(steps, _value.rate());
            Q_EMIT valueChanged(_value);
            _textUpdate();
        }

        QValidator::State TimeSpinBox::validate(QString&, int& pos) const
        {
            return QValidator::Acceptable;
        }

        void TimeSpinBox::setValue(const otime::RationalTime& value)
        {
            if (_value == value)
                return;
            _value = value;
            Q_EMIT valueChanged(_value);
            _textUpdate();
        }

        void TimeSpinBox::setUnits(TimeObject::Units units)
        {
            if (_units == units)
                return;
            _units = units;
            Q_EMIT unitsChanged(_units);
            _textUpdate();
        }

        QAbstractSpinBox::StepEnabled TimeSpinBox::stepEnabled() const
        {
            return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
        }

        QSize TimeSpinBox::minimumSizeHint() const
        {
            //! \todo Cache the size hint.
            ensurePolished();
            int h = lineEdit()->minimumSizeHint().height();
            const QFontMetrics fm(fontMetrics());
            int w = fm.horizontalAdvance(" " + TimeObject::unitsSizeHintString(_units));
            w += 2; // cursor blinking space
            QStyleOptionSpinBox opt;
            initStyleOption(&opt);
            QSize hint(w, h);
            return style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
                .expandedTo(QApplication::globalStrut());
        }

        void TimeSpinBox::_lineEditCallback()
        {
            otime::ErrorStatus errorStatus;
            const auto time = TimeObject::textToTime(lineEdit()->text(), _value.rate(), _units, &errorStatus);
            if (otime::ErrorStatus::OK == errorStatus && time != _value)
            {
                _value = time;
                Q_EMIT valueChanged(_value);
            }
            _textUpdate();
        }

        void TimeSpinBox::_textUpdate()
        {
            if (_validator)
            {
                _validator->setParent(nullptr);
            }
            _validator = new QRegExpValidator(QRegExp(TimeObject::unitsValidator(_units)), this);
            lineEdit()->setValidator(_validator);
            lineEdit()->setText(TimeObject::timeToText(_value, _units));
        }
    }
}
