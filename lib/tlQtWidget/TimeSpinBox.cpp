// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimeSpinBox.h>

#include <QApplication>
#include <QFontDatabase>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QStyleOptionSpinBox>

namespace tl
{
    namespace qtwidget
    {
        struct TimeSpinBox::Private
        {
            otime::RationalTime value = time::invalidTime;
            qt::TimeUnits units = qt::TimeUnits::Timecode;
            QRegularExpressionValidator* validator = nullptr;
            qt::TimeObject* timeObject = nullptr;
        };

        TimeSpinBox::TimeSpinBox(QWidget* parent) :
            QAbstractSpinBox(parent),
            _p(new Private)
        {
            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            setFont(fixedFont);

            _vaidatorUpdate();
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

        TimeSpinBox::~TimeSpinBox()
        {}

        void TimeSpinBox::setTimeObject(qt::TimeObject* timeObject)
        {
            TLRENDER_P();
            if (timeObject == p.timeObject)
                return;
            if (p.timeObject)
            {
                disconnect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tl::qt::TimeUnits)),
                    this,
                    SLOT(setUnits(tl::qt::TimeUnits)));
            }
            p.timeObject = timeObject;
            if (p.timeObject)
            {
                p.units = p.timeObject->units();
                connect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tl::qt::TimeUnits)),
                    SLOT(setUnits(tl::qt::TimeUnits)));
            }
            _vaidatorUpdate();
            _textUpdate();
            updateGeometry();
        }

        const otime::RationalTime& TimeSpinBox::value() const
        {
            return _p->value;
        }

        qt::TimeUnits TimeSpinBox::units() const
        {
            return _p->units;
        }

        void TimeSpinBox::stepBy(int steps)
        {
            TLRENDER_P();
            p.value += otime::RationalTime(steps, p.value.rate());
            Q_EMIT valueChanged(p.value);
            _textUpdate();
        }

        QValidator::State TimeSpinBox::validate(QString&, int& pos) const
        {
            return QValidator::Acceptable;
        }

        void TimeSpinBox::setValue(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (value.value() == p.value.value() &&
                value.rate() == p.value.rate())
                return;
            p.value = value;
            Q_EMIT valueChanged(p.value);
            _textUpdate();
        }

        void TimeSpinBox::setUnits(qt::TimeUnits units)
        {
            TLRENDER_P();
            if (units == p.units)
                return;
            p.units = units;
            Q_EMIT unitsChanged(p.units);
            _vaidatorUpdate();
            _textUpdate();
            updateGeometry();
        }

        QAbstractSpinBox::StepEnabled TimeSpinBox::stepEnabled() const
        {
            return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
        }

        QSize TimeSpinBox::minimumSizeHint() const
        {
            TLRENDER_P();
            //! \todo Cache the size hint.
            ensurePolished();
            int h = lineEdit()->minimumSizeHint().height();
            const QFontMetrics fm(fontMetrics());
            int w = fm.horizontalAdvance(" " + sizeHintString(p.units));
            w += 2; // cursor blinking space
            QStyleOptionSpinBox opt;
            initStyleOption(&opt);
            QSize hint(w, h);
            return style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this);
        }

        void TimeSpinBox::_lineEditCallback()
        {
            TLRENDER_P();
            otime::ErrorStatus errorStatus;
            const auto time = textToTime(lineEdit()->text(), p.value.rate(), p.units, &errorStatus);
            if (otime::ErrorStatus::OK == errorStatus && time != p.value)
            {
                p.value = time;
                Q_EMIT valueChanged(p.value);
            }
            _textUpdate();
        }

        void TimeSpinBox::_vaidatorUpdate()
        {
            TLRENDER_P();
            if (p.validator)
            {
                p.validator->setParent(nullptr);
            }
            p.validator = new QRegularExpressionValidator(QRegularExpression(validator(p.units)), this);
            lineEdit()->setValidator(p.validator);
        }

        void TimeSpinBox::_textUpdate()
        {
            TLRENDER_P();
            lineEdit()->setText(timeToText(p.value, p.units));
        }
    }
}
