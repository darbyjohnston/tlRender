// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQWidget/SpeedLabel.h>

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>

namespace tlr
{
    namespace qwidget
    {
        struct SpeedLabel::Private
        {
            otime::RationalTime value = time::invalidTime;
            QLabel* label = nullptr;
        };

        SpeedLabel::SpeedLabel(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            setFont(fixedFont);

            p.label = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(p.label);
            setLayout(layout);

            _textUpdate();
        }
        
        SpeedLabel::~SpeedLabel()
        {}

        void SpeedLabel::setValue(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            if (p.value.value() == value.value() &&
                p.value.rate() == value.rate())
                return;
            p.value = value;
            _textUpdate();
        }

        void SpeedLabel::_textUpdate()
        {
            TLR_PRIVATE_P();
            p.label->setText(QString("%1").arg(p.value.rate(), 0, 'f', 2));
        }
    }
}
