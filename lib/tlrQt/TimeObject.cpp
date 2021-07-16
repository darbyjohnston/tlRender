// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimeObject.h>

#include <QRegExpValidator>

namespace tlr
{
    namespace qt
    {
        QString sizeHintString(TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case TimeUnits::Frames:
                out = "000000";
                break;
            case TimeUnits::Seconds:
                out = "000000.00";
                break;
            case TimeUnits::Timecode:
                out = "00:00:00:00";
                break;
            default: break;
            }
            return out;
        }

        QString validator(TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case TimeUnits::Frames:
                out = "[0-9]*";
                break;
            case TimeUnits::Seconds:
                out = "[0-9]*\\.[0-9]+|[0-9]+";
                break;
            case TimeUnits::Timecode:
                out = "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]";
                break;
            default: break;
            }
            return out;
        }

        QString timeToText(const otime::RationalTime& time, TimeUnits units)
        {
            QString out;
            switch (units)
            {
            case TimeUnits::Frames:
                out = QString::number(time.to_frames());
                break;
            case TimeUnits::Seconds:
                out = QString::number(time.to_seconds(), 'f', 2);
                break;
            case TimeUnits::Timecode:
            {
                otime::ErrorStatus errorStatus;
                out = time.to_timecode(&errorStatus).c_str();
                break;
            }
            default: break;
            }
            return out;
        }

        otime::RationalTime textToTime(
            const QString& text,
            double rate,
            TimeUnits units,
            otime::ErrorStatus* errorStatus)
        {
            otime::RationalTime out = time::invalidTime;
            switch (units)
            {
            case TimeUnits::Frames:
                out = otime::RationalTime::from_frames(text.toInt(), rate);
                break;
            case TimeUnits::Seconds:
                out = otime::RationalTime::from_seconds(text.toDouble());
                break;
            case TimeUnits::Timecode:
                out = otime::RationalTime::from_timecode(text.toLatin1().data(), rate, errorStatus);
                break;
            default: break;
            }
            return out;
        }

        TimeObject::TimeObject(QObject* parent) :
            QObject(parent)
        {}

        TimeUnits TimeObject::units() const
        {
            return _units;
        }

        void TimeObject::setUnits(TimeUnits units)
        {
            if (_units == units)
                return;
            _units = units;
            Q_EMIT unitsChanged(_units);
        }
    }
}
