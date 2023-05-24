// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimeSpinBox.h>

#include <tlQtWidget/Util.h>

#include <QApplication>
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
            timeline::TimeUnits timeUnits = timeline::TimeUnits::Timecode;
            QRegularExpressionValidator* validator = nullptr;
            qt::TimeObject* timeObject = nullptr;
        };

        TimeSpinBox::TimeSpinBox(QWidget* parent) :
            QAbstractSpinBox(parent),
            _p(new Private)
        {
            const QFont fixedFont("Noto Mono");
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
                    SIGNAL(timeUnitsChanged(tl::timeline::TimeUnits)),
                    this,
                    SLOT(setTimeUnits(tl::timeline::TimeUnits)));
            }
            p.timeObject = timeObject;
            if (p.timeObject)
            {
                p.timeUnits = p.timeObject->timeUnits();
                connect(
                    p.timeObject,
                    SIGNAL(timeUnitsChanged(tl::timeline::TimeUnits)),
                    SLOT(setTimeUnits(tl::timeline::TimeUnits)));
            }
            _vaidatorUpdate();
            _textUpdate();
            updateGeometry();
        }

        const otime::RationalTime& TimeSpinBox::value() const
        {
            return _p->value;
        }

        timeline::TimeUnits TimeSpinBox::timeUnits() const
        {
            return _p->timeUnits;
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

        void TimeSpinBox::setTimeUnits(timeline::TimeUnits value)
        {
            TLRENDER_P();
            if (value == p.timeUnits)
                return;
            p.timeUnits = value;
            Q_EMIT timeUnitsChanged(p.timeUnits);
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
            int w = fm.horizontalAdvance(" " + qt::sizeHintString(p.timeUnits));
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
            const auto time = qt::textToTime(lineEdit()->text(), p.value.rate(), p.timeUnits, &errorStatus);
            if (!otime::is_error(errorStatus) && time != p.value)
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
            p.validator = new QRegularExpressionValidator(QRegularExpression(qt::validator(p.timeUnits)), this);
            lineEdit()->setValidator(p.validator);
        }

        void TimeSpinBox::_textUpdate()
        {
            TLRENDER_P();
            lineEdit()->setText(qt::timeToText(p.value, p.timeUnits));
        }
    }
}
