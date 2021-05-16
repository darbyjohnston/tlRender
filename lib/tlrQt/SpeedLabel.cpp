// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/SpeedLabel.h>

#include <QFontDatabase>
#include <QHBoxLayout>

namespace tlr
{
    namespace qt
    {
        SpeedLabel::SpeedLabel(QWidget* parent) :
            QWidget(parent)
        {
            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            setFont(fixedFont);

            _label = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(_label);
            setLayout(layout);

            _textUpdate();
        }

        void SpeedLabel::setValue(const otime::RationalTime& value)
        {
            if (_value == value)
                return;
            _value = value;
            _textUpdate();
        }

        void SpeedLabel::_textUpdate()
        {
            _label->setText(QString("%1").arg(_value.rate(), 0, 'f', 2));
        }
    }
}
