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

            //! \todo How 
            //setMinimumWidth(100);

            auto validator = new QRegExpValidator(QRegExp("[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]"), this);
            lineEdit()->setValidator(validator);

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

        void TimeSpinBox::fixup(QString& value) const
        {
        }

        void TimeSpinBox::stepBy(int steps)
        {
            _time += otime::RationalTime(steps, _time.rate());
            Q_EMIT valueChanged(_time);
            _textUpdate();
        }

        QValidator::State TimeSpinBox::validate(QString&, int& pos) const
        {
            return QValidator::Acceptable;
        }

        void TimeSpinBox::setValue(const otime::RationalTime& value)
        {
            if (_time == value)
                return;
            _time = value;
            Q_EMIT valueChanged(_time);
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
            int w = fm.horizontalAdvance(" 00:00:00:00");
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
            const auto time = otime::RationalTime::from_timecode(lineEdit()->text().toLatin1().data(), _time.rate(), &errorStatus);
            if (otime::ErrorStatus::OK == errorStatus && time != _time)
            {
                _time = time;
                Q_EMIT valueChanged(_time);
            }
            _textUpdate();
        }

        void TimeSpinBox::_textUpdate()
        {
            otime::ErrorStatus errorStatus;
            const auto s = _time.to_timecode(&errorStatus);
            lineEdit()->setText(s.c_str());
        }
    }
}
