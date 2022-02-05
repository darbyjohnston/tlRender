// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/TimeLabel.h>

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>

namespace tlr
{
    namespace qwidget
    {
        struct TimeLabel::Private
        {
            otime::RationalTime value = time::invalidTime;
            qt::TimeUnits units = qt::TimeUnits::Timecode;
            QLabel* label = nullptr;
            qt::TimeObject* timeObject = nullptr;
        };

        TimeLabel::TimeLabel(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            setFont(fixedFont);

            p.label = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.label);
            setLayout(layout);

            _textUpdate();
        }
        
        TimeLabel::~TimeLabel()
        {}

        void TimeLabel::setTimeObject(qt::TimeObject* timeObject)
        {
            TLR_PRIVATE_P();
            if (timeObject == p.timeObject)
                return;
            if (p.timeObject)
            {
                disconnect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tlr::qt::Time::Units)),
                    this,
                    SLOT(setUnits(tlr::qt::Time::Units)));
            }
            p.timeObject = timeObject;
            if (p.timeObject)
            {
                p.units = p.timeObject->units();
                connect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tlr::qt::TimeUnits)),
                    SLOT(setUnits(tlr::qt::TimeUnits)));
            }
            _textUpdate();
            updateGeometry();
        }

        const otime::RationalTime& TimeLabel::value() const
        {
            return _p->value;
        }

        qt::TimeUnits TimeLabel::units() const
        {
            return _p->units;
        }

        void TimeLabel::setValue(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            if (value.value() == p.value.value() &&
                value.rate() == p.value.rate())
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeLabel::setUnits(qt::TimeUnits units)
        {
            TLR_PRIVATE_P();
            if (units == p.units)
                return;
            p.units = units;
            _textUpdate();
            updateGeometry();
        }

        void TimeLabel::_textUpdate()
        {
            TLR_PRIVATE_P();
            p.label->setText(timeToText(p.value, p.units));
        }
    }
}
