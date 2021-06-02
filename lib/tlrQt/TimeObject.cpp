// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimeObject.h>

#include <QRegExpValidator>

namespace tlr
{
    namespace qt
    {
        TimeObject::TimeObject(QObject* parent) :
            QObject(parent)
        {}

        TimeObject::Units TimeObject::units() const
        {
            return _units;
        }

        void TimeObject::setUnits(Units units)
        {
            if (_units == units)
                return;
            _units = units;
            Q_EMIT unitsChanged(_units);
        }

        QString TimeObject::unitsSizeHintString(Units units)
        {
            QString out;
            switch (units)
            {
            case Units::Frames:
                out = "000000";
                break;
            case Units::Seconds:
                out = "000000.00";
                break;
            case Units::Timecode:
                out = "00:00:00:00";
                break;
            default: break;
            }
            return out;
        }

        QString TimeObject::unitsValidator(Units units)
        {
            QString out;
            switch (units)
            {
            case Units::Frames:
                out = "[0-9]*";
                break;
            case Units::Seconds:
                out = "[0-9]*\\.[0-9]+|[0-9]+";
                break;
            case Units::Timecode:
                out = "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]";
                break;
            default: break;
            }
            return out;
        }

        QString TimeObject::timeToText(const otime::RationalTime& time, Units units)
        {
            QString out;
            switch (units)
            {
            case Units::Frames:
                out = QString::number(time.to_frames());
                break;
            case Units::Seconds:
                out = QString::number(time.to_seconds(), 'f', 2);
                break;
            case Units::Timecode:
            {
                otime::ErrorStatus errorStatus;
                out = time.to_timecode(&errorStatus).c_str();
                break;
            }
            default: break;
            }
            return out;
        }

        otime::RationalTime TimeObject::textToTime(
            const QString& text,
            double rate,
            Units units,
            otime::ErrorStatus* errorStatus)
        {
            otime::RationalTime out = invalidTime;
            switch (units)
            {
            case Units::Frames:
                out = otime::RationalTime::from_frames(text.toInt(), rate);
                break;
            case Units::Seconds:
                out = otime::RationalTime::from_seconds(text.toDouble());
                break;
            case Units::Timecode:
                out = otime::RationalTime::from_timecode(text.toLatin1().data(), rate, errorStatus);
                break;
            default: break;
            }
            return out;
        }
    }
}
